/**
 * @defgroup mco_server Server
 * @ingroup microcanopen
 *
 * @file
 * @ingroup microcanopen mco_server
 */


#pragma once


#include "emb/emb_common.h"
#include "emb/emb_array.h"
#include "mcodef.h"
#include "tpdoservice/tpdoservice.h"
#include "rpdoservice/rpdoservice.h"
#include "sdoservice/sdoservice.h"

#include "syslog/syslog.h"
#include "mcu/can/mcucan.h"
#include "mcu/ipc/mcuipc.h"
#include "mcu/cputimers/mcucputimers.h"


namespace microcanopen {
/// @addtogroup mco_server
/// @{

/**
 * @brief IPC flags.
 */
struct IpcSignals
{
	mcu::IpcSignalPair rpdo1;
	mcu::IpcSignalPair rpdo2;
	mcu::IpcSignalPair rpdo3;
	mcu::IpcSignalPair rpdo4;
	mcu::IpcSignalPair rsdo;
	mcu::IpcSignalPair tsdo;
};

/**
 * @brief MicroCANopen server class.
 */
template <mcu::CanModule Module, emb::MasterSlaveMode Mode>
class McoServer : public emb::c28x::Singleton<McoServer<Module, Mode> >
{
private:
	mcu::CanUnit<Module>* m_canUnit;
	TpdoService<Module>* m_tpdoService;
	RpdoService<Module>* m_rpdoService;
	SdoService<Module>* m_sdoService;

	volatile NmtState m_state;
	unsigned int m_nodeId;

	emb::Array<mcu::CanMessageObject, COB_TYPE_COUNT> m_msgObjects;

	uint64_t m_heartbeatPeriod;
	emb::Array<uint64_t, 4> m_tpdoPeriods;

	// IPC signals
	mcu::IpcSignalPair RSDO_RECEIVED;
	mcu::IpcSignalPair TSDO_READY;
	mcu::IpcSignalPair RPDO1_RECEIVED;
	mcu::IpcSignalPair RPDO2_RECEIVED;
	mcu::IpcSignalPair RPDO3_RECEIVED;
	mcu::IpcSignalPair RPDO4_RECEIVED;

public:
	/**
	 * @brief Configures server on CPU that is not CAN master.
	 * @param tpdoService - pointer to TPDO service
	 * @param rpdoService - pointer to RPDO service
	 * @param sdoService - pointer to SDO service
	 */
	McoServer(TpdoService<Module>* tpdoService, RpdoService<Module>* rpdoService, SdoService<Module>* sdoService,
			const IpcSignals& ipcSignals)
		: emb::c28x::Singleton<McoServer<Module, Mode> >(this)
		, m_canUnit(NULL)
		, m_tpdoService(tpdoService)
		, m_rpdoService(rpdoService)
		, m_sdoService(sdoService)
		// IPC signals
		, RSDO_RECEIVED(ipcSignals.rsdo)
		, TSDO_READY(ipcSignals.tsdo)
		, RPDO1_RECEIVED(ipcSignals.rpdo1)
		, RPDO2_RECEIVED(ipcSignals.rpdo2)
		, RPDO3_RECEIVED(ipcSignals.rpdo3)
		, RPDO4_RECEIVED(ipcSignals.rpdo4)
	{
		EMB_STATIC_ASSERT(Mode == emb::MODE_SLAVE);

		rpdoService->initIpcSignals(&RPDO1_RECEIVED, &RPDO2_RECEIVED,
				&RPDO3_RECEIVED, &RPDO4_RECEIVED);
		sdoService->initIpcSignals(&RSDO_RECEIVED, &TSDO_READY);
	}

