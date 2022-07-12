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
#include "mcu/can/mcucan.h"
#include "mcu/ipc/mcuipc.h"
#include "mcu/cputimers/mcucputimers.h"
#include "mcodef.h"
#include "tpdoservice/tpdoservice.h"
#include "rpdoservice/rpdoservice.h"
#include "sdoservice/sdoservice.h"

// APP-SPECIFIC headers
#include "syslog/syslog.h"


namespace microcanopen {
/// @addtogroup mco_server
/// @{


/**
 * @brief IPC flags.
 */
struct IpcFlags
{
	mcu::IpcFlag RPDO1_RECEIVED;
	mcu::IpcFlag RPDO2_RECEIVED;
	mcu::IpcFlag RPDO3_RECEIVED;
	mcu::IpcFlag RPDO4_RECEIVED;
	mcu::IpcFlag RSDO_RECEIVED;
	mcu::IpcFlag TSDO_READY;
};


/**
 * @brief MicroCANopen server class.
 */
template <mcu::CanModule Module, mcu::IpcMode Ipc, emb::MasterSlaveMode Mode>
class McoServer : public emb::c28x::Singleton<McoServer<Module, Ipc, Mode> >
{
private:
	mcu::CanUnit<Module>* m_canUnit;
	TpdoService<Module, Ipc, Mode>* m_tpdoService;
	RpdoService<Module, Ipc, Mode>* m_rpdoService;
	SdoService<Module, Ipc, Mode>* m_sdoService;

	volatile NmtState m_state;
	unsigned int m_nodeId;

	emb::Array<mcu::CanMessageObject, COB_TYPE_COUNT> m_msgObjects;

	uint64_t m_heartbeatPeriod;
	emb::Array<uint64_t, 4> m_tpdoPeriods;

