/**
 * @defgroup ucanopen_server Server
 * @ingroup ucanopen
 *
 * @file
 * @ingroup ucanopen ucanopen_server
 */


#pragma once


#include "emb/emb_common.h"
#include "emb/emb_array.h"
#include "mcu/can/mcu_can.h"
#include "mcu/ipc/mcu_ipc.h"
#include "mcu/cputimers/mcu_cputimers.h"
#include "ucanopen_def.h"
#include "rpdoservice/rpdoservice.h"
#include "tpdoservice/tpdoservice.h"
#include "sdoservice/sdoservice.h"

// APP-SPECIFIC headers
#include "sys/syslog/syslog.h"


namespace ucanopen {
/// @addtogroup ucanopen_server
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
 * @brief uCANopen server class.
 */
template <mcu::CanModule Module, mcu::IpcMode Ipc, emb::MasterSlaveMode Mode>
class Server : public emb::c28x::Singleton<Server<Module, Ipc, Mode> >
{
private:
	mcu::Can<Module>* m_can;
	RpdoService<Module, Ipc, Mode>* m_rpdoService;
	TpdoService<Module, Ipc, Mode>* m_tpdoService;
	SdoService<Module, Ipc, Mode>* m_sdoService;

	volatile NmtState m_state;
	unsigned int m_nodeId;

	emb::Array<mcu::CanMessageObject, COB_TYPE_COUNT> m_msgObjects;

	uint64_t m_heartbeatPeriod;
	emb::Array<uint64_t, 4> m_tpdoPeriods;

	// signals from ISR
	emb::Array<bool, 5> m_hasRawRdo;

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
	 * @param rpdoService - pointer to RPDO service
	 * @param tpdoService - pointer to TPDO service
	 * @param sdoService - pointer to SDO service
	 */
	Server(RpdoService<Module, Ipc, Mode>* rpdoService,
			TpdoService<Module, Ipc, Mode>* tpdoService,
			SdoService<Module, Ipc, Mode>* sdoService,
			const IpcFlags& ipcFlags)
		: emb::c28x::Singleton<Server<Module, Ipc, Mode> >(this)
		, m_can(NULL)
		, m_rpdoService(rpdoService)
		, m_tpdoService(tpdoService)
		, m_sdoService(sdoService)
		// IPC flags
		, RPDO1_RECEIVED(ipcFlags.RPDO1_RECEIVED)
		, RPDO2_RECEIVED(ipcFlags.RPDO2_RECEIVED)
		, RPDO3_RECEIVED(ipcFlags.RPDO3_RECEIVED)
		, RPDO4_RECEIVED(ipcFlags.RPDO4_RECEIVED)
		, RSDO_RECEIVED(ipcFlags.RSDO_RECEIVED)
		, TSDO_READY(ipcFlags.TSDO_READY)
	{
		EMB_STATIC_ASSERT(Mode == emb::MODE_SLAVE);
		EMB_STATIC_ASSERT(Ipc != mcu::IPC_MODE_SINGLECORE);

		rpdoService->initIpcFlags(RPDO1_RECEIVED, RPDO2_RECEIVED,
				RPDO3_RECEIVED, RPDO4_RECEIVED);
		sdoService->initIpcFlags(RSDO_RECEIVED, TSDO_READY);

		m_hasRawRdo.fill(false);
	}

	/**
	 * @brief Configures server on CPU that is CAN master.
	 * @param rxPin - MCU CAN-RX pin
	 * @param txPin - MCU CAN-TX pin
	 * @param bitrate - CAN bus bitrate
	 * @param mode - CAN mode
	 * @param nodeId - node ID
	 * @param clock - pointer to system clock (ms)
	 * @param rpdoService - pointer to RPDO service
	 * @param tpdoService - pointer to TPDO service
	 * @param sdoService - pointer to SDO service
	 */
	Server(mcu::GpioConfig rxPin, mcu::GpioConfig txPin,
			mcu::CanBitrate bitrate, mcu::CanMode mode,
			NodeId nodeId,
			RpdoService<Module, Ipc, Mode>* rpdoService,
			TpdoService<Module, Ipc, Mode>* tpdoService,
			SdoService<Module, Ipc, Mode>* sdoService,
			const IpcFlags& ipcFlags)
		: emb::c28x::Singleton<Server<Module, Ipc, Mode> >(this)
		, m_rpdoService(rpdoService)
		, m_tpdoService(tpdoService)
		, m_sdoService(sdoService)
		, m_nodeId(nodeId.value)
		// IPC flags
		, RPDO1_RECEIVED(ipcFlags.RPDO1_RECEIVED)
		, RPDO2_RECEIVED(ipcFlags.RPDO2_RECEIVED)
		, RPDO3_RECEIVED(ipcFlags.RPDO3_RECEIVED)
		, RPDO4_RECEIVED(ipcFlags.RPDO4_RECEIVED)
		, RSDO_RECEIVED(ipcFlags.RSDO_RECEIVED)
		, TSDO_READY(ipcFlags.TSDO_READY)
	{
		EMB_STATIC_ASSERT(Mode == emb::MODE_MASTER);

		rpdoService->initIpcFlags(RPDO1_RECEIVED, RPDO2_RECEIVED,
				RPDO3_RECEIVED, RPDO4_RECEIVED);
		sdoService->initIpcFlags(RSDO_RECEIVED, TSDO_READY);

		m_state = INITIALIZING;
		m_can = new mcu::Can<Module>(rxPin, txPin, bitrate, mode);

		initMsgObjects();
		for (size_t i = 1; i < COB_TYPE_COUNT; ++i)
		{
			m_can->setupMessageObject(m_msgObjects[i]);
		}

		m_heartbeatPeriod = 0;
		m_tpdoPeriods.fill(0);

		m_hasRawRdo.fill(false);

		m_can->registerInterruptHandler(onFrameReceived);
		m_state = PRE_OPERATIONAL;
	}

