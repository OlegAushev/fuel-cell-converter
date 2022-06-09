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
#include "boostconverter/boostconverter.h"


namespace microcanopen {
/// @addtogroup mco_rpdo_service
/// @{


/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART BEGIN ====================== */
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

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART END ======================== */
/* ========================================================================== */

/**
 * @brief RPDO-service class.
 */
template <mcu::CanModule Module>
class RpdoService
{
	friend class RpdoServiceTest;
private:
	RpdoService(const RpdoService& other);			// no copy constructor
	RpdoService& operator=(const RpdoService& other);	// no copy assignment operator

	mcu::IpcSignalPair* RPDO1_RECEIVED;
	mcu::IpcSignalPair* RPDO2_RECEIVED;
	mcu::IpcSignalPair* RPDO3_RECEIVED;
	mcu::IpcSignalPair* RPDO4_RECEIVED;

public:
	/**
	 * @brief Configures IPC signals.
	 * @return (none)
	 */
	void initIpcSignals(mcu::IpcSignalPair* signalPairRpdo1Received,
			mcu::IpcSignalPair* signalPairRpdo2Received,
			mcu::IpcSignalPair* signalPairRpdo3Received,
			mcu::IpcSignalPair* signalPairRpdo4Received)
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
	void processRpdo1(uint64_t rawMsg);

	/**
	 * @ingroup mco_app_spec
	 * @brief Processes RPDO2 message. Used by McoServer's ISR as callback.
	 * @param rawMsg - RPDO2 message raw data.
	 * @return (none)
	 */
	void processRpdo2(uint64_t rawMsg);

	/**
	 * @ingroup mco_app_spec
	 * @brief Processes RPDO3 message. Used by McoServer's ISR as callback.
	 * @param rawMsg - RPDO3 message raw data.
	 * @return (none)
	 */
	void processRpdo3(uint64_t rawMsg);

	/**
	 * @ingroup mco_app_spec
	 * @brief Processes RPDO4 message. Used by McoServer's ISR as callback.
	 * @param rawMsg - RPDO4 message raw data.
	 * @return (none)
	 */
	void processRpdo4(uint64_t rawMsg);

#ifdef CPU1
public:
	/**
	 * @ingroup mco_app_spec
	 * @brief Checks if there is processed new RPDO data placed in static structure and utilizes it. Used by McoServer's run().
	 * @param (none)
	 * @return (none)
	 */
	void respondToProcessedRpdo();

private:
	void _respondToProcessedRpdo1();
	void _respondToProcessedRpdo2();
	void _respondToProcessedRpdo3();
	void _respondToProcessedRpdo4();
#endif

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART BEGIN ====================== */
/* ========================================================================== */
public:
#ifdef CPU2
	/**
	 * @brief Configures service on CPU2.
	 */
	RpdoService() {};
#endif

#ifdef CPU1
	/**
	 * @ingroup mco_app_spec
	 * @brief Configures service.
	 */
	RpdoService(BoostConverter* _converter)
	{
		converter = _converter;
	}

private:
	BoostConverter* converter;
#endif

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
/* =================== APPLICATION-SPECIFIC PART END ======================== */
/* ========================================================================== */
};


/// @}
} // namespace microcanopen


