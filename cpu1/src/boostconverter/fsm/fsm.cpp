/**
 * @file
 * @ingroup boost_converter boost_converter_fsm
 */


#include "fsm.h"
#include "../boostconverter.h"


namespace boostconverter {


/*STANDBY_State STANDBY_State::s_instance;
POWERUP_State POWERUP_State::s_instance;
READY_State READY_State::s_instance;
STARTING_State STARTING_State::s_instance;
IN_OPERATION_State IN_OPERATION_State::s_instance;
STOPPING_State STOPPING_State::s_instance;
POWERDOWN_State POWERDOWN_State::s_instance;*/


///
///
///
void IState::changeState(BoostConverter* converter, IState* state)
{
	converter->changeState(state);
}







} // namespace boostconverter


