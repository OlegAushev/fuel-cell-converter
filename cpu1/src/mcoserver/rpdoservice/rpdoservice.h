/**
 * @defgroup mco_rpdo_service RPDO Service
 * @ingroup microcanopen
 *
 * @file
 * @ingroup microcanopen mco_rpdo_service
 */


#pragma once


#include "driverlib.h"
#include "device.h"

#include "emb/emb_common.h"
#include "../mcodef.h"
#include "mcu/can/mcucan.h"
#include "mcu/ipc/mcuipc.h"
#include "mcu/gpio/mcugpio.h"
#include "mcu/cputimers/mcucputimers.h"

// APP-SPECIFIC headers
#include "boostconverter/boostconverter.h"


namespace microcanopen {
/// @addtogroup mco_rpdo_service
/// @{


/* ========================================================================== */
/* ======================= APPLICATION-SPECIFIC BEGIN ======================= */
/* ========================================================================== */

/**
 * @ingroup mco_app_spec
 * @brief RPDO1 message data.
 */
struct CobRpdo1
{
	union
	{
		struct
		{
			uint16_t run : 1;
			uint32_t reserved1 : 31;
			uint16_t emergencyStop : 1;
			uint32_t reserved2 : 31;
		} can1;
		struct
		{
			uint64_t reserved;
		} can2;
	};
	CobRpdo1()
	{
		uint64_t rawMsg = 0;
		memcpy(this, &rawMsg, sizeof(CobRpdo1));
	}
	CobRpdo1(uint64_t rawMsg) { memcpy(this, &rawMsg, sizeof(CobRpdo1)); }
};


/**
 * @ingroup mco_app_spec
 * @brief RPDO2 message data.
 */
struct CobRpdo2
{
	union
	{
		struct
		{
			float32 speed;
			float32 torque;
		} can1;
		struct
		{
			uint64_t reserved;
		} can2;
	};
	CobRpdo2()
	{
		uint64_t rawMsg = 0;
		memcpy(this, &rawMsg, sizeof(CobRpdo2));
	}
	CobRpdo2(uint64_t rawMsg) { memcpy(this, &rawMsg, sizeof(CobRpdo2)); }
};


/**
 * @ingroup mco_app_spec
 * @brief Data storage for IPC.
 */
struct ProcessedRpdoData
{
	bool bitRun;
	bool bitEmergencyStop;
	float speedRef;
	float torquePuRef;
};
extern ProcessedRpdoData rpdoProcessedDataShared;
extern ProcessedRpdoData rpdoProcessedDataNonShared;

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART END ======================== */
/* ========================================================================== */


#if (defined(ACIM_MOTOR_SIX_PHASE))
#define RPDO_DRIVE(module, ipc, mode) RpdoService<module, ipc, mode>::drive6Ph
#define RPDO_DRIVE2(module, ipc, mode) static_cast<acim::Drive<acim::SIX_PHASE, acim::DRIVE_INSTANCE_1>*>(NULL)
#elif (defined(ACIM_MOTOR_THREE_PHASE))
#define RPDO_DRIVE(module, ipc, mode) RpdoService<module, ipc, mode>::drive3Ph_1
#define RPDO_DRIVE2(module, ipc, mode) RpdoService<module, ipc, mode>::drive3Ph_2
#elif (defined(ACIM_TWO_MOTORS))
#define RPDO_DRIVE(module, ipc, mode) RpdoService<module, ipc, mode>::drive3Ph_1
#define RPDO_DRIVE2(module, ipc, mode) RpdoService<module, ipc, mode>::drive3Ph_2
#endif


/**
 * @brief RPDO-service class.
 */
template <mcu::CanModule Module, mcu::IpcMode Ipc, emb::MasterSlaveMode Mode>
class RpdoService
{
	friend class RpdoServiceTest;
private:
	// APP-SPECIFIC objects
	BoostConverter* converter;
private:
	mcu::IpcSignalPair RPDO1_RECEIVED;
	mcu::IpcSignalPair RPDO2_RECEIVED;
	mcu::IpcSignalPair RPDO3_RECEIVED;
	mcu::IpcSignalPair RPDO4_RECEIVED;

