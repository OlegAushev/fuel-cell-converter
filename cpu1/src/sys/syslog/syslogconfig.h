/**
 * @file
 * @ingroup syslog
 */


#pragma once


namespace sys {
/// @addtogroup syslog
/// @{


namespace Error {


/// System faults
enum Error
{
	OVP_IN,
	UVP_IN,
	OVP_OUT,
	OCP_IN,
	DRIVER_FLT,
	MODULE_OVERTEMP,
	HEATSINK_OVERTEMP,
	CONNECTION_LOST,
	CAN_BUS_ERROR,
	RUNTIME_ERROR,
	EMERGENCY_STOP,
	FUELCELL_STARTUP_FAILED,
	FUELCELL_SHUTDOWN_FAILED,
	FUELCELL_OVERHEAT,
	FUELCELL_BATT_LOWCHARGE,
	FUELCELL_NOCONNECTION,
	FUELCELL_LOWPRESSURE,
};


const uint32_t FATAL_ERRORS = 0;	//(1UL << DRIVER_FLT)
					//| (1UL << RUNTIME_ERROR);


} // namespace Fault


namespace Warning {


/// System warnings
enum Warning
{
	BATTERY_CHARGED,
	CAN_BUS_WARNING,
	CAN_BUS_OVERRUN,
	MODULE_OVERHEATING,
	CASE_OVERHEATING,
};


const uint32_t FATAL_WARNINGS = 0;


} // namespace Warning


namespace Message {


/// System messages
enum Message
{
	// BEGIN of compatibility-messages
	NO_MESSAGE,

	// END of compatibility-messages
	DEVICE_BOOT_CPU1,
	DEVICE_CPU1_BOOT_SUCCESS,
	DEVICE_BOOT_CPU2,
	DEVICE_CPU2_BOOT_SUCCESS,
	DEVICE_CPU1_READY,
	DEVICE_CPU2_READY,
	DEVICE_READY,
	DEVICE_BUSY,
	DEVICE_SW_RESET,
	SDO_REQUEST_LOST,
};


} // namespace Message


/// @}
} // namespace sys


