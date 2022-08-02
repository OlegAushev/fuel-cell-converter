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

//////////////


inline ODAccessStatus converterRelayOn(CobSdoData val)
{
	converter->turnRelayOn();
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus converterRelayOff(CobSdoData val)
{
	converter->turnRelayOff();
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus resetDevice(CobSdoData val)
{
	Syslog::addMessage(sys::Message::DEVICE_SW_RESET);
	mcu::SystemClock::registerDelayedTask(mcu::resetDevice, 2000);
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus startup(CobSdoData val)
{
	converter->startup();
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus shutdown(CobSdoData val)
{
	converter->shutdown();
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus fuelcellStart(CobSdoData val)
{
	fuelcell::Controller::start();
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus fuelcellStop(CobSdoData val)
{
	fuelcell::Controller::stop();
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus resetAllFaults(CobSdoData val)
{
	mcu::SystemClock::resetWatchdog();
	Syslog::resetFaultsAndWarnings();
	return OD_ACCESS_SUCCESS;
}


} // namespace od


extern ODEntry OBJECT_DICTIONARY[] = {
{{0x1008, 0x00}, {"SYSTEM", "INFO", "DEVICE_NAME", "", OD_STRING, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::getDeviceName, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5FFF, 0x00}, {"SYSTEM", "INFO", "FIRMWARE_VERSION", "", OD_UINT32, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::getSoftwareVersion, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5FFF, 0x01}, {"SYSTEM", "INFO", "BUILD_CONFIGURATION", "", OD_STRING, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::getBuildConfiguration, OD_NO_INDIRECT_WRITE_ACCESS}},


{{0x2000, 0x00}, {"WATCH", "WATCH", "UPTIME",		"s",	OD_FLOAT32,	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::getUptime,			OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x2000, 0x03}, {"WATCH", "WATCH", "VOLTAGE_IN",	"V",	OD_FLOAT32, 	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::getConverterVoltageIn,	OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x2000, 0x04}, {"WATCH", "WATCH", "VOLTAGE_OUT",	"V",	OD_FLOAT32, 	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::getConverterVoltageOut,	OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x2000, 0x05}, {"WATCH", "WATCH", "CURRENT_IN",	"A",	OD_FLOAT32, 	OD_ACCESS_RW,	OD_NO_DIRECT_ACCESS,	od::getConverterCurrentIn,	od::setConverterCurrentIn}},
{{0x2000, 0x0C}, {"WATCH", "WATCH", "PHA_TEMP",		"°C",	OD_FLOAT32, 	OD_ACCESS_RO,	OD_NO_DIRECT_ACCESS,	od::getConverterTempHeatsink,	OD_NO_INDIRECT_WRITE_ACCESS}},


{{0x2001, 0x00}, {"CONVERTER", 	"CONVERTER", "RELAY ON",	"",	OD_TASK, OD_ACCESS_WO,	OD_NO_DIRECT_ACCESS,	OD_NO_INDIRECT_READ_ACCESS,	od::converterRelayOn}},
{{0x2001, 0x01}, {"CONVERTER",	"CONVERTER", "RELAY OFF",	"",	OD_TASK, OD_ACCESS_WO,	OD_NO_DIRECT_ACCESS,	OD_NO_INDIRECT_READ_ACCESS,	od::converterRelayOff}},

{{0x2002, 0x00}, {"SYSTEM CONTROL", 	"SYSTEM CONTROL",	"RESET DEVICE",	"",	OD_TASK, 	OD_ACCESS_WO,	OD_NO_DIRECT_ACCESS,	OD_NO_INDIRECT_READ_ACCESS,	od::resetDevice}},
{{0x2002, 0x01}, {"SYSTEM CONTROL", 	"SYSTEM CONTROL",	"STARTUP", 	"",	OD_TASK, 	OD_ACCESS_WO,	OD_NO_DIRECT_ACCESS,	OD_NO_INDIRECT_READ_ACCESS,	od::startup}},
{{0x2002, 0x02}, {"SYSTEM CONTROL", 	"SYSTEM CONTROL", 	"SHUTDOW", 	"",	OD_TASK, 	OD_ACCESS_WO,	OD_NO_DIRECT_ACCESS,	OD_NO_INDIRECT_READ_ACCESS,	od::shutdown}},
{{0x2002, 0x03}, {"FUELCELL",		"FUELCELL",		"START",	"",	OD_TASK,	OD_ACCESS_WO,	OD_NO_DIRECT_ACCESS,	OD_NO_INDIRECT_READ_ACCESS,	od::fuelcellStart}},
{{0x2002, 0x04}, {"FUELCELL",		"FUELCELL",		"STOP",		"",	OD_TASK,	OD_ACCESS_WO,	OD_NO_DIRECT_ACCESS,	OD_NO_INDIRECT_READ_ACCESS,	od::fuelcellStop}},
{{0x2002, 0x05}, {"SYSTEM CONTROL", 	"SYSTEM CONTROL",	"RESET FAULTS",	"",	OD_TASK, 	OD_ACCESS_WO,	OD_NO_DIRECT_ACCESS,	OD_NO_INDIRECT_READ_ACCESS,	od::resetAllFaults}},
};

extern const size_t OD_SIZE = sizeof(OBJECT_DICTIONARY) / sizeof(OBJECT_DICTIONARY[0]);
extern ODEntry* OBJECT_DICTIONARY_END = OBJECT_DICTIONARY + OD_SIZE;
}