	/**
	 * @brief Configures server on CPU that is CAN master.
	 * @param txPin - MCU CAN-TX pin
	 * @param rxPin - MCU CAN-RX pin
	 * @param bitrate - CAN bus bitrate
	 * @param nodeId - node ID
	 * @param clock - pointer to system clock (ms)
	 * @param tpdoService - pointer to TPDO service
	 * @param rpdoService - pointer to RPDO service
	 * @param sdoService - pointer to SDO service
	 */
	McoServer(mcu::GpioPinConfig txPin, mcu::GpioPinConfig rxPin, mcu::CanBitrate bitrate, NodeId nodeId,
			TpdoService<Module>* tpdoService, RpdoService<Module>* rpdoService, SdoService<Module>* sdoService,
			const IpcSignals& ipcSignals)
		: emb::c28x::Singleton<McoServer<Module, Mode> >(this)
		, m_tpdoService(tpdoService)
		, m_rpdoService(rpdoService)
		, m_sdoService(sdoService)
		, m_nodeId(nodeId.value)
		// IPC signals
		, RSDO_RECEIVED(ipcSignals.rsdo)
		, TSDO_READY(ipcSignals.tsdo)
		, RPDO1_RECEIVED(ipcSignals.rpdo1)
		, RPDO2_RECEIVED(ipcSignals.rpdo2)
		, RPDO3_RECEIVED(ipcSignals.rpdo3)
		, RPDO4_RECEIVED(ipcSignals.rpdo4)
	{
		EMB_STATIC_ASSERT(Mode == emb::MODE_MASTER);

		rpdoService->initIpcSignals(&RPDO1_RECEIVED, &RPDO2_RECEIVED,
				&RPDO3_RECEIVED, &RPDO4_RECEIVED);
		sdoService->initIpcSignals(&RSDO_RECEIVED, &TSDO_READY);

		m_state = INITIALIZING;
		m_canUnit = new mcu::CanUnit<Module>(txPin, rxPin, bitrate);

		initMsgObjects();
		for (size_t i = 1; i < COB_TYPE_COUNT; ++i)
		{
			m_canUnit->setupMessageObject(m_msgObjects[i]);
		}

		m_heartbeatPeriod = 0;
		m_tpdoPeriods.fill(0);

		m_canUnit->registerRxInterruptHandler(onFrameReceived);
		m_state = PRE_OPERATIONAL;
	}

	/**
	 * @brief Server destructor.
	 */
	~McoServer()
	{
		if (m_canUnit)
		{
			delete m_canUnit;
		}
	}

	/**
	 * @brief Sets HEARTBEAT message period.
	 * @param periodMs - period in milliseconds
	 * @return (none)
	 */
	void setHeartbeatPeriod(uint64_t periodMs) { m_heartbeatPeriod = periodMs; }

	/**
	 * @brief Sets specified TPDO message period.
	 * @param tpdoNum - TPDO number
	 * @param periodMs - period in milliseconds
	 * @return (none)
	 */
	void setTpdoPeriod(TpdoNum tpdoNum, uint64_t periodMs) { m_tpdoPeriods[tpdoNum] = periodMs; }

	/**
	 * @brief Registers specified RPDO processing callback.
	 * @param rpdoNum - RPDO message number
	 * @param cb - pointer to callback function
	 * @return (none)
	 */
	void registerRpdoCallback(RpdoNum rpdoNum, void (*cb)(uint64_t)) { m_processRpdoCallbacks[rpdoNum] = cb; }

	/**
	 * @brief Sets specified RPDO message ID
	 * @param rpdoNum - RPDO number
	 * @param frameId - frame ID
	 * @return (none)
	 */
	void setRpdoId(RpdoNum rpdoNum, unsigned int frameId)
	{
		size_t RPDOx = RPDO1 + rpdoNum * 2;
		m_msgObjects[RPDOx].frameId = frameId;
		m_canUnit->setupMessageObject(m_msgObjects[RPDOx]);
	}

	/**
	 * @brief Enables server.
	 * @param (none)
	 * @return (none)
	 */
	void enable()
	{
		m_canUnit->enableRxInterrupt();
		m_state = OPERATIONAL;
	}

	/**
	 * @brief Disables server.
	 * @param (none)
	 * @return (none)
	 */
	void disable()
	{;
		m_canUnit->disableRxInterrupt();
		m_state = STOPPED;
	}

	/**
	 * @brief Runs all server operations.
	 * @param (none)
	 * @return (none)
	 */
	void run()
	{
#ifdef CPU1
		m_rpdoService->respondToProcessedRpdo();
		m_sdoService->processRequest();
#ifndef DUALCORE
		runPeriodicTasks();
		sendSdoResponse();
#endif
#endif
#ifdef CPU2
		runPeriodicTasks();
		sendSdoResponse();
#endif
	}