	/// Data-storage for IPC
	ProcessedRpdoData* s_rpdoProcessedData;

private:
	RpdoService(const RpdoService& other);			// no copy constructor
	RpdoService& operator=(const RpdoService& other);	// no copy assignment operator
public:
	/**
	 * @brief Configures service.
	 */
	RpdoService()
	{
		EMB_STATIC_ASSERT(Ipc == mcu::IPC_MODE_DUALCORE);
		EMB_STATIC_ASSERT(Mode == emb::MODE_MASTER);
		switch (Ipc)
		{
		case mcu::IPC_MODE_SINGLECORE:
			s_rpdoProcessedData = &rpdoProcessedDataNonShared;
			break;
		case mcu::IPC_MODE_DUALCORE:
			s_rpdoProcessedData = &rpdoProcessedDataShared;
			break;
		}
	}

	/**
	 * @ingroup mco_app_spec
	 * @brief Configures service on server that responds to processed RPDO messages.
	 */
	RpdoService(BoostConverter* _converter)
	{
		switch (Ipc)
		{
		case mcu::IPC_MODE_SINGLECORE:
			s_rpdoProcessedData = &rpdoProcessedDataNonShared;
			break;
		case mcu::IPC_MODE_DUALCORE:
			s_rpdoProcessedData = &rpdoProcessedDataShared;
			break;
		}

		// APP-SPECIFIC BEGIN
		converter = _converter;
		// APP-SPECIFIC END
	}

public:
	/**
	 * @brief Configures IPC signals.
	 * @return (none)
	 */
	void initIpcSignals(const mcu::IpcSignalPair& signalPairRpdo1Received,
			const mcu::IpcSignalPair& signalPairRpdo2Received,
			const mcu::IpcSignalPair& signalPairRpdo3Received,
			const mcu::IpcSignalPair& signalPairRpdo4Received)
	{
		RPDO1_RECEIVED = signalPairRpdo1Received;
		RPDO2_RECEIVED = signalPairRpdo2Received;
		RPDO3_RECEIVED = signalPairRpdo3Received;
		RPDO4_RECEIVED = signalPairRpdo4Received;
	}

	/**
	 * @ingroup mco_app_spec
	 * @brief Processes RPDO1 message. Used by McoServer's ISR as callback.
	 * @param rawMsg - RPDO1 message raw data.
	 * @return (none)
	 */
	void processRpdo1(uint64_t rawMsg)
	{
		EMB_STATIC_ASSERT(Mode != emb::MODE_SLAVE);
		CobRpdo1 pdoMsg(rawMsg);
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case MCO_CAN1:
			s_rpdoProcessedData->bitRun = pdoMsg.can1.run;
			s_rpdoProcessedData->bitEmergencyStop = pdoMsg.can1.emergencyStop;
			break;
		case MCO_CAN2:
			// RESERVED
			break;
		}
		// APP-SPECIFIC END
		mcu::sendIpcSignal(RPDO1_RECEIVED.local);
	}

	/**
	 * @ingroup mco_app_spec
	 * @brief Processes RPDO2 message. Used by McoServer's ISR as callback.
	 * @param rawMsg - RPDO2 message raw data.
	 * @return (none)
	 */
	void processRpdo2(uint64_t rawMsg)
	{
		EMB_STATIC_ASSERT(Mode != emb::MODE_SLAVE);
		CobRpdo2 pdoMsg(rawMsg);
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case MCO_CAN1:
			s_rpdoProcessedData->speedRef = RpdoService::speedRef(pdoMsg);
			s_rpdoProcessedData->torquePuRef = RpdoService::torquePuRef(pdoMsg);
			break;
		case MCO_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::sendIpcSignal(RPDO2_RECEIVED.local);
	}