	// IPC flags
	mcu::IpcFlag RPDO1_RECEIVED;
	mcu::IpcFlag RPDO2_RECEIVED;
	mcu::IpcFlag RPDO3_RECEIVED;
	mcu::IpcFlag RPDO4_RECEIVED;
	mcu::IpcFlag RSDO_RECEIVED;
	mcu::IpcFlag TSDO_READY;

public:
	/**
	 * @brief Configures server on CPU that is not CAN master.
	 * @param tpdoService - pointer to TPDO service
	 * @param rpdoService - pointer to RPDO service
	 * @param sdoService - pointer to SDO service
	 */
	McoServer(TpdoService<Module, Ipc, Mode>* tpdoService,
			RpdoService<Module, Ipc, Mode>* rpdoService,
			SdoService<Module, Ipc, Mode>* sdoService,
			const IpcFlags& ipcSignals)
		: emb::c28x::Singleton<McoServer<Module, Ipc, Mode> >(this)
		, m_canUnit(NULL)
		, m_tpdoService(tpdoService)
		, m_rpdoService(rpdoService)
		, m_sdoService(sdoService)
		// IPC flags
		, RSDO_RECEIVED(ipcSignals.RSDO_RECEIVED)
		, TSDO_READY(ipcSignals.TSDO_READY)
		, RPDO1_RECEIVED(ipcSignals.RPDO1_RECEIVED)
		, RPDO2_RECEIVED(ipcSignals.RPDO2_RECEIVED)
		, RPDO3_RECEIVED(ipcSignals.RPDO3_RECEIVED)
		, RPDO4_RECEIVED(ipcSignals.RPDO4_RECEIVED)
	{
		EMB_STATIC_ASSERT(Mode == emb::MODE_SLAVE);
		EMB_STATIC_ASSERT(Ipc != mcu::IPC_MODE_SINGLECORE);

		rpdoService->initIpcFlags(RPDO1_RECEIVED, RPDO2_RECEIVED,
				RPDO3_RECEIVED, RPDO4_RECEIVED);
		sdoService->initIpcFlags(RSDO_RECEIVED, TSDO_READY);
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
	McoServer(mcu::GpioPinConfig txPin, mcu::GpioPinConfig rxPin,
			mcu::CanBitrate bitrate, mcu::CanMode mode,
			NodeId nodeId,
			TpdoService<Module, Ipc, Mode>* tpdoService,
			RpdoService<Module, Ipc, Mode>* rpdoService,
			SdoService<Module, Ipc, Mode>* sdoService,
			const IpcFlags& ipcSignals)
		: emb::c28x::Singleton<McoServer<Module, Ipc, Mode> >(this)
		, m_tpdoService(tpdoService)
		, m_rpdoService(rpdoService)
		, m_sdoService(sdoService)
		, m_nodeId(nodeId.value)
		// IPC flags
		, RSDO_RECEIVED(ipcSignals.RSDO_RECEIVED)
		, TSDO_READY(ipcSignals.TSDO_READY)
		, RPDO1_RECEIVED(ipcSignals.RPDO1_RECEIVED)
		, RPDO2_RECEIVED(ipcSignals.RPDO2_RECEIVED)
		, RPDO3_RECEIVED(ipcSignals.RPDO3_RECEIVED)
		, RPDO4_RECEIVED(ipcSignals.RPDO4_RECEIVED)
	{
		EMB_STATIC_ASSERT(Mode == emb::MODE_MASTER);

		rpdoService->initIpcFlags(RPDO1_RECEIVED, RPDO2_RECEIVED,
				RPDO3_RECEIVED, RPDO4_RECEIVED);
		sdoService->initIpcFlags(RSDO_RECEIVED, TSDO_READY);

		m_state = INITIALIZING;
		m_canUnit = new mcu::CanUnit<Module>(txPin, rxPin, bitrate, mode);

		initMsgObjects();
		for (size_t i = 1; i < COB_TYPE_COUNT; ++i)
		{
			m_canUnit->setupMessageObject(m_msgObjects[i]);
		}

		m_heartbeatPeriod = 0;
		m_tpdoPeriods.fill(0);

		m_canUnit->registerInterruptHandler(onFrameReceived);
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
		m_canUnit->enableInterrupts();
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
		switch (Ipc)
		{
		case mcu::IPC_MODE_SINGLECORE:
			m_rpdoService->respondToProcessedRpdo();
			m_sdoService->processRequest();
			runPeriodicTasks();
			sendSdoResponse();
			break;
		case mcu::IPC_MODE_DUALCORE:
			switch (Mode)
			{
			case emb::MODE_MASTER:
				runPeriodicTasks();
				sendSdoResponse();
				break;
			case emb::MODE_SLAVE:
				m_rpdoService->respondToProcessedRpdo();
				m_sdoService->processRequest();
				break;
			}
			break;
		}
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
		if (!mcu::isIpcFlagSet(TSDO_READY, Ipc)) return;

		emb::c28x::to_bytes8<CobSdo>(m_msgObjects[TSDO].data, SdoService<Module, Ipc, Mode>::tsdoData());
		m_canUnit->send(TSDO, m_msgObjects[TSDO].data, cobDataLen[TSDO]);

		mcu::resetIpcFlag(TSDO_READY, Ipc);
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
		emb::c28x::to_bytes8<uint64_t>(m_msgObjects[HEARTBEAT].data, heartbeatData);
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
		emb::c28x::to_bytes8<uint64_t>(m_msgObjects[TPDOx].data, tpdoData);
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
		McoServer<Module, Ipc, Mode>* server = McoServer<Module, Ipc, Mode>::instance();
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
			emb::c28x::from_bytes8<uint64_t>(rawPdoMsg, server->m_msgObjects[interruptCause].data);
			server->m_rpdoService->processRpdo1(rawPdoMsg);
			break;
		}

		case RPDO2:
		{
			canUnit->recv(interruptCause, server->m_msgObjects[interruptCause].data);
			uint64_t rawPdoMsg = 0;
			emb::c28x::from_bytes8<uint64_t>(rawPdoMsg, server->m_msgObjects[interruptCause].data);
			server->m_rpdoService->processRpdo2(rawPdoMsg);
			break;
		}

		case RPDO3:
		{
			canUnit->recv(interruptCause, server->m_msgObjects[interruptCause].data);
			uint64_t rawPdoMsg = 0;
			emb::c28x::from_bytes8<uint64_t>(rawPdoMsg, server->m_msgObjects[interruptCause].data);
			server->m_rpdoService->processRpdo3(rawPdoMsg);
			break;
		}

		case RPDO4:
		{
			canUnit->recv(interruptCause, server->m_msgObjects[interruptCause].data);
			uint64_t rawPdoMsg = 0;
			emb::c28x::from_bytes8<uint64_t>(rawPdoMsg, server->m_msgObjects[interruptCause].data);
			server->m_rpdoService->processRpdo4(rawPdoMsg);
			break;
		}

		case RSDO:
		{
			canUnit->recv(RSDO, server->m_msgObjects[RSDO].data);
			uint64_t rawSdoMsg = 0;
			emb::c28x::from_bytes8<uint64_t>(rawSdoMsg, server->m_msgObjects[RSDO].data);
			server->m_sdoService->processRsdo(rawSdoMsg);
			break;
		}

		default:
			break;
		}

		canUnit->acknowledgeInterrupt(interruptCause);
	}
};


/// @}
} // namespace microcanopen


