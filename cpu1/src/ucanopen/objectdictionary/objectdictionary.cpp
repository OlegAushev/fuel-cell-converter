/**
 * @file
 * @ingroup ucanopen ucanopen_od
 */


#include "objectdictionary.h"


namespace ucanopen {

namespace od {


// APP-SPECIFIC objects
fuelcell::Converter* converter = static_cast<fuelcell::Converter*>(NULL);


/* ========================================================================== */
/* =================== INFO ====================== */
/* ========================================================================== */
inline ODAccessStatus getDeviceName(CobSdoData& dest)
{
	uint16_t name[4] = {0};
	memcpy(name, sys::DEVICE_NAME_SHORT, sizeof(name)/sizeof(*name));
	uint32_t nameRaw = 0;
	emb::c28x::from_bytes8<uint32_t>(nameRaw, name);
	memcpy(&dest, &nameRaw, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getSoftwareVersion(CobSdoData& dest)
{
	memcpy(&dest, &sys::FIRMWARE_VERSION_NUM, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getBuildConfiguration(CobSdoData& dest)
{
	uint16_t name[4];
	memcpy(name, sys::BUILD_CONFIGURATION_SHORT, sizeof(name)/sizeof(*name));
	uint32_t nameRaw;
	emb::c28x::from_bytes8<uint32_t>(nameRaw, name);
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

inline ODAccessStatus getConverterVoltageIn(CobSdoData& dest)
{
	float value = converter->voltageIn();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getConverterVoltageOut(CobSdoData& dest)
{
	float value = converter->voltageOut();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getConverterCurrentIn(CobSdoData& dest)
{
	float value = converter->currentIn();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus setConverterCurrentIn(CobSdoData val)
{
	converter->setCurrentIn(val.f32);
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getConverterTempHeatsink(CobSdoData& dest)
{
	float value = converter->tempHeatsink();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getFuelcellState(CobSdoData& dest)
{
	float value = fuelcell::Controller::state();
	memcpy(&dest, &value, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

/*============================================================================*/


inline ODAccessStatus converterRelayOn(CobSdoData& dest)
{
	converter->turnRelayOn();
	dest.u32 = TASK_SUCCESS;
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus converterRelayOff(CobSdoData& dest)
{
	converter->turnRelayOff();
	dest.u32 = TASK_SUCCESS;
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus resetDevice(CobSdoData& dest)
{
	Syslog::addMessage(sys::Message::DEVICE_SW_RESET);
	mcu::SystemClock::registerDelayedTask(mcu::resetDevice, 2000);
	dest.u32 = TASK_SUCCESS;
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus resetAllFaults(CobSdoData& dest)
{
	mcu::SystemClock::resetWatchdog();
	Syslog::resetErrorsWarnings();
	dest.u32 = TASK_SUCCESS;
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus startup(CobSdoData& dest)
{
	converter->startup();
	dest.u32 = TASK_SUCCESS;
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus shutdown(CobSdoData& dest)
{
	converter->shutdown();
	dest.u32 = TASK_SUCCESS;
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus fuelcellStart(CobSdoData& dest)
{
	fuelcell::Controller::start();
	dest.u32 = TASK_SUCCESS;
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus fuelcellStop(CobSdoData& dest)
{
	fuelcell::Controller::stop();
	dest.u32 = TASK_SUCCESS;
	return OD_ACCESS_SUCCESS;
}


} // namespace od


extern ODEntry OBJECT_DICTIONARY[] = {
{{0x1008, 0x00}, {"SYSTEM", "INFO", "DEVICE_NAME", "", OD_STRING, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::getDeviceName, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5FFF, 0x00}, {"SYSTEM", "INFO", "FIRMWARE_VERSION", "", OD_UINT32, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::getSoftwareVersion, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5FFF, 0x01}, {"SYSTEM", "INFO", "BUILD_CONFIGURATION", "", OD_STRING, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::getBuildConfiguration, OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x2000, 0x00}, {"SYSTEM", "SYSLOG", "SYSLOG_MSG", "", OD_UINT32, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::getSyslogMessage, OD_NO_INDIRECT_WRITE_ACCESS}},


{{0x5000, 0x00}, {"WATCH", "WATCH", "UPTIME",		"s",	OD_FLOAT32,	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::getUptime,			OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5000, 0x01}, {"WATCH", "WATCH", "VOLTAGE_IN",	"V",	OD_FLOAT32, 	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::getConverterVoltageIn,	OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5000, 0x02}, {"WATCH", "WATCH", "VOLTAGE_OUT",	"V",	OD_FLOAT32, 	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::getConverterVoltageOut,	OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5000, 0x03}, {"WATCH", "WATCH", "CURRENT_IN",	"A",	OD_FLOAT32, 	OD_ACCESS_RW,	OD_NO_DIRECT_ACCESS,	od::getConverterCurrentIn,	od::setConverterCurrentIn}},
{{0x5000, 0x04}, {"WATCH", "WATCH", "TEMP_HEATSINK",	"°C",	OD_FLOAT32, 	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::getConverterTempHeatsink,	OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5000, 0x05}, {"WATCH", "WATCH", "FC_STATE",		"",	OD_FLOAT32,	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::getFuelcellState,		OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x2001, 0x00}, {"CONVERTER", 	"CONVERTER",	"RELAY ON",	"",	OD_TASK,	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::converterRelayOn,	OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x2001, 0x01}, {"CONVERTER",	"CONVERTER",	"RELAY OFF",	"",	OD_TASK,	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::converterRelayOff,	OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x2002, 0x00}, {"SYSCTL", 	"SYSCTL",	"RESET DEVICE",	"",	OD_TASK,	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::resetDevice,	OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x2002, 0x01}, {"SYSCTL", 	"SYSCTL",	"RESET FAULTS",	"",	OD_TASK, 	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::resetAllFaults,	OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x2002, 0x02}, {"SYSCTL", 	"SYSCTL", 	"SHUTDOWN", 	"",	OD_TASK, 	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::shutdown,		OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x2002, 0x03}, {"SYSCTL",	"SYSCTL",	"STARTUP",	"",	OD_TASK,	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::startup,		OD_NO_INDIRECT_WRITE_ACCESS}},
};

extern const size_t OD_SIZE = sizeof(OBJECT_DICTIONARY) / sizeof(OBJECT_DICTIONARY[0]);
extern ODEntry* OBJECT_DICTIONARY_END = OBJECT_DICTIONARY + OD_SIZE;
}


