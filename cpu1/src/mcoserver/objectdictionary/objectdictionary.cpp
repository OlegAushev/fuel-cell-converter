/**
 * @file
 * @ingroup microcanopen mco_od
 */


#include "objectdictionary.h"


namespace microcanopen {

namespace od {

//acim::Drive<acim::SIX_PHASE, acim::DRIVE_INSTANCE_1>* drive6Ph =
//		static_cast<acim::Drive<acim::SIX_PHASE, acim::DRIVE_INSTANCE_1>*>(NULL);
//
//acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_1>* drive3Ph_1 =
//		static_cast<acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_1>*>(NULL);
//
//acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_2>* drive3Ph_2 =
//		static_cast<acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_2>*>(NULL);
//
//SetupManager* setupManager = static_cast<SetupManager*>(NULL);

#if (defined(ACIM_MOTOR_SIX_PHASE))
static acim::Drive<acim::SIX_PHASE, acim::DRIVE_INSTANCE_1>*& drive = drive6Ph;
static acim::Drive<acim::SIX_PHASE, acim::DRIVE_INSTANCE_1>*& drive2 = drive6Ph;
#elif (defined(ACIM_MOTOR_THREE_PHASE))
static acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_1>*& drive = drive3Ph_1;
static acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_2>*& drive2 = drive3Ph_2;
#elif (defined(ACIM_TWO_MOTORS))
static acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_1>*& drive = drive3Ph_1;
static acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_2>*& drive2 = drive3Ph_2;
#endif

/* ========================================================================== */
/* =================== INFO ====================== */
/* ========================================================================== */
inline ODAccessStatus getDeviceName(CobSdoData& dest)
{
	uint16_t name[4] = {0};
	memcpy(name, Syslog::DEVICE_NAME, sizeof(name)/sizeof(*name));
	uint32_t nameRaw = 0;
	emb::c28x::from_8bit_bytes<uint32_t>(nameRaw, name);
	memcpy(&dest, &nameRaw, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getSoftwareVersion(CobSdoData& dest)
{
	memcpy(&dest, &Syslog::SOFTWARE_VERSION, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getBuildConfiguration(CobSdoData& dest)
{
	uint16_t name[4];
	memcpy(name, Syslog::BUILD_CONFIGURATION, sizeof(name)/sizeof(*name));
	uint32_t nameRaw;
	emb::c28x::from_8bit_bytes<uint32_t>(nameRaw, name);
	memcpy(&dest, &nameRaw, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getSyslogMessage(CobSdoData& dest)
{
	uint32_t message = static_cast<uint32_t>(Syslog::readMessage());
	Syslog::popMessage();
	memcpy(&dest, &message, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

/* ========================================================================== */
/* =================== WATCH ====================== */
/* ========================================================================== */
inline ODAccessStatus getUptime(CobSdoData& dest)
{
	float time = mcu::SystemClock::now() / 1000.f;
	memcpy(&dest, &time, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}


#ifdef OBSOLETE

inline ODAccessStatus getDriveState(CobSdoData& dest)
{
	uint32_t state = static_cast<uint32_t>(drive->state());
	memcpy(&dest, &state, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getFault(CobSdoData& dest)
{
	uint32_t fault = Syslog::faults();
	memcpy(&dest, &fault, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveVoltageDc(CobSdoData& dest)
{
	float value = drive->converter.voltageDc();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveCurrentPhU(CobSdoData& dest)
{
	float value = drive->converter.current(crd600::PHASE_U);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveCurrentPhV(CobSdoData& dest)
{
	float value = drive->converter.current(crd600::PHASE_V);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveCurrentPhW(CobSdoData& dest)
{
	float value = drive->converter.current(crd600::PHASE_W);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveCurrentPhX(CobSdoData& dest)
{
	if (drive2 == NULL)
	{
		return OD_ACCESS_FAIL;
	}
	float value = drive2->converter.current(crd600::PHASE_X);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveCurrentPhY(CobSdoData& dest)
{
	if (drive2 == NULL)
	{
		return OD_ACCESS_FAIL;
	}
	float value = drive2->converter.current(crd600::PHASE_Y);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveCurrentPhZ(CobSdoData& dest)
{
	if (drive2 == NULL)
	{
		return OD_ACCESS_FAIL;
	}
	float value = drive2->converter.current(crd600::PHASE_Z);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveTempPhU(CobSdoData& dest)
{
	float value = drive->converter.temperatureModule(crd600::PHASE_U);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveTempPhV(CobSdoData& dest)
{
	float value = drive->converter.temperatureModule(crd600::PHASE_V);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveTempPhW(CobSdoData& dest)
{
	float value = drive->converter.temperatureModule(crd600::PHASE_W);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveTempPhX(CobSdoData& dest)
{
	if (drive2 == NULL)
	{
		return OD_ACCESS_FAIL;
	}
	float value = drive2->converter.temperatureModule(crd600::PHASE_X);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveTempPhY(CobSdoData& dest)
{
	if (drive2 == NULL)
	{
		return OD_ACCESS_FAIL;
	}
	float value = drive2->converter.temperatureModule(crd600::PHASE_Y);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveTempPhZ(CobSdoData& dest)
{
	if (drive2 == NULL)
	{
		return OD_ACCESS_FAIL;
	}
	float value = drive2->converter.temperatureModule(crd600::PHASE_Z);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveTempCase(CobSdoData& dest)
{
	float value = drive->converter.temperatureCase();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveCurrentS(CobSdoData& dest)
{
	float value = drive->model.currentPhaseRms();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveCurrentD(CobSdoData& dest)
{
	float value = drive->model.currentD();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveCurrentQ(CobSdoData& dest)
{
	float value = drive->model.currentQ();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveSpeedRpm(CobSdoData& dest)
{
	float value = drive->speed().to_rpm();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveTorque(CobSdoData& dest)
{
	float value = drive->torque();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDrivePowerMech(CobSdoData& dest)
{
	float value = drive->powerMech();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveVoltageD(CobSdoData& dest)
{
	float value = drive->model.voltageD();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveVoltageQ(CobSdoData& dest)
{
	float value = drive->model.voltageQ();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveFlux(CobSdoData& dest)
{
	float value = drive->model.flux();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDrivePowerElec(CobSdoData& dest)
{
	float value = drive->model.powerElec();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveTempMotorS(CobSdoData& dest)
{
	float value = 0; // TODO drive->motor.temperature(srm::Motor::STATOR);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveTempMotorFw(CobSdoData& dest)
{
	float value = 0; // TODO drive->motor.temperature(srm::Motor::FIELD_WINDING);
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getDriveGammaAngle(CobSdoData& dest)
{
	float value = embdd::toDeg(drive->model.gamma());
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}






/* ========================================================================== */
/* =================== SET VARIABLES ====================== */
/* ========================================================================== */
inline ODAccessStatus setDriveSpeed(CobSdoData val)
{
	drive->setSpeedRef(val.f32);
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus setDriveTorque(CobSdoData val)
{
	drive->setTorqueRef(val.f32);
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus setDriveOpenLoopAngle(CobSdoData val)
{
	drive->setAngleRef(val.f32);
//	if (drive2 != NULL && drive2->instanceId() != acim::DRIVE_INSTANCE_1)
//	{
//#warning "Test configuration."
//		drive2->setAngleRef(val.f32 + 180);
//	}
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus setDriveCurrentD(CobSdoData val)
{
	drive->setCurrentDRef(val.f32);
//	if (drive2 != NULL && drive2->instanceId() != acim::DRIVE_INSTANCE_1)
//	{
//#warning "Test configuration."
//		drive2->setCurrentDRef(val.f32);
//	}
	return OD_ACCESS_SUCCESS;
}

/* ========================================================================== */
/* =================== POWER UP/DOWN ====================== */
/* ========================================================================== */
inline ODAccessStatus drivePowerUp(CobSdoData& dest)
{
	uint32_t taskStatus;

	if (Syslog::faults() == 0)
	{
		drive->powerUp();
		if (drive2 != NULL && drive2->instanceId() != acim::DRIVE_INSTANCE_1)
		{
			drive2->powerUp();
		}
		taskStatus = TASK_SUCCESS;
	}
	else
	{
		taskStatus = TASK_FAIL;
	}

	memcpy(&dest, &taskStatus, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus drivePowerDown(CobSdoData& dest)
{
	uint32_t taskStatus;

	drive->powerDown();
	if (drive2 != NULL && drive2->instanceId() != acim::DRIVE_INSTANCE_1)
	{
		drive2->powerDown();
	}

	taskStatus = TASK_SUCCESS;
	memcpy(&dest, &taskStatus, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}


/* ========================================================================== */
/* =================== SETUP MANAGER ====================== */
/* ========================================================================== */
inline ODAccessStatus resetDevice(CobSdoData& dest)
{
	uint32_t taskStatus;

	if (setupManager->resetDevice() == SetupManager::REQUEST_PROCESSED)
	{
		taskStatus = TASK_SUCCESS;
	}
	else
	{
		taskStatus = TASK_FAIL;
	}

	memcpy(&dest, &taskStatus, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus resetParametersToDefault(CobSdoData& dest)
{
	uint32_t taskStatus;

	if (setupManager->resetParametersToDefault() == SetupManager::REQUEST_PROCESSED)
	{
		taskStatus = TASK_SUCCESS;
	}
	else
	{
		taskStatus = TASK_FAIL;
	}

	memcpy(&dest, &taskStatus, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus applyParameters(CobSdoData& dest)
{
	uint32_t taskStatus;

	if (setupManager->applyParameters() == SetupManager::REQUEST_PROCESSED)
	{
		taskStatus = TASK_SUCCESS;
	}
	else
	{
		taskStatus = TASK_FAIL;
	}

	memcpy(&dest, &taskStatus, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus resetAllFaults(CobSdoData& dest)
{
	uint32_t taskStatus;

	if (setupManager->resetAllFaults() == SetupManager::REQUEST_PROCESSED)
	{
		taskStatus = TASK_SUCCESS;
	}
	else
	{
		taskStatus = TASK_FAIL;
	}

	memcpy(&dest, &taskStatus, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

/* ========================================================================== */
/* =================== MRAS ====================== */
/* ========================================================================== */
inline ODAccessStatus getMrasKp(CobSdoData& dest)
{
	float value = drive->model.m_mras.m_controller.kP();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getMrasKi(CobSdoData& dest)
{
	float value = drive->model.m_mras.m_controller.kI();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus setMrasKp(CobSdoData val)
{
	drive->model.m_mras.m_controller.setKp(val.f32);
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus setMrasKi(CobSdoData val)
{
	drive->model.m_mras.m_controller.setKi(val.f32);
	return OD_ACCESS_SUCCESS;
}
#endif

} // namespace od

extern ODEntry OBJECT_DICTIONARY[] = {
{{0x1008, 0x00}, {"SYSTEM", "INFO", "DEVICE_NAME", "", OD_STRING, true, false, OD_NO_DIRECT_ACCESS, od::getDeviceName, OD_NO_WRITE_ACCESS}},
{{0x5FFF, 0x00}, {"SYSTEM", "INFO", "SOFTWARE_VERSION", "", OD_UINT32, true, false, OD_NO_DIRECT_ACCESS, od::getSoftwareVersion, OD_NO_WRITE_ACCESS}},
{{0x5FFF, 0x01}, {"SYSTEM", "INFO", "BUILD_CONFIGURATION", "", OD_STRING, true, false, OD_NO_DIRECT_ACCESS, od::getBuildConfiguration, OD_NO_WRITE_ACCESS}},

{{0x2000, 0x00}, {"SYSTEM", "SYSLOG", "SYSLOG_MSG", "", OD_UINT32, true, false, OD_NO_DIRECT_ACCESS, od::getSyslogMessage, OD_NO_WRITE_ACCESS}},

{{0x5000, 0x00}, {"WATCH", "WATCH", "UPTIME",		"s",	OD_FLOAT32,	true,	false,	OD_NO_DIRECT_ACCESS,	od::getUptime,			OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x01}, {"WATCH", "WATCH", "DRIVE_STATE",	"",	OD_ENUM16,	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveState,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x02}, {"WATCH", "WATCH", "VOLTAGE_DC",	"V",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveVoltageDc,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x03}, {"WATCH", "WATCH", "CURRENT_PHU",	"A",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveCurrentPhU,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x04}, {"WATCH", "WATCH", "CURRENT_PHV",	"A",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveCurrentPhV,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x05}, {"WATCH", "WATCH", "CURRENT_PHW",	"A",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveCurrentPhW,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x06}, {"WATCH", "WATCH", "CURRENT_PHX",	"A",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveCurrentPhX,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x07}, {"WATCH", "WATCH", "CURRENT_PHY",	"A",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveCurrentPhY,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x08}, {"WATCH", "WATCH", "CURRENT_PHZ",	"A",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveCurrentPhZ,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x09}, {"WATCH", "WATCH", "TEMP_PHU",		"°C",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveTempPhU,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x0A}, {"WATCH", "WATCH", "TEMP_PHV",		"°C",	OD_FLOAT32,	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveTempPhV,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x0B}, {"WATCH", "WATCH", "TEMP_PHW",		"°C",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveTempPhW,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x0C}, {"WATCH", "WATCH", "TEMP_PHX",		"°C",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveTempPhX,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x0D}, {"WATCH", "WATCH", "TEMP_PHY",		"°C",	OD_FLOAT32,	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveTempPhY,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x0E}, {"WATCH", "WATCH", "TEMP_PHZ",		"°C",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveTempPhZ,		OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x0F}, {"WATCH", "WATCH", "TEMP_CASE",	"°C",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveTempCase,		OD_NO_WRITE_ACCESS}},
//
//{{0x5000, 0x10}, {"WATCH", "WATCH", "CURRENT_S",	"A",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveCurrentS,	OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x11}, {"WATCH", "WATCH", "CURRENT_D",	"A",	OD_FLOAT32, 	true,	true,	OD_NO_DIRECT_ACCESS,	od::getDriveCurrentD,	od::setDriveCurrentD}},
//{{0x5000, 0x12}, {"WATCH", "WATCH", "CURRENT_Q",	"A",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveCurrentQ,	OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x13}, {"WATCH", "WATCH", "SPEED_RPM",	"rpm",	OD_FLOAT32, 	true,	true,	OD_NO_DIRECT_ACCESS,	od::getDriveSpeedRpm,	od::setDriveSpeed}},
//{{0x5000, 0x14}, {"WATCH", "WATCH", "TORQUE",		"Nm",	OD_FLOAT32, 	true,	true,	OD_NO_DIRECT_ACCESS,	od::getDriveTorque,	od::setDriveTorque}},
//{{0x5000, 0x15}, {"WATCH", "WATCH", "POWER_MECH",	"W",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDrivePowerMech,	OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x16}, {"WATCH", "WATCH", "ROTOR ANGLE", 	"°",	OD_FLOAT32, 	false,	true,	OD_NO_DIRECT_ACCESS,	OD_NO_READ_ACCESS,	od::setDriveOpenLoopAngle}},
//{{0x5000, 0x17}, {"WATCH", "WATCH", "VOLTAGE_D",	"V",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveVoltageD,	OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x18}, {"WATCH", "WATCH", "VOLTAGE_Q",	"V",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveVoltageQ,	OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x19}, {"WATCH", "WATCH", "FLUX",	 	"Wb",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDriveFlux,	OD_NO_WRITE_ACCESS}},
//{{0x5000, 0x1A}, {"WATCH", "WATCH", "POWER_ELEC",	"VA",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getDrivePowerElec,	OD_NO_WRITE_ACCESS}},
//
//
//{{0x2001, 0x00}, {"DRIVE", "CONTROL", "POWERUP", 	"",	OD_TASK,	true,	false,	OD_NO_DIRECT_ACCESS,	od::drivePowerUp,		OD_NO_WRITE_ACCESS}},
//{{0x2001, 0x01}, {"DRIVE", "CONTROL", "POWERDOWN", 	"",	OD_TASK, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::drivePowerDown,		OD_NO_WRITE_ACCESS}},
//
//
//{{0x2002, 0x00}, {"SYSTEM", "CONTROL", "RESET_DEVICE", 		"",	OD_TASK, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::resetDevice,	OD_NO_WRITE_ACCESS}},
//{{0x2002, 0x01}, {"SYSTEM", "CONTROL", "RESET_FAULTS", 		"",	OD_TASK, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::resetAllFaults,	OD_NO_WRITE_ACCESS}},
//{{0x2002, 0x02}, {"SYSTEM", "CONTROL", "RESET_PARAMETERS",	"",	OD_TASK, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::resetParametersToDefault, OD_NO_WRITE_ACCESS}},
//{{0x2002, 0x03}, {"SYSTEM", "CONTROL", "APPLY_PARAMETERS",	"",	OD_TASK, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::applyParameters,	OD_NO_WRITE_ACCESS}},
//
//
//{{0x2100, 0x00}, {"CONFIG",	"MOTOR",	"PARAMS_KNOWN",	"",	OD_BOOL,	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MOTOR_CONFIG.parametersSpecified), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2100, 0x01}, {"CONFIG",	"MOTOR",	"POLE_PAIRS",	"",	OD_INT16, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MOTOR_CONFIG.polePairs), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2100, 0x02}, {"CONFIG",	"MOTOR",	"Rs",	"Ω",		OD_FLOAT32,	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MOTOR_CONFIG.Rs), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2100, 0x03}, {"CONFIG",	"MOTOR",	"Rr",	"Ω",		OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MOTOR_CONFIG.Rr), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2100, 0x04}, {"CONFIG",	"MOTOR",	"Lm",	"H",		OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MOTOR_CONFIG.Lm), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2100, 0x05}, {"CONFIG",	"MOTOR",	"Lls",	"H",		OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MOTOR_CONFIG.Lls), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2100, 0x06}, {"CONFIG",	"MOTOR",	"Llr",	"H",		OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MOTOR_CONFIG.Llr), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//
//{{0x2101, 0x00}, {"CONFIG",	"MODEL",	"CONTROL_LOOP_TYPE", 	"o-c",	OD_ENUM16, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.controlLoopType), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x01}, {"CONFIG",	"MODEL",	"REFERENCE", 		"n-M",	OD_ENUM16, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.reference), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x02}, {"CONFIG",	"MODEL",	"FORWARD_SIGN",		"",	OD_INT16, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.signOfForwardDirection), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x03}, {"CONFIG",	"MODEL",	"KP_SPEED", 		"",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.kP_speed), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x04}, {"CONFIG",	"MODEL",	"KI_SPEED", 		"",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.kI_speed), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x05}, {"CONFIG",	"MODEL",	"KP_ID", 		"",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.kP_iD), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x06}, {"CONFIG",	"MODEL",	"KI_ID", 		"",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.kI_iD), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x07}, {"CONFIG",	"MODEL",	"KP_IQ", 		"",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.kP_iQ), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x08}, {"CONFIG",	"MODEL",	"KI_IQ", 		"",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.kI_iQ), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x09}, {"CONFIG",	"MODEL",	"IS_MOTOR_MAX", 	"A",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.iSMotorMax), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x0A}, {"CONFIG",	"MODEL",	"IS_GENER_MAX", 	"A",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.iSGenerMax), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x0B}, {"CONFIG",	"MODEL",	"TORQUE_POS_MAX", 	"Nm",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.torquePosMax), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x0C}, {"CONFIG",	"MODEL",	"TORQUE_NEG_MAX",	"Nm",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.torqueNegMax), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x0D}, {"CONFIG",	"MODEL",	"SPEED_MAX",		"rpm",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.speedRpmMax), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x0E}, {"CONFIG",	"MODEL",	"KP_FLUXWEAK",		"",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.kP_fluxWeakening), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x0F}, {"CONFIG",	"MODEL",	"KI_FLUXWEAK",		"",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.kI_fluxWeakening), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2101, 0x10}, {"CONFIG",	"MODEL",	"ID_MAX_FLUXWEAK",	"A",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.iDmax_fluxWeakening), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//
//{{0x2102, 0x00}, {"CONFIG",	"CONVERTER",	"UVP_DC",		"V",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.CONVERTER_CONFIG.uvpDC), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2102, 0x01}, {"CONFIG",	"CONVERTER",	"OVP_DC",		"V",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.CONVERTER_CONFIG.ovpDC), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2102, 0x02}, {"CONFIG",	"CONVERTER",	"OCP_PHASE", 		"A",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.CONVERTER_CONFIG.ocpPhase), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2102, 0x05}, {"CONFIG",	"CONVERTER",	"OTP_JUNCTION",		"°C",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.CONVERTER_CONFIG.otpJunction), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2102, 0x06}, {"CONFIG",	"CONVERTER",	"OTP_CASE",		"°C",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.CONVERTER_CONFIG.otpCase), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2102, 0x07}, {"CONFIG",	"CONVERTER",	"FAN_TEMP_TH_ON",	"°C",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.CONVERTER_CONFIG.fanTempThOn), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2102, 0x08}, {"CONFIG",	"CONVERTER",	"FAN_TEMP_TH_OFF",	"°C",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.CONVERTER_CONFIG.fanTempThOff), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//
//{{0x2103, 0x00}, {"CONFIG",	"MRAS",		"KP",			"",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MRAS_CONFIG.kP), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2103, 0x01}, {"CONFIG",	"MRAS",		"KI",			"",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.ACIM_MRAS_CONFIG.kI), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2103, 0x00}, {"CONFIG",	"MRAS",		"KP",			"",	OD_FLOAT32, 	true,	true,	OD_NO_DIRECT_ACCESS, od::getMrasKp, od::setMrasKp}},
//{{0x2103, 0x01}, {"CONFIG",	"MRAS",		"KI",			"",	OD_FLOAT32, 	true,	true,	OD_NO_DIRECT_ACCESS, od::getMrasKi, od::setMrasKi}},
//
//
//

//{{0x2103, 0x00}, {"CONFIG",	"CONTACTOR",	"DCLINK_CHARGE_THRESHOLD",	"V",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.CONTACTOR_CONFIG.dclinkChargeThreshold), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2103, 0x01}, {"CONFIG",	"CONTACTOR",	"DCLINK_CHARGE_TIMEOUT",	"ms",	OD_UINT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.CONTACTOR_CONFIG.dclinkChargeTimeout), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2103, 0x02}, {"CONFIG",	"CONTACTOR",	"DCLINK_CONTACTOR_HOLDUP",	"ms",	OD_UINT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.CONTACTOR_CONFIG.chargingContactorHoldup), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2103, 0x03}, {"CONFIG",	"CONTACTOR",	"DCLINK_DISCHARGE_THRESHOLD",	"V",	OD_FLOAT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.CONTACTOR_CONFIG.dclinkDischargeThreshold), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2103, 0x04}, {"CONFIG",	"CONTACTOR",	"DCLINK_DISCHARGE_TIMEOUT",	"ms",	OD_UINT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.CONTACTOR_CONFIG.dclinkDischargeTimeout), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//
//{{0x2104, 0x00}, {"CONFIG",	"MCOSERVER",	"PERIOD_HB",		"ms",	OD_UINT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.MCOSERVER_CONFIG.periodHeartbeat), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2104, 0x01}, {"CONFIG",	"MCOSERVER",	"PERIOD_TPDO1",		"ms",	OD_UINT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.MCOSERVER_CONFIG.periodTpdo1), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2104, 0x02}, {"CONFIG",	"MCOSERVER",	"PERIOD_TPDO2",		"ms",	OD_UINT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.MCOSERVER_CONFIG.periodTpdo2), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2104, 0x03}, {"CONFIG",	"MCOSERVER",	"PERIOD_TPDO3",		"ms",	OD_UINT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.MCOSERVER_CONFIG.periodTpdo3), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},
//{{0x2104, 0x04}, {"CONFIG",	"MCOSERVER",	"PERIOD_TPDO4",		"ms",	OD_UINT32, 	true,	true,	OD_PTR(&SetupManager::SYSTEM_CONFIG.MCOSERVER_CONFIG.periodTpdo4), OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}},

{{0xFFFF, 0xFF}, {"NULL", "NULL", "END_OF_OD", "", OD_TASK, false, false, OD_NO_DIRECT_ACCESS, OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}}};

extern const size_t OD_SIZE = sizeof(OBJECT_DICTIONARY) / sizeof(OBJECT_DICTIONARY[0]);
extern ODEntry* OBJECT_DICTIONARY_END = OBJECT_DICTIONARY + OD_SIZE;
}


