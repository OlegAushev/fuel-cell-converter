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
#include "mcu/ipc/mcuipc.h"
#include "mcu/gpio/mcugpio.h"
#include "mcu/cputimers/mcucputimers.h"


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
	uint16_t run : 1;
	uint32_t reserved1 : 31;
	uint16_t emergencyStop : 1;
	uint32_t reserved2 : 31;
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
	float32 speed;
	float32 torque;
	CobRpdo2()
	{
		uint64_t rawMsg = 0;
		memcpy(this, &rawMsg, sizeof(CobRpdo2));
	}
	CobRpdo2(uint64_t rawMsg) { memcpy(this, &rawMsg, sizeof(CobRpdo2)); }
};

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART END ======================== */
/* ========================================================================== */

/**
 * @brief RPDO-service class.
 */
class RpdoService
{
	friend class RpdoServiceTest;
private:
	RpdoService(const RpdoService& other);			// no copy constructor
	RpdoService& operator=(const RpdoService& other);	// no copy assignment operator
public:
	/**
	 * @ingroup mco_app_spec
	 * @brief Processes RPDO1 message. Used by McoServer as callback.
	 * @param rawMsg - RPDO1 message raw data.
	 * @return (none)
	 */
	static void processRpdo1(uint64_t rawMsg);

	/**
	 * @ingroup mco_app_spec
	 * @brief Processes RPDO2 message. Used by McoServer as callback.
	 * @param rawMsg - RPDO2 message raw data.
	 * @return (none)
	 */
	static void processRpdo2(uint64_t rawMsg);

	/**
	 * @ingroup mco_app_spec
	 * @brief Processes RPDO3 message. Used by McoServer as callback.
	 * @param rawMsg - RPDO3 message raw data.
	 * @return (none)
	 */
	static void processRpdo3(uint64_t rawMsg);

	/**
	 * @ingroup mco_app_spec
	 * @brief Processes RPDO4 message. Used by McoServer as callback.
	 * @param rawMsg - RPDO4 message raw data.
	 * @return (none)
	 */
	static void processRpdo4(uint64_t rawMsg);
#ifdef CPU1
	/**
	 * @ingroup mco_app_spec
	 * @brief Checks if there is processed new RPDO data placed in static structure and utilizes it.
	 * @param (none)
	 * @return (none)
	 */
	void utilizeProcessedMessages();
#endif

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART BEGIN ====================== */
/* ========================================================================== */
public:
	/**
	 * @ingroup mco_app_spec
	 * @brief Configures service.
	 * @param (none)
	 */
	RpdoService()
	{

	}

	/**
	 * @ingroup mco_app_spec
	 * @brief Data storage for IPC.
	 */
	struct PdoData
	{
		bool bitRun;
		bool bitEmergencyStop;
		float speedRef;
		float torquePuRef;
	};
	static PdoData pdoData;

private:
	/**
	 * @ingroup mco_app_spec
	 * @brief Returns speed reference from COB RPDO2.
	 * @param - COB RPDO2
	 * @return Speed reference.
	 */
	static float speedRef(const CobRpdo2& msg) { return msg.speed; }

	/**
	 * @ingroup mco_app_spec
	 * @brief Returns torque per-unit reference from COB RPDO2.
	 * @param - COB RPDO2
	 * @return Torque per-unit reference.
	 */
	static float torquePuRef(const CobRpdo2& msg) { return msg.torque; }

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART END ======================== */
/* ========================================================================== */
};


/// @}
} // namespace microcanopen