	/**
	 * @brief Server destructor.
	 */
	~Server()
	{
		if (m_can)
		{
			delete m_can;
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
		m_can->setupMessageObject(m_msgObjects[RPDOx]);
	}

	/**
	 * @brief Enables server.
	 * @param (none)
	 * @return (none)
	 */
	void enable()
	{
		m_can->enableInterrupts();
		m_state = OPERATIONAL;
	}

	/**
	 * @brief Disables server.
	 * @param (none)
	 * @return (none)
	 */
	void disable()
	{;
		m_can->disableRxInterrupt();
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
			processRawRdo();
			m_rpdoService->respondToProcessedRpdo();
			m_sdoService->processRequest();
			runPeriodicTasks();
			sendSdoResponse();
			break;
		case mcu::IPC_MODE_DUALCORE:
			switch (Mode)
			{
			case emb::MODE_MASTER:
				processRawRdo();
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
		m_can->send(TSDO, m_msgObjects[TSDO].data, cobDataLen[TSDO]);

		mcu::resetIpcFlag(TSDO_READY, Ipc);
	}

	/**
	 * @brief Processes raw RPDO and RSDO received by ISR.
	 * @param (none)
	 * @return (none)
	 */
	void processRawRdo()
	{
		for (size_t i = 0; i < m_hasRawRdo.size(); ++i)
		{
			if (m_hasRawRdo[i])
			{
				uint64_t rawMsg = 0;
				uint32_t rdo = 2 * i + static_cast<uint32_t>(RPDO1);
				emb::c28x::from_bytes8<uint64_t>(rawMsg, m_msgObjects[rdo].data);
				switch (rdo)
				{
				case RPDO1:
					m_rpdoService->processRpdo1(rawMsg);
					break;
				case RPDO2:
					m_rpdoService->processRpdo2(rawMsg);
					break;
				case RPDO3:
					m_rpdoService->processRpdo3(rawMsg);
					break;
				case RPDO4:
					m_rpdoService->processRpdo4(rawMsg);
					break;
				case RSDO:
					m_sdoService->processRsdo(rawMsg);
					break;
				default:
					break;
				}
				m_hasRawRdo[i] = false;
			}
		}
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
		m_can->send(HEARTBEAT, m_msgObjects[HEARTBEAT].data, cobDataLen[HEARTBEAT]);
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
		m_can->send(TPDOx, m_msgObjects[TPDOx].data, cobDataLen[TPDOx]);
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
		Server<Module, Ipc, Mode>* server = Server<Module, Ipc, Mode>::instance();
		mcu::Can<Module>* can = mcu::Can<Module>::instance();

		uint32_t interruptCause = CAN_getInterruptCause(can->base());
		uint16_t status = CAN_getStatus(can->base());

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
				Syslog::setWarning(sys::Warning::CAN_BUS_WARNING);
				break;
			default:
				break;
			}
			break;

		case RPDO1:
		case RPDO2:
		case RPDO3:
		case RPDO4:
		case RSDO:
		{
			uint32_t rdoIdx = (interruptCause - static_cast<uint32_t>(RPDO1)) / 2;
			if (!server->m_hasRawRdo[rdoIdx])
			{
				// there is no raw unprocessed data of this type
				can->recv(interruptCause, server->m_msgObjects[interruptCause].data);
				server->m_hasRawRdo[rdoIdx] = true;
			}
			else
			{
				Syslog::setWarning(sys::Warning::CAN_BUS_OVERRUN);
			}
			break;
		}

		default:
			break;
		}

		can->acknowledgeInterrupt(interruptCause);
	}
};


/// @}
} // namespace ucanopen