	/**
	 * @ingroup mco_app_spec
	 * @brief Processes RPDO3 message. Used by McoServer's ISR as callback.
	 * @param rawMsg - RPDO3 message raw data.
	 * @return (none)
	 */
	void processRpdo3(uint64_t rawMsg)
	{
		EMB_STATIC_ASSERT(Mode != emb::MODE_SLAVE);
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case MCO_CAN1:
			// RESERVED
			break;
		case MCO_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::sendIpcSignal(RPDO3_RECEIVED.local);
	}

	/**
	 * @ingroup mco_app_spec
	 * @brief Processes RPDO4 message. Used by McoServer's ISR as callback.
	 * @param rawMsg - RPDO4 message raw data.
	 * @return (none)
	 */
	void processRpdo4(uint64_t rawMsg)
	{
		EMB_STATIC_ASSERT(Mode != emb::MODE_SLAVE);
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case MCO_CAN1:
			// RESERVED
			break;
		case MCO_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::sendIpcSignal(RPDO4_RECEIVED.local);
	}

public:
	/**
	 * @ingroup mco_app_spec
	 * @brief Checks if there is processed new RPDO data placed in static structure and utilizes it. Used by McoServer's run().
	 * @param (none)
	 * @return (none)
	 */
	void respondToProcessedRpdo()
	{
		_respondToProcessedRpdo1();
		_respondToProcessedRpdo2();
		_respondToProcessedRpdo3();
		_respondToProcessedRpdo4();
	}

private:
	///
	///
	///
	void _respondToProcessedRpdo1()
	{
		if (!mcu::ipcSignalSent(RPDO1_RECEIVED, Ipc)) return;
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case MCO_CAN1:
			mcu::SystemClock::resetWatchdogTimer();	// phew! CAN bus is OK
			if (s_rpdoProcessedData->bitRun == true)
			{
				converter->start();
			}
			else
			{
				converter->stop();
			}
			break;
		case MCO_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::resetIpcSignal(RPDO1_RECEIVED, Ipc);
	}

	///
	///
	///
	void _respondToProcessedRpdo2()
	{
		if (!mcu::ipcSignalSent(RPDO2_RECEIVED, Ipc)) return;
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case MCO_CAN1:
			// RESERVED;
			break;
		case MCO_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::resetIpcSignal(RPDO2_RECEIVED, Ipc);
	}

	///
	///
	///
	void _respondToProcessedRpdo3()
	{
		if (!mcu::ipcSignalSent(RPDO3_RECEIVED, Ipc)) return;
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case MCO_CAN1:
			// RESERVED
			break;
		case MCO_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::resetIpcSignal(RPDO3_RECEIVED, Ipc);
	}

	///
	///
	///
	void _respondToProcessedRpdo4()
	{
		if (!mcu::ipcSignalSent(RPDO4_RECEIVED, Ipc)) return;
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case MCO_CAN1:
			// RESERVED
			break;
		case MCO_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::resetIpcSignal(RPDO4_RECEIVED, Ipc);
	}

/* ========================================================================== */
/* ======================= APPLICATION-SPECIFIC BEGIN ======================= */
/* ========================================================================== */
private:
	/**
	 * @ingroup mco_app_spec
	 * @brief Returns speed reference from COB RPDO2.
	 * @param - COB RPDO2
	 * @return Speed reference.
	 */
	static float speedRef(const CobRpdo2& msg) { return msg.can1.speed; }

	/**
	 * @ingroup mco_app_spec
	 * @brief Returns torque per-unit reference from COB RPDO2.
	 * @param - COB RPDO2
	 * @return Torque per-unit reference.
	 */
	static float torquePuRef(const CobRpdo2& msg) { return msg.can1.torque; }
/* ========================================================================== */
/* ======================== APPLICATION-SPECIFIC END ======================== */
/* ========================================================================== */
};


/// @}
} // namespace microcanopen


