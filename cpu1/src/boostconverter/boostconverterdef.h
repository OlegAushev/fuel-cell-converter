/**
 * @defgroup boost_converter Boost-Converter
 *
 * @file
 * @ingroup boost_converter
 */


#pragma once


namespace boostconverter {
/// @addtogroup boost_converter
/// @{


// Boost-converter states
enum State
{
	STANDBY,
	IDLE,
	POWERUP,
	READY,
	STARTING,
	IN_OPERATION,
	STOPPING,
	POWERDOWN,
};


/// @}
} // namespace boostconverter


