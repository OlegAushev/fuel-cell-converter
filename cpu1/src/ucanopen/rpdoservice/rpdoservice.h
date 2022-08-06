/**
 * @defgroup ucanopen_rpdo_service RPDO Service
 * @ingroup ucanopen
 *
 * @file
 * @ingroup ucanopen ucanopen_rpdo_service
 */


#pragma once


#include "driverlib.h"
#include "device.h"

#include "emb/emb_common.h"
#include "../ucanopen_def.h"
#include "mcu/can/mcu_can.h"
#include "mcu/ipc/mcu_ipc.h"
#include "mcu/gpio/mcu_gpio.h"
#include "mcu/cputimers/mcu_cputimers.h"

// APP-SPECIFIC headers
#include "fuelcell/converter/fuelcell_converter.h"


namespace ucanopen {
/// @addtogroup ucanopen_rpdo_service
/// @{


/* ========================================================================== */
/* ======================= APPLICATION-SPECIFIC BEGIN ======================= */
/* ========================================================================== */

/**
 * @ingroup ucanopen_app_spec
 * @brief RPDO1 message data.
 */
struct CobRpdo1
{
	union
	{
		struct
		{
			uint32_t reserved1 : 32;
			uint32_t reserved2 : 32;
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
 * @ingroup ucanopen_app_spec
 * @brief RPDO2 message data.
 */
struct CobRpdo2
{
	union
	{
		struct
		{
			uint32_t reserved1 : 3;
			uint32_t chargeEn : 1;
			uint32_t reserved2 : 28;
			uint32_t reserved3 : 10;
			uint32_t fatalError : 1;
			uint32_t reserved4 : 21;
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
 * @ingroup ucanopen_app_spec
 * @brief RPDO3 message data.
 */
struct CobRpdo3
{
	union
	{
		struct
		{
			uint32_t chargeEn : 1;
			uint32_t reserved1 : 31;
			uint32_t fatalError : 1;
			uint32_t reserved3 : 31;
		} can1;
		struct
		{
			uint64_t reserved;
		} can2;
	};
	CobRpdo3()
	{
		uint64_t rawMsg = 0;
		memcpy(this, &rawMsg, sizeof(CobRpdo1));
	}
	CobRpdo3(uint64_t rawMsg) { memcpy(this, &rawMsg, sizeof(CobRpdo1)); }
};



/**
 * @ingroup ucanopen_app_spec
 * @brief Data storage for IPC.
 */
struct ProcessedRpdoData
{
	bool bitBmsChargeEn;
	bool bitBmsChargeEnDebug;
	bool bitBmsFatalError;
	bool bitBmsFatalErrorDebug;
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
	fuelcell::Converter* converter;
private:
	mcu::IpcFlag RPDO1_RECEIVED;
	mcu::IpcFlag RPDO2_RECEIVED;
	mcu::IpcFlag RPDO3_RECEIVED;
	mcu::IpcFlag RPDO4_RECEIVED;

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
	 * @ingroup ucanopen_app_spec
	 * @brief Configures service on server that responds to processed RPDO messages.
	 */
	RpdoService(fuelcell::Converter* _converter)
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
	 * @brief Configures IPC flags.
	 * @return (none)
	 */
	void initIpcFlags(const mcu::IpcFlag& flagRpdo1Received,
			const mcu::IpcFlag& flagRpdo2Received,
			const mcu::IpcFlag& flagRpdo3Received,
			const mcu::IpcFlag& flagRpdo4Received)
	{
		RPDO1_RECEIVED = flagRpdo1Received;
		RPDO2_RECEIVED = flagRpdo2Received;
		RPDO3_RECEIVED = flagRpdo3Received;
		RPDO4_RECEIVED = flagRpdo4Received;
	}

	/**
	 * @brief Returns reference to processed RPDO data structure.
	 * @param (none)
	 * @return Reference to processed RPDO data structure.
	 */
	ProcessedRpdoData& data()
	{
		return *s_rpdoProcessedData;
	}

	/**
	 * @ingroup ucanopen_app_spec
	 * @brief Processes RPDO1 message. Used by Server's ISR as callback.
	 * @param rawMsg - RPDO1 message raw data.
	 * @return (none)
	 */
	void processRpdo1(uint64_t rawMsg)
	{
		assert(Mode != emb::MODE_SLAVE);
		CobRpdo1 pdoMsg(rawMsg);
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case UCANOPEN_CAN1:
			// RESERVED;
			break;
		case UCANOPEN_CAN2:
			// RESERVED
			break;
		}
		// APP-SPECIFIC END
		mcu::setLocalIpcFlag(RPDO1_RECEIVED.local);
	}

