/**
 * @file
 * @ingroup microcanopen mco_tpdo_service
 */


#include "tpdoservice.h"


namespace microcanopen {

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART BEGIN ====================== */
/* ========================================================================== */
#if (defined(ACIM_MOTOR_SIX_PHASE))
#define DRIVE(x) TpdoService<x>::drive6Ph
#elif (defined(ACIM_MOTOR_THREE_PHASE))
#define DRIVE(x) TpdoService<x>::drive3Ph_1
#elif (defined(ACIM_TWO_MOTORS))
#define DRIVE(x) TpdoService<x>::drive3Ph_1
#endif


///
///
///
template <mcu::CanModule Module>
uint64_t TpdoService<Module>::makeTpdo1()
{
	CobTpdo1 msg;
	// APP-SPECIFIC BEGIN
	switch (Module)
	{
	case MCO_CAN1:
		saveRunStatus(msg, converter->state());
		saveFaultStatus(msg, Syslog::faults());
		saveWarningStatus(msg, Syslog::warnings());
		break;
	case MCO_CAN2:
		// RESERVED;
		break;
	}
	// APP-SPECIFIC END
	return msg.all();
}
// Explicit instantiation
template uint64_t TpdoService<mcu::CANA>::makeTpdo1();
template uint64_t TpdoService<mcu::CANB>::makeTpdo1();

///
///
///
template <mcu::CanModule Module>
uint64_t TpdoService<Module>::makeTpdo2()
{
	CobTpdo2 msg;
	// APP-SPECIFIC BEGIN
	switch (Module)
	{
	case MCO_CAN1:
		// TODO
		break;
	case MCO_CAN2:
		// RESERVED;
		break;
	}
	// APP-SPECIFIC END
	return msg.all();
}
// Explicit instantiation
template uint64_t TpdoService<mcu::CANA>::makeTpdo2();
template uint64_t TpdoService<mcu::CANB>::makeTpdo2();

///
///
///
template <mcu::CanModule Module>
uint64_t TpdoService<Module>::makeTpdo3()
{
	CobTpdo3 msg;
	// APP-SPECIFIC BEGIN
	switch (Module)
	{
	case MCO_CAN1:
		// TODO
		break;
	case MCO_CAN2:
		// RESERVED;
		break;
	}
	// APP-SPECIFIC END
	return msg.all();
}
// Explicit instantiation
template uint64_t TpdoService<mcu::CANA>::makeTpdo3();
template uint64_t TpdoService<mcu::CANB>::makeTpdo3();

///
///
///
template <mcu::CanModule Module>
uint64_t TpdoService<Module>::makeTpdo4()
{
	CobTpdo4 msg;
	// APP-SPECIFIC BEGIN
	switch (Module)
	{
	case MCO_CAN1:
		saveFaults(msg, Syslog::faults());
		saveWarnings(msg, Syslog::warnings());
		break;
	case MCO_CAN2:
		// RESERVED;
		break;
	}
	// APP-SPECIFIC END
	return msg.all();
}
// Explicit instantiation
template uint64_t TpdoService<mcu::CANA>::makeTpdo4();
template uint64_t TpdoService<mcu::CANB>::makeTpdo4();

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART END ======================== */
/* ========================================================================== */

} // namespace microcanopen




