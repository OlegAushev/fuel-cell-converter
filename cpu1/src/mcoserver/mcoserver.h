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


extern McoServerConfig config;

/**
 * @brief MicroCANopen server class.
 */
template <mcu::CanModule Module, emb::MasterSlaveMode Mode>
class McoServer
{
private:
	mcu::CanUnit<Module>* m_canUnit;
	TpdoService* m_tpdoService;
	RpdoService* m_rpdoService;
	SdoService* m_sdoService;

	volatile NmtState m_state;
	unsigned int m_nodeId;

	static emb::Array<mcu::CanMessageObject, COB_TYPE_COUNT> m_msgObjects;

	uint64_t m_heartbeatPeriod;
	emb::Array<uint64_t, 4> m_tpdoPeriods;
	static emb::Array<uint64_t(*)(), 4> m_getTpdoCallbacks;
	static emb::Array<void(*)(uint64_t), 4> m_processRpdoCallbacks;
	static void (*m_processRsdoCallback)(uint64_t);

public:
	/**
	 * @brief Configures server on CPU that is not CAN master.
	 * @param tpdoService - pointer to TPDO service
	 * @param rpdoService - pointer to RPDO service
	 * @param sdoService - pointer to SDO service
	 */
	McoServer(TpdoService* tpdoService, RpdoService* rpdoService, SdoService* sdoService)
		: m_canUnit(NULL)
		, m_tpdoService(tpdoService)
		, m_rpdoService(rpdoService)
		, m_sdoService(sdoService)
	{
		EMB_STATIC_ASSERT(Mode == emb::MODE_SLAVE);
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
	McoServer(mcu::CanTxPin txPin, mcu::CanRxPin rxPin, mcu::CanBitrate bitrate, NodeId nodeId,
			TpdoService* tpdoService, RpdoService* rpdoService, SdoService* sdoService)
		: m_tpdoService(tpdoService)
		, m_rpdoService(rpdoService)
		, m_sdoService(sdoService)
		, m_nodeId(nodeId.value)
	{
		EMB_STATIC_ASSERT(Mode == emb::MODE_MASTER);

		m_state = INITIALIZING;
		mcu::CanUnit<Module> canUnit(txPin, rxPin, bitrate);
		m_canUnit = mcu::CanUnit<Module>::instance();

		initMsgObjects();
		for (size_t i = 1; i < COB_TYPE_COUNT; ++i)
		{
			canUnit.setupMessageObject(m_msgObjects[i]);
		}

		m_heartbeatPeriod = 0;

		m_tpdoPeriods.fill(0);
		m_getTpdoCallbacks[TPDO_NUM1] = TpdoService::makeTpdo1;
		m_getTpdoCallbacks[TPDO_NUM2] = TpdoService::makeTpdo2;
		m_getTpdoCallbacks[TPDO_NUM3] = TpdoService::makeTpdo3;
		m_getTpdoCallbacks[TPDO_NUM4] = TpdoService::makeTpdo4;

		m_processRpdoCallbacks[RPDO_NUM1] = RpdoService::processRpdo1;
		m_processRpdoCallbacks[RPDO_NUM2] = RpdoService::processRpdo2;
		m_processRpdoCallbacks[RPDO_NUM3] = RpdoService::processRpdo3;
		m_processRpdoCallbacks[RPDO_NUM4] = RpdoService::processRpdo4;

		m_processRsdoCallback = SdoService::processRsdo;

		canUnit.registerRxInterruptHandler(onFrameReceived);
		m_state = PRE_OPERATIONAL;
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
	 * @brief Checks if periodic tasks must run and runs them if they must.
	 * @param (none)
	 * @return (none)
	 */
	void runPeriodicTasks()
	{
		static uint64_t timePrev = 0;
		if (mcu::Clock::now() == timePrev)
		{
			return;
		}
		timePrev = mcu::Clock::now();

		if (m_heartbeatPeriod != 0)
		{
			if ((mcu::Clock::now() % m_heartbeatPeriod) == 0)
			{
				sendHeartbeat();
			}
		}

		for (size_t i = 0; i < 4; ++i)
		{
			if (m_tpdoPeriods[i] != 0)
			{
				if ((mcu::Clock::now() % m_tpdoPeriods[i]) == 0)
				{
					sendTpdo(static_cast<TpdoNum>(i), m_getTpdoCallbacks[i]());
				}
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
		if (!mcu::isIpcSignalSet(CAN_TSDO_READY))
		{
			return;
		}

		emb::c28x::to_8bit_bytes<CobSdo>(m_msgObjects[TSDO].data, SdoService::tsdoData.cob);
		m_canUnit->send(TSDO, m_msgObjects[TSDO].data, cobDataLen[TSDO]);
		mcu::acknowledgeIpcSignal(CAN_TSDO_READY);
	}

	/**
	 * @brief Runs all server operations.
	 * @param (none)
	 * @return (none)
	 */
	void run()
	{
#ifdef CPU1
		m_rpdoService->utilizeProcessedMessages();
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
		mcu::CanUnit<Module>* canUnit = mcu::CanUnit<Module>::instance();

		uint32_t interruptCause = CAN_getInterruptCause(canUnit->module.base);
		uint16_t status = CAN_getStatus(canUnit->module.base);

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
		case RPDO2:
		case RPDO3:
		case RPDO4:
		{
			canUnit->recv(interruptCause, m_msgObjects[interruptCause].data);
			uint64_t rawPdoMsg = 0;
			emb::c28x::from_8bit_bytes<uint64_t>(rawPdoMsg, m_msgObjects[interruptCause].data);
			size_t rpdoNum = (interruptCause - RPDO1) / 2;
			m_processRpdoCallbacks[rpdoNum](rawPdoMsg);
			break;
		}

		case RSDO:
		{
			canUnit->recv(RSDO, m_msgObjects[RSDO].data);
			uint64_t rawSdoMsg = 0;
			emb::c28x::from_8bit_bytes<uint64_t>(rawSdoMsg, m_msgObjects[RSDO].data);
			m_processRsdoCallback(rawSdoMsg);
			break;
		}

		default:
			break;
		}

		CAN_clearInterruptStatus(canUnit->module.base, interruptCause);
		CAN_clearGlobalInterruptStatus(canUnit->module.base, CAN_GLOBAL_INT_CANINT0);
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
	}
};


template <mcu::CanModule Module, emb::MasterSlaveMode Mode>
emb::Array<mcu::CanMessageObject, COB_TYPE_COUNT> McoServer<Module, Mode>::m_msgObjects;
template <mcu::CanModule Module, emb::MasterSlaveMode Mode>
emb::Array<uint64_t(*)(), 4> McoServer<Module, Mode>::m_getTpdoCallbacks;
template <mcu::CanModule Module, emb::MasterSlaveMode Mode>
emb::Array<void(*)(uint64_t), 4> McoServer<Module, Mode>::m_processRpdoCallbacks;
template <mcu::CanModule Module, emb::MasterSlaveMode Mode>
void (*McoServer<Module, Mode>::m_processRsdoCallback)(uint64_t);



/// @}
} // namespace microcanopen


