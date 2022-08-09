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
	CONVERTER_STARTUP,
	CONVERTER_READY,
	CONVERTER_CHARGING_START,
	CONVERTER_CHARGING,
	CONVERTER_CHARGING_STOP,
	CONVERTER_SHUTDOWN,
	CONVERTER_WAIT,
};


/// @}


/// @addtogroup fuel_cell_converter
/// @{





/// @}
} // namespace fuelcell


