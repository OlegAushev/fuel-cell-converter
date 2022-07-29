/**
 * @file
 * @ingroup microcanopen mco_od
 */


#include "objectdictionary.h"


namespace microcanopen {

namespace od {


// APP-SPECIFIC objects
fuelcell::Converter* converter = static_cast<fuelcell::Converter*>(NULL);


/* ========================================================================== */
/* =================== INFO ====================== */
/* ========================================================================== */
inline ODAccessStatus getDeviceName(CobSdoData& dest)
{
	uint16_t name[4] = {0};
	memcpy(name, Syslog::DEVICE_NAME, sizeof(name)/sizeof(*name));
	uint32_t nameRaw = 0;
	emb::c28x::from_bytes8<uint32_t>(nameRaw, name);
	memcpy(&dest, &nameRaw, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getSoftwareVersion(CobSdoData& dest)
{
	memcpy(&dest, &Syslog::FIRMWARE_VERSION, sizeof(uint32_t));
	return OD_ACCESS_SUCCESS;
}

inline ODAccessStatus getBuildConfiguration(CobSdoData& dest)
{
	uint16_t name[4];
	memcpy(name, Syslog::BUILD_CONFIGURATION, sizeof(name)/sizeof(*name));
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


} // namespace od

extern ODEntry OBJECT_DICTIONARY[] = {
{{0x1008, 0x00}, {"SYSTEM", "INFO", "DEVICE_NAME", "", OD_STRING, true, false, OD_NO_DIRECT_ACCESS, od::getDeviceName, OD_NO_WRITE_ACCESS}},
{{0x5FFF, 0x00}, {"SYSTEM", "INFO", "FIRMWARE_VERSION", "", OD_UINT32, true, false, OD_NO_DIRECT_ACCESS, od::getSoftwareVersion, OD_NO_WRITE_ACCESS}},
{{0x5FFF, 0x01}, {"SYSTEM", "INFO", "BUILD_CONFIGURATION", "", OD_STRING, true, false, OD_NO_DIRECT_ACCESS, od::getBuildConfiguration, OD_NO_WRITE_ACCESS}},


{{0x2000, 0x00}, {"WATCH", "WATCH", "UPTIME",		"s",	OD_FLOAT32,	true,	false,	OD_NO_DIRECT_ACCESS,	od::getUptime,			OD_NO_WRITE_ACCESS}},
{{0x2000, 0x03}, {"WATCH", "WATCH", "VOLTAGE_IN",	"V",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getConverterVoltageIn,	OD_NO_WRITE_ACCESS}},
{{0x2000, 0x04}, {"WATCH", "WATCH", "VOLTAGE_OUT",	"V",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getConverterVoltageOut,	OD_NO_WRITE_ACCESS}},
{{0x2000, 0x05}, {"WATCH", "WATCH", "CURRENT_IN",	"A",	OD_FLOAT32, 	true,	true,	OD_NO_DIRECT_ACCESS,	od::getConverterCurrentIn,	od::setConverterCurrentIn}},
{{0x2000, 0x0C}, {"WATCH", "WATCH", "PHA_TEMP",		"°C",	OD_FLOAT32, 	true,	false,	OD_NO_DIRECT_ACCESS,	od::getConverterTempHeatsink,	OD_NO_WRITE_ACCESS}},


{{0x2001, 0x00}, {"CONVERTER", 	"CONVERTER", "RELAY ON",	"",	OD_TASK, false, true,	OD_NO_DIRECT_ACCESS,	OD_NO_READ_ACCESS,	od::converterRelayOn}},
{{0x2001, 0x01}, {"CONVERTER",	"CONVERTER", "RELAY OFF",	"",	OD_TASK, false, true,	OD_NO_DIRECT_ACCESS,	OD_NO_READ_ACCESS,	od::converterRelayOff}},


{{0x2002, 0x03}, {"FUELCELL", 	"FUELCELL", "START",		"",	OD_TASK, false, true,	OD_NO_DIRECT_ACCESS,	OD_NO_READ_ACCESS,	od::fuelcellStart}},
{{0x2002, 0x04}, {"FUELCELL", 	"FUELCELL", "STOP",		"",	OD_TASK, false, true,	OD_NO_DIRECT_ACCESS,	OD_NO_READ_ACCESS,	od::fuelcellStop}},


{{0xFFFF, 0xFF}, {"NULL", "NULL", "END_OF_OD", "", OD_TASK, false, false, OD_NO_DIRECT_ACCESS, OD_NO_READ_ACCESS, OD_NO_WRITE_ACCESS}}};

extern const size_t OD_SIZE = sizeof(OBJECT_DICTIONARY) / sizeof(OBJECT_DICTIONARY[0]);
extern ODEntry* OBJECT_DICTIONARY_END = OBJECT_DICTIONARY + OD_SIZE;
}