	/**
	 * @brief Checks if periodic tasks must run and runs them if they must.
	 * @param (none)
	 * @return (none)
	 */
	void runPeriodicTasks()
	{
		static uint64_t timeHbPrev = 0;
		static uint64_t timeTpdoPrev[4] = {0};

		if ((m_heartbeatPeriod != 0)
				&& (mcu::SystemClock::now() >= (timeHbPrev + m_heartbeatPeriod)))
		{
			sendHeartbeat();
			timeHbPrev = mcu::SystemClock::now();
		}

		for (size_t i = 0; i < 4; ++i)
		{
			if ((m_tpdoPeriods[i] != 0)
					&& (mcu::SystemClock::now() >= (timeTpdoPrev[i] + m_tpdoPeriods[i])))
			{
				switch (i)
				{
				case 0:
					sendTpdo(TPDO_NUM1, m_tpdoService->makeTpdo1());
					break;
				case 1:
					sendTpdo(TPDO_NUM2, m_tpdoService->makeTpdo2());
					break;
				case 2:
					sendTpdo(TPDO_NUM3, m_tpdoService->makeTpdo3());
					break;
				case 3:
					sendTpdo(TPDO_NUM4, m_tpdoService->makeTpdo4());
					break;
				}
				timeTpdoPrev[i] = mcu::SystemClock::now();
			}
		}
	}

	/**
	 * @brief Processes SDO response, sends TSDO.
	 * @param (none)
	 * @return (none)
	 */
	void sendSdoResponse()
	{
#ifdef DUALCORE
		if (!mcu::remoteIpcSignalSent(TSDO_READY.remote)) return;
#else
		if (!mcu::localIpcSignalSent(TSDO_READY.local)) return;
#endif

		emb::c28x::to_8bit_bytes<CobSdo>(m_msgObjects[TSDO].data, SdoService<Module>::tsdoData());
		m_canUnit->send(TSDO, m_msgObjects[TSDO].data, cobDataLen[TSDO]);
#ifdef DUALCORE
		mcu::acknowledgeRemoteIpcSignal(TSDO_READY.remote);
#else
		mcu::revokeLocalIpcSignal(TSDO_READY.local);
#endif
	}

protected:
	/**
	 * @brief Initializes message objects.
	 * @param (none)
	 * @return (none)
	 */
	void initMsgObjects()
	{
		for (size_t i = 0; i < COB_TYPE_COUNT; ++i)
		{
			m_msgObjects[i].objId = i;
			m_msgObjects[i].frameId = cobId(static_cast<CobType>(i), m_nodeId);
			m_msgObjects[i].frameType = CAN_MSG_FRAME_STD;
			m_msgObjects[i].frameIdMask = 0;
			m_msgObjects[i].dataLen = cobDataLen[i];
		}

		m_msgObjects[EMCY].objType = m_msgObjects[TPDO1].objType
				= m_msgObjects[TPDO2].objType = m_msgObjects[TPDO3].objType
				= m_msgObjects[TPDO4].objType = m_msgObjects[TSDO].objType
				= m_msgObjects[HEARTBEAT].objType = CAN_MSG_OBJ_TYPE_TX;
		m_msgObjects[NMT].objType = m_msgObjects[SYNC].objType
				= m_msgObjects[TIME].objType = m_msgObjects[RPDO1].objType
				= m_msgObjects[RPDO2].objType = m_msgObjects[RPDO3].objType
				= m_msgObjects[RPDO4].objType = m_msgObjects[RSDO].objType = CAN_MSG_OBJ_TYPE_RX;

		m_msgObjects[EMCY].flags = m_msgObjects[TPDO1].flags
				= m_msgObjects[TPDO2].flags = m_msgObjects[TPDO3].flags
				= m_msgObjects[TPDO4].flags = m_msgObjects[TSDO].flags
				= m_msgObjects[HEARTBEAT].flags
				= CAN_MSG_OBJ_NO_FLAGS;

		m_msgObjects[NMT].flags = m_msgObjects[SYNC].flags
				= m_msgObjects[TIME].flags = m_msgObjects[RPDO1].flags
				= m_msgObjects[RPDO2].flags = m_msgObjects[RPDO3].flags
				= m_msgObjects[RPDO4].flags = m_msgObjects[RSDO].flags
				= CAN_MSG_OBJ_RX_INT_ENABLE;
	}

