/**
 * @file
 * @ingroup microcanopen mco_rpdo_service
 */


#include "rpdoservice.h"


namespace microcanopen {

/// Common IPC data storage for all MCO servers.
#ifdef DUALCORE
#pragma DATA_SECTION("SHARED_MCO_RPDO_DATA")
ProcessedRpdoData rpdoProcessedData;
#else
ProcessedRpdoData rpdoProcessedData;
#endif

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART BEGIN ====================== */
/* ========================================================================== */
#ifdef CPU1
#if (defined(ACIM_MOTOR_SIX_PHASE))
#define DRIVE(x) RpdoService<x>::drive6Ph
#define DRIVE2(x) static_cast<acim::Drive<acim::SIX_PHASE, acim::DRIVE_INSTANCE_1>*>(NULL)
#elif (defined(ACIM_MOTOR_THREE_PHASE))
#define DRIVE(x) RpdoService<x>::drive3Ph_1
#define DRIVE2(x) RpdoService<x>::drive3Ph_2
#elif (defined(ACIM_TWO_MOTORS))
#define DRIVE(x) RpdoService<x>::drive3Ph_1
#define DRIVE2(x) RpdoService<x>::drive3Ph_2
#endif
#endif


///
///
///
template <mcu::CanModule Module>
void RpdoService<Module>::processRpdo1(uint64_t rawMsg)
{
#if (!defined(DUALCORE) || defined(CPU2))
	CobRpdo1 pdoMsg(rawMsg);
	// APP-SPECIFIC BEGIN
	switch (Module)
	{
	case MCO_CAN1:
		rpdoProcessedData.bitRun = pdoMsg.can1.run;
		rpdoProcessedData.bitEmergencyStop = pdoMsg.can1.emergencyStop;
		break;
	case MCO_CAN2:
		// RESERVED
		break;
	}
	// APP-SPECIFIC END
	mcu::sendIpcSignal(RPDO1_RECEIVED->local);
#else
	while (true) {} // CPU1 in DUALCORE build must not get here
#endif
}
// Explicit instantiation
template void RpdoService<mcu::CANA>::processRpdo1(uint64_t rawMsg);
template void RpdoService<mcu::CANB>::processRpdo1(uint64_t rawMsg);

///
///
///
template <mcu::CanModule Module>
void RpdoService<Module>::processRpdo2(uint64_t rawMsg)
{
#if (!defined(DUALCORE) || defined(CPU2))
	CobRpdo2 pdoMsg(rawMsg);
	// APP-SPECIFIC BEGIN
	switch (Module)
	{
	case MCO_CAN1:
		rpdoProcessedData.speedRef = RpdoService::speedRef(pdoMsg);
		rpdoProcessedData.torquePuRef = RpdoService::torquePuRef(pdoMsg);
		break;
	case MCO_CAN2:
		// RESERVED;
		break;
	}
	// APP-SPECIFIC END
	mcu::sendIpcSignal(RPDO2_RECEIVED->local);
#else
	while (true) {} // CPU1 in DUALCORE build must not get here
#endif
}
// Explicit instantiation
template void RpdoService<mcu::CANA>::processRpdo2(uint64_t rawMsg);
template void RpdoService<mcu::CANB>::processRpdo2(uint64_t rawMsg);

///
///
///
template <mcu::CanModule Module>
void RpdoService<Module>::processRpdo3(uint64_t rawMsg)
{
#if (!defined(DUALCORE) || defined(CPU2))
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
	mcu::sendIpcSignal(RPDO3_RECEIVED->local);
#else
	while (true) {} // CPU1 in DUALCORE build must not get here
#endif
}
// Explicit instantiation
template void RpdoService<mcu::CANA>::processRpdo3(uint64_t rawMsg);
template void RpdoService<mcu::CANB>::processRpdo3(uint64_t rawMsg);

///
///
///
template <mcu::CanModule Module>
void RpdoService<Module>::processRpdo4(uint64_t rawMsg)
{
#if (!defined(DUALCORE) || defined(CPU2))
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
	mcu::sendIpcSignal(RPDO4_RECEIVED->local);
#else
	while (true) {} // CPU1 in DUALCORE build must not get here
#endif
}
// Explicit instantiation
template void RpdoService<mcu::CANA>::processRpdo4(uint64_t rawMsg);
template void RpdoService<mcu::CANB>::processRpdo4(uint64_t rawMsg);


#ifdef CPU1
///
///
///
template <mcu::CanModule Module>
void RpdoService<Module>::respondToProcessedRpdo()
{
	_respondToProcessedRpdo1();
	_respondToProcessedRpdo2();
	_respondToProcessedRpdo3();
	_respondToProcessedRpdo4();
}
// Explicit instantiation
template void RpdoService<mcu::CANA>::respondToProcessedRpdo();
template void RpdoService<mcu::CANB>::respondToProcessedRpdo();

///
///
///
template <mcu::CanModule Module>
void RpdoService<Module>::_respondToProcessedRpdo1()
{
#ifdef DUALCORE
	if (!mcu::remoteIpcSignalSent(RPDO1_RECEIVED->remote)) return;
#else
	if (!mcu::localIpcSignalSent(RPDO1_RECEIVED->local)) return;
#endif
	// APP-SPECIFIC BEGIN
	switch (Module)
	{
	case MCO_CAN1:
//		mcu::Clock::resetWatchdogTimer();	// phew! CAN bus is OK
//		if (rpdoProcessedData.bitEmergencyStop == true)
//		{
//			DRIVE(Module)->emergencyStop();
//			if (DRIVE2(Module) != NULL)
//			{
//				DRIVE2(Module)->emergencyStop();
//			}
//		}
//		else
//		{
//			if (rpdoProcessedData.bitRun == true)
//			{
//				DRIVE(Module)->start();
//				if (DRIVE2(Module) != NULL)
//				{
//					DRIVE2(Module)->start();
//				}
//			}
//			else
//			{
//				DRIVE(Module)->stop();
//				if (DRIVE2(Module) != NULL)
//				{
//					DRIVE2(Module)->stop();
//				}
//			}
//		}
		break;
	case MCO_CAN2:
		// RESERVED;
		break;
	}
	// APP-SPECIFIC END
#ifdef DUALCORE
	mcu::acknowledgeRemoteIpcSignal(RPDO1_RECEIVED->remote);
#else
	mcu::revokeLocalIpcSignal(RPDO1_RECEIVED->local);
#endif
}

///
///
///
template <mcu::CanModule Module>
void RpdoService<Module>::_respondToProcessedRpdo2()
{
#ifdef DUALCORE
	if (!mcu::remoteIpcSignalSent(RPDO2_RECEIVED->remote)) return;
#else
	if (!mcu::localIpcSignalSent(RPDO2_RECEIVED->local)) return;
#endif
	// APP-SPECIFIC BEGIN
	switch (Module)
	{
	case MCO_CAN1:
//		DRIVE(Module)->setSpeedRef(rpdoProcessedData.speedRef);
//		DRIVE(Module)->setTorqueRef(rpdoProcessedData.torquePuRef);
		break;
	case MCO_CAN2:
		// RESERVED;
		break;
	}
	// APP-SPECIFIC END
#ifdef DUALCORE
	mcu::acknowledgeRemoteIpcSignal(RPDO2_RECEIVED->remote);
#else
	mcu::revokeLocalIpcSignal(RPDO2_RECEIVED->local);
#endif
}

///
///
///
template <mcu::CanModule Module>
void RpdoService<Module>::_respondToProcessedRpdo3()
{
#ifdef DUALCORE
	if (!mcu::remoteIpcSignalSent(RPDO3_RECEIVED->remote)) return;
#else
	if (!mcu::localIpcSignalSent(RPDO3_RECEIVED->local)) return;
#endif
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
#ifdef DUALCORE
	mcu::acknowledgeRemoteIpcSignal(RPDO3_RECEIVED->remote);
#else
	mcu::revokeLocalIpcSignal(RPDO3_RECEIVED->local);
#endif
}

///
///
///
template <mcu::CanModule Module>
void RpdoService<Module>::_respondToProcessedRpdo4()
{
#ifdef DUALCORE
	if (!mcu::remoteIpcSignalSent(RPDO4_RECEIVED->remote)) return;
#else
	if (!mcu::localIpcSignalSent(RPDO4_RECEIVED->local)) return;
#endif
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
#ifdef DUALCORE
	mcu::acknowledgeRemoteIpcSignal(RPDO4_RECEIVED->remote);
#else
	mcu::revokeLocalIpcSignal(RPDO4_RECEIVED->local);
#endif
}


#endif
/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART END ======================== */
/* ========================================================================== */


} // namespace microcanopen


