/**
 * @file
 * @ingroup microcanopen mco_rpdo_service
 */


#include "rpdoservice.h"


namespace microcanopen {


#ifdef DUALCORE
#pragma DATA_SECTION("CANRPDODATA")
RpdoService::PdoData RpdoService::pdoData;
#else
RpdoService::PdoData RpdoService::pdoData;
#endif

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART BEGIN ====================== */
/* ========================================================================== */


///
///
///
void RpdoService::processRpdo1(uint64_t rawMsg)
{
#if (!defined(DUALCORE) || defined(CPU2))
	CobRpdo1 pdoMsg(rawMsg);

	pdoData.bitRun = pdoMsg.run;
	pdoData.bitEmergencyStop = pdoMsg.emergencyStop;

	mcu::sendIpcSignal(CAN_RPDO1_RECEIVED);
#else
	while (true) {} // CPU1 in DUALCORE build must not get here
#endif
}

///
///
///
void RpdoService::processRpdo2(uint64_t rawMsg)
{
#if (!defined(DUALCORE) || defined(CPU2))
	CobRpdo2 pdoMsg(rawMsg);

	pdoData.speedRef = RpdoService::speedRef(pdoMsg);
	pdoData.torquePuRef = RpdoService::torquePuRef(pdoMsg);

	mcu::sendIpcSignal(CAN_RPDO2_RECEIVED);
#else
	while (true) {} // CPU1 in DUALCORE build must not get here
#endif
}

///
///
///
void RpdoService::processRpdo3(uint64_t rawMsg)
{
#if (!defined(DUALCORE) || defined(CPU2))
	mcu::sendIpcSignal(CAN_RPDO3_RECEIVED);
#else
	while (true) {} // CPU1 in DUALCORE build must not get here
#endif
}

///
///
///
void RpdoService::processRpdo4(uint64_t rawMsg)
{
#if (!defined(DUALCORE) || defined(CPU2))
	mcu::sendIpcSignal(CAN_RPDO4_RECEIVED);
	mcu::sendIpcSignal(CAN_FRAME_RECEIVED);
#else
	while (true) {} // CPU1 in DUALCORE build must not get here
#endif
}


} // namespace microcanopen



#ifdef CPU1
///
///
///
void microcanopen::RpdoService::utilizeProcessedMessages()
{
	if (mcu::isIpcSignalSet(CAN_RPDO1_RECEIVED))
	{
		mcu::Clock::resetWatchdogTimer();	// phew! CAN bus is OK
				mcu::acknowledgeIpcSignal(CAN_RPDO1_RECEIVED);
	}

	if (mcu::isIpcSignalSet(CAN_RPDO2_RECEIVED))
	{
		mcu::acknowledgeIpcSignal(CAN_RPDO2_RECEIVED);
	}

	if (mcu::isIpcSignalSet(CAN_RPDO3_RECEIVED))
	{
		mcu::acknowledgeIpcSignal(CAN_RPDO3_RECEIVED);
	}

	if (mcu::isIpcSignalSet(CAN_RPDO4_RECEIVED))
	{
		mcu::acknowledgeIpcSignal(CAN_RPDO4_RECEIVED);
	}
}

#endif
/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART END ======================== */
/* ========================================================================== */





























