/**
 * @file
 * @ingroup microcanopen mco_tpdo_service
 */


#include "tpdoservice.h"


namespace microcanopen {

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART BEGIN ====================== */
/* ========================================================================== */


///
///
///
uint64_t TpdoService::makeTpdo1()
{
	CobTpdo1 msg;
//
//	TpdoService::saveDriveState(msg, drive->state());
//	TpdoService::saveRunStatus(msg, drive->converter.state());
//	TpdoService::saveFaultStatus(msg, Syslog::faults());
//	TpdoService::saveWarningStatus(msg, Syslog::warnings());
//	// TODO tpdoService.saveOverheatStatus(msg,
//	TpdoService::saveReferenceType(msg, drive->model.reference());
//	TpdoService::saveControlLoopType(msg, drive->model.controlLoopType());

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
	return msg.all();
}

///
///
///
uint64_t TpdoService::makeTpdo2()
{
	CobTpdo2 msg;
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
	return msg.all();
}

///
///
///
uint64_t TpdoService::makeTpdo3()
{
	CobTpdo3 msg;
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
	return msg.all();
}

///
///
///
uint64_t TpdoService::makeTpdo4()
{
	CobTpdo4 msg;

	TpdoService::saveFaults(msg, Syslog::faults());
	TpdoService::saveWarnings(msg, Syslog::warnings());

	return msg.all();
}

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART END ======================== */
/* ========================================================================== */

} // namespace microcanopen




