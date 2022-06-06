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
//		saveDriveState(msg, DRIVE(Module)->state());
//		saveRunStatus(msg, DRIVE(Module)->converter.state());
//		saveFaultStatus(msg, Syslog::faults());
//		saveWarningStatus(msg, Syslog::warnings());
//		// TODO tpdoService.saveOverheatStatus(msg,
//		saveReferenceType(msg, DRIVE(Module)->model.reference());
//		saveControlLoopType(msg, DRIVE(Module)->model.controlLoopType());
		/* TODO
		tpdoService.saveTorquePU(msg, m_drive->model.torque() * m_drive->positionSensor.signOfPositiveDirection() / m_drive->model.torqueMax());
		tpdoService.saveSpeed(msg, m_drive->positionSensor.speed(srm::PositionSensor::CAPTURE_BASIS).rpm() * m_drive->positionSensor.signOfPositiveDirection());
		tpdoService.saveStatorCurrent(msg, m_drive->model.currentPhaseRms());
		tpdoService.savePower(msg, m_drive->powerMech());
		tpdoService.saveDcVoltage(msg, m_drive->converter.voltageDC());
		tpdoService.saveFieldCurrent(msg, m_drive->converter.currents()[srm::CURRENT_FIELD]);
		tpdoService.saveRunStatus(msg, m_drive->converter.state());
		tpdoService.saveFaultStatus(msg, syslog.faults());
		tpdoService.saveWarningStatus(msg, syslog.warnings());
		tpdoService.saveOverheatStatus(msg,
				syslog.hasWarning(Warning::JUNCTION_OVERHEATING)
				|| syslog.hasWarning(Warning::CASE_AIR_OVERHEATING)
				|| syslog.hasWarning(Warning::STATOR_OVERHEATING)
				|| syslog.hasWarning(Warning::FIELD_WINDING_OVERHEATING));
		*/
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
		/* TODO
		tpdoService.saveMotorTemp(msg, m_drive->motor.temperature(srm::Motor::STATOR));
		tpdoService.saveOutputVoltagePU(msg, m_drive->model.pwmDutyCycle());
		tpdoService.saveTorqueLimitationStatus(msg, 0);	// TODO
		tpdoService.saveFwMotorTemp(msg, m_drive->motor.temperature(srm::Motor::FIELD_WINDING));
		#ifdef CRD300
		tpdoService.saveHeatsinkTemp(msg, m_drive->converter.temperature(srm::Converter::MODULE_PHASE_B));
		#else
		tpdoService.saveHeatsinkTemp(msg, m_drive->converter.temperature(srm::Converter::AIR));
		#endif
		tpdoService.saveCaseAirTemp(msg, m_drive->converter.temperature(srm::Converter::AIR));
		*/
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
		/* TODO
		tpdoService.savePosHousingVoltage(msg, m_drive->converter.voltageDC()/2);
		tpdoService.saveNegHousingVoltage(msg, m_drive->converter.voltageDC()/2);
		tpdoService.saveInsulationLowStatus(msg, 0);
		tpdoService.saveInsulationLowStatusWoFilter(msg, 0);
		tpdoService.saveDcCurrent(msg, m_drive->converter.currents()[srm::CURRENT_DC]);

		Syslog syslog;
		msg.syslogInfo = static_cast<uint32_t>(syslog.readMessage());
		syslog.popMessage();
		*/
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