	/**
	 * @brief Sends HEARTBEAT message.
	 * @param (none)
	 * @return (none)
	 */
	void sendHeartbeat()
	{
		uint64_t heartbeatData = m_state;
		emb::c28x::to_8bit_bytes<uint64_t>(m_msgObjects[HEARTBEAT].data, heartbeatData);
		m_canUnit->send(HEARTBEAT, m_msgObjects[HEARTBEAT].data, cobDataLen[HEARTBEAT]);
	}

	/**
	 * @brief Sends specified TPDO message.
	 * @param tpdoNum - TPDO message number
	 * @param tpdoData - TPDO message raw data
	 * @return (none)
	 */
	void sendTpdo(TpdoNum tpdoNum, uint64_t tpdoData)
	{
		size_t TPDOx = TPDO1 + tpdoNum * 2;
		emb::c28x::to_8bit_bytes<uint64_t>(m_msgObjects[TPDOx].data, tpdoData);
		m_canUnit->send(TPDOx, m_msgObjects[TPDOx].data, cobDataLen[TPDOx]);
	}

/* ========================================================================== */
/* ====================== INTERRUPT SERVICE ROUTINES ======================== */
/* ========================================================================== */

	/**
	 * @brief CAN RX ISR
	 * @param (none)
	 * @return (none)
	 */
	static __interrupt void onFrameReceived()
	{
		McoServer<Module, Mode>* server = McoServer<Module, Mode>::instance();
		mcu::CanUnit<Module>* canUnit = mcu::CanUnit<Module>::instance();

		uint32_t interruptCause = CAN_getInterruptCause(canUnit->base());
		uint16_t status = CAN_getStatus(canUnit->base());

		switch (interruptCause)
		{
		case CAN_INT_INT0ID_STATUS:
			switch (status)
			{
			case CAN_STATUS_PERR:
			case CAN_STATUS_BUS_OFF:
			case CAN_STATUS_EWARN:
			case CAN_STATUS_LEC_BIT1:
			case CAN_STATUS_LEC_BIT0:
			case CAN_STATUS_LEC_CRC:
				Syslog::setWarning(Warning::CAN_BUS_WARNING);
				break;
			default:
				break;
			}
			break;

		case RPDO1:
		{
			canUnit->recv(interruptCause, server->m_msgObjects[interruptCause].data);
			uint64_t rawPdoMsg = 0;
			emb::c28x::from_8bit_bytes<uint64_t>(rawPdoMsg, server->m_msgObjects[interruptCause].data);
			server->m_rpdoService->processRpdo1(rawPdoMsg);
			break;
		}

		case RPDO2:
		{
			canUnit->recv(interruptCause, server->m_msgObjects[interruptCause].data);
			uint64_t rawPdoMsg = 0;
			emb::c28x::from_8bit_bytes<uint64_t>(rawPdoMsg, server->m_msgObjects[interruptCause].data);
			server->m_rpdoService->processRpdo2(rawPdoMsg);
			break;
		}

		case RPDO3:
		{
			canUnit->recv(interruptCause, server->m_msgObjects[interruptCause].data);
			uint64_t rawPdoMsg = 0;
			emb::c28x::from_8bit_bytes<uint64_t>(rawPdoMsg, server->m_msgObjects[interruptCause].data);
			server->m_rpdoService->processRpdo3(rawPdoMsg);
			break;
		}

		case RPDO4:
		{
			canUnit->recv(interruptCause, server->m_msgObjects[interruptCause].data);
			uint64_t rawPdoMsg = 0;
			emb::c28x::from_8bit_bytes<uint64_t>(rawPdoMsg, server->m_msgObjects[interruptCause].data);
			server->m_rpdoService->processRpdo4(rawPdoMsg);
			break;
		}

		case RSDO:
		{
			canUnit->recv(RSDO, server->m_msgObjects[RSDO].data);
			uint64_t rawSdoMsg = 0;
			emb::c28x::from_8bit_bytes<uint64_t>(rawSdoMsg, server->m_msgObjects[RSDO].data);
			server->m_sdoService->processRsdo(rawSdoMsg);
			break;
		}

		default:
			break;
		}

		CAN_clearInterruptStatus(canUnit->base(), interruptCause);
		CAN_clearGlobalInterruptStatus(canUnit->base(), CAN_GLOBAL_INT_CANINT0);
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
	}
};


/// @}
} // namespace microcanopen