	/**
	 * @ingroup ucanopen_app_spec
	 * @brief Processes RPDO2 message. Used by Server's ISR as callback.
	 * @param rawMsg - RPDO2 message raw data.
	 * @return (none)
	 */
	void processRpdo2(uint64_t rawMsg)
	{
		assert(Mode != emb::MODE_SLAVE);
		CobRpdo2 pdoMsg(rawMsg);
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case UCANOPEN_CAN1:
			data().bitBmsChargeEn = pdoMsg.can1.chargeEn;
			data().bitBmsFatalError = pdoMsg.can1.fatalError;
			break;
		case UCANOPEN_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::setLocalIpcFlag(RPDO2_RECEIVED.local);
	}

	/**
	 * @ingroup ucanopen_app_spec
	 * @brief Processes RPDO3 message. Used by Server's ISR as callback.
	 * @param rawMsg - RPDO3 message raw data.
	 * @return (none)
	 */
	void processRpdo3(uint64_t rawMsg)
	{
		assert(Mode != emb::MODE_SLAVE);
		CobRpdo3 pdoMsg(rawMsg);
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case UCANOPEN_CAN1:
			data().bitBmsChargeEnDebug = pdoMsg.can1.chargeEn;
			data().bitBmsFatalErrorDebug = pdoMsg.can1.fatalError;
			break;
		case UCANOPEN_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::setLocalIpcFlag(RPDO3_RECEIVED.local);
	}

	/**
	 * @ingroup ucanopen_app_spec
	 * @brief Processes RPDO4 message. Used by Server's ISR as callback.
	 * @param rawMsg - RPDO4 message raw data.
	 * @return (none)
	 */
	void processRpdo4(uint64_t rawMsg)
	{
		assert(Mode != emb::MODE_SLAVE);
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case UCANOPEN_CAN1:
			// RESERVED
			break;
		case UCANOPEN_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::setLocalIpcFlag(RPDO4_RECEIVED.local);
	}

public:
	/**
	 * @ingroup ucanopen_app_spec
	 * @brief Checks if there is processed new RPDO data placed in static structure and utilizes it. Used by Server's run().
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
		if (!mcu::isIpcFlagSet(RPDO1_RECEIVED, Ipc)) return;
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case UCANOPEN_CAN1:
			// RESERVED;
			break;
		case UCANOPEN_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::resetIpcFlag(RPDO1_RECEIVED, Ipc);
	}

	///
	///
	///
	void _respondToProcessedRpdo2()
	{
		if (!mcu::isIpcFlagSet(RPDO2_RECEIVED, Ipc)) return;
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case UCANOPEN_CAN1:
			mcu::SystemClock::resetWatchdogTimer();	// phew! CAN bus is OK
			if (data().bitBmsFatalError == true)
			{
				converter->emergencyShutdown();
			}
			else
			{
				if (data().bitBmsChargeEn == true)
				{
					converter->startCharging();
				}
				else
				{
					converter->stopCharging();
				}
			}
			break;
		case UCANOPEN_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::resetIpcFlag(RPDO2_RECEIVED, Ipc);
	}

	///
	///
	///
	void _respondToProcessedRpdo3()
	{
		if (!mcu::isIpcFlagSet(RPDO3_RECEIVED, Ipc)) return;
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case UCANOPEN_CAN1:
			mcu::SystemClock::resetWatchdogTimer();	// phew! CAN bus is OK
			if (data().bitBmsFatalErrorDebug == true)
			{
				converter->emergencyShutdown();
			}
			else
			{
				if (data().bitBmsChargeEnDebug == true)
				{
					converter->startCharging();
				}
				else
				{
					converter->stopCharging();
				}
			}
			break;
		case UCANOPEN_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::resetIpcFlag(RPDO3_RECEIVED, Ipc);
	}

	///
	///
	///
	void _respondToProcessedRpdo4()
	{
		if (!mcu::isIpcFlagSet(RPDO4_RECEIVED, Ipc)) return;
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case UCANOPEN_CAN1:
			// RESERVED
			break;
		case UCANOPEN_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		mcu::resetIpcFlag(RPDO4_RECEIVED, Ipc);
	}

/* ========================================================================== */
/* ======================= APPLICATION-SPECIFIC BEGIN ======================= */
/* ========================================================================== */
private:
	/**
	 * @ingroup ucanopen_app_spec
	 * @brief Returns speed reference from COB RPDO2.
	 * @param - COB RPDO2
	 * @return Speed reference.
	 */
	//static float speedRef(const CobRpdo2& msg) { return msg.can1.speed; }

	/**
	 * @ingroup ucanopen_app_spec
	 * @brief Returns torque per-unit reference from COB RPDO2.
	 * @param - COB RPDO2
	 * @return Torque per-unit reference.
	 */
	//static float torquePuRef(const CobRpdo2& msg) { return msg.can1.torque; }
/* ========================================================================== */
/* ======================== APPLICATION-SPECIFIC END ======================== */
/* ========================================================================== */
};


/// @}
} // namespace ucanopen


