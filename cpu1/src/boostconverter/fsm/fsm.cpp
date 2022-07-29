/**
 * @file
 * @ingroup boost_converter boost_converter_fsm
 */


#include "fsm.h"
#include "../boostconverter.h"
#include "fuelcellcontroller/fuelcellcontroller.h"


namespace boostconverter {


STANDBY_State STANDBY_State::s_instance;
IDLE_State IDLE_State::s_instance;
POWERUP_State POWERUP_State::s_instance;
READY_State READY_State::s_instance;
STARTING_State STARTING_State::s_instance;
IN_OPERATION_State IN_OPERATION_State::s_instance;
STOPPING_State STOPPING_State::s_instance;
POWERDOWN_State POWERDOWN_State::s_instance;


/* ################################################################################################################## */
/* ##################### */
/* ##### INTERFACE ##### */
/* ##################### */
///
///
///
void IState::changeState(BoostConverter* converter, IState* state)
{
	converter->changeState(state);
}


/* ################################################################################################################## */
/* ############################ */
/* ##### STANDBY state ##### */
/* ############################ */
///
///
///
void STANDBY_State::start(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STANDBY_State::run(BoostConverter* converter)
{
	static float voltPrev = 0;
	float voltDiff = fabsf(converter->voltageIn() - voltPrev);

	if ((converter->voltageIn() > 110) && (voltDiff < 0.25))
	{
		converter->turnRelayOn();
		changeState(converter, IDLE_State::instance());
	}
	else if (converter->voltageIn() < 90)
	{
		converter->turnRelayOff();
	}
}


///
///
///
void STANDBY_State::stop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STANDBY_State::emergencyStop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


/* ################################################################################################################## */
/* ###################### */
/* ##### IDLE state ##### */
/* ###################### */
///
///
///
void IDLE_State::start(BoostConverter* converter)
{
	fuelcell::Controller::start();
	changeState(converter, POWERUP_State::instance());
}


///
///
///
void IDLE_State::run(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void IDLE_State::stop(BoostConverter* converter)
{
	fuelcell::Controller::stop();
	changeState(converter, STANDBY_State::instance());
}


///
///
///
void IDLE_State::emergencyStop(BoostConverter* converter)
{
	fuelcell::Controller::stop();
	changeState(converter, STANDBY_State::instance());
}


/* ################################################################################################################## */
/* ######################### */
/* ##### POWERUP state ##### */
/* ######################### */
///
///
///
void POWERUP_State::start(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void POWERUP_State::run(BoostConverter* converter)
{
	if (fuelcell::Controller::inOperation())
	{
		changeState(converter, READY_State::instance());
	}
}


///
///
///
void POWERUP_State::stop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void POWERUP_State::emergencyStop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


/* ################################################################################################################## */
/* ####################### */
/* ##### READY state ##### */
/* ####################### */
///
///
///
void READY_State::start(BoostConverter* converter)
{
	converter->start();
	changeState(converter, STARTING_State::instance());
}


///
///
///
void READY_State::run(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void READY_State::stop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void READY_State::emergencyStop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


/* ################################################################################################################## */
/* ########################## */
/* ##### STARTING state ##### */
/* ########################## */
///
///
///
void STARTING_State::start(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STARTING_State::run(BoostConverter* converter)
{
	if (Syslog::faults() != 0)
	{
		emergencyStop(converter);
	}
}


///
///
///
void STARTING_State::stop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STARTING_State::emergencyStop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


/* ################################################################################################################## */
/* ############################## */
/* ##### IN_OPERATION state ##### */
/* ############################## */
///
///
///
void IN_OPERATION_State::start(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void IN_OPERATION_State::run(BoostConverter* converter)
{
	if (Syslog::faults() != 0)
	{
		emergencyStop(converter);
	}
}


///
///
///
void IN_OPERATION_State::stop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void IN_OPERATION_State::emergencyStop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


/* ################################################################################################################## */
/* ########################## */
/* ##### STOPPING state ##### */
/* ########################## */
///
///
///
void STOPPING_State::start(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STOPPING_State::run(BoostConverter* converter)
{
	if (Syslog::faults() != 0)
	{
		emergencyStop(converter);
	}
}


///
///
///
void STOPPING_State::stop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STOPPING_State::emergencyStop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


/* ################################################################################################################## */
/* ########################### */
/* ##### POWERDOWN state ##### */
/* ########################### */
///
///
///
void POWERDOWN_State::start(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void POWERDOWN_State::run(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void POWERDOWN_State::stop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void POWERDOWN_State::emergencyStop(BoostConverter* converter)
{
	// TODO /* DO NOTHING */
}


} // namespace boostconverter


