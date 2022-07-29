/**
 * @defgroup fuel_cell_converter  Fuel Cell Converter
 * @defgroup fuel_cell_controller  Fuel Cell Controller
 *
 * @file
 * @ingroup fuel_cell_converter fuel_cell_controller
 */


#pragma once


namespace fuelcell {
/// @addtogroup fuel_cell_converter
/// @{


// Converter states
enum ConverterState
{
	CONVERTER_STANDBY,
	CONVERTER_POWERUP,




	CONVERTER_IDLE,
	CONVERTER_READY,
	CONVERTER_STARTING,
	CONVERTER_IN_OPERATION,
	CONVERTER_STOPPING,
	CONVERTER_POWERDOWN,
};


/// @}


/// @addtogroup fuel_cell_converter
/// @{


/// Fuel cell status
enum FuelcellStatus
{
	FUELCELL_NA,
	FUELCELL_STANDBY,
	FUELCELL_READY,
	FUELCELL_INOP,
	FUELCELL_OVERHEAT,
	FUELCELL_BATT_LOWCHARGE,
	FUELCELL_NOCONNECTION,
	FUELCELL_LOWPRESSURE,
};


/// @}
} // namespace fuelcell


