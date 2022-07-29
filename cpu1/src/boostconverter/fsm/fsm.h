/**
 * @defgroup boost_converter_fsm FSM
 * @ingroup boost_converter
 *
 * @file
 * @ingroup boost_converter boost_converter_fsm
 */


#pragma once


#include "../boostconverterdef.h"


/// Forward declaration.
class BoostConverter;


namespace boostconverter {
/// @addtogroup boost_converter
/// @{


/**
 * @brief
 */
class IState
{
private:
	const State STATE_ID;
protected:
	IState(State stateId) : STATE_ID(stateId) {}
	void changeState(BoostConverter* converter, IState* state);
};





























/// @}
} // namespace boostconverter


