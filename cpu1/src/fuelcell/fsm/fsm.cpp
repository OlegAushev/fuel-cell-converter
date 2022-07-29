/**
 * @file
 * @ingroup fuel_cell_fsm
 */


#include "fsm.h"
#include "../converter/fuelcell_converter.h"
#include "../controller/fuelcell_controller.h"
#include "mcu/cputimers/mcu_cputimers.h"


namespace fuelcell {


uint64_t IState::s_timestamp = 0;

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
void IState::changeState(Converter* converter, IState* state)
{
	converter->changeState(state);
	s_timestamp = mcu::SystemClock::now();
}


/* ################################################################################################################## */
/* ############################ */
/* ##### STANDBY state ##### */
/* ############################ */
///
///
///
void STANDBY_State::start(Converter* converter)
{
	if ((!Syslog::faults()) && (!Controller::fault()))
	{
		Controller::start();
		changeState(converter, POWERUP_State::instance());
	}
}


///
///
///
void STANDBY_State::run(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STANDBY_State::stop(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STANDBY_State::emergencyStop(Converter* converter)
{
	// TODO /* DO NOTHING */
}


/* ################################################################################################################## */
/* ######################### */
/* ##### POWERUP state ##### */
/* ######################### */
///
///
///
void POWERUP_State::start(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void POWERUP_State::run(Converter* converter)
{
	if (fuelcell::Controller::inOperation()/* && vin */)
	{
		changeState(converter, READY_State::instance());
	}
	else if (mcu::SystemClock::now() - timestamp() > 15000)
	{
		Controller::stop();
		Syslog::setFault(Fault::FUELCELL_START_FAILED);
		changeState(converter, STANDBY_State::instance());
	}
}


///
///
///
void POWERUP_State::stop(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void POWERUP_State::emergencyStop(Converter* converter)
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
void IDLE_State::start(Converter* converter)
{
	fuelcell::Controller::start();
	changeState(converter, POWERUP_State::instance());
}


///
///
///
void IDLE_State::run(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void IDLE_State::stop(Converter* converter)
{
	fuelcell::Controller::stop();
	changeState(converter, STANDBY_State::instance());
}


///
///
///
void IDLE_State::emergencyStop(Converter* converter)
{
	fuelcell::Controller::stop();
	changeState(converter, STANDBY_State::instance());
}





/* ################################################################################################################## */
/* ####################### */
/* ##### READY state ##### */
/* ####################### */
///
///
///
void READY_State::start(Converter* converter)
{
	converter->start();
	changeState(converter, STARTING_State::instance());
}


///
///
///
void READY_State::run(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void READY_State::stop(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void READY_State::emergencyStop(Converter* converter)
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
void STARTING_State::start(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STARTING_State::run(Converter* converter)
{
	if (Syslog::faults() != 0)
	{
		emergencyStop(converter);
	}
}


///
///
///
void STARTING_State::stop(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STARTING_State::emergencyStop(Converter* converter)
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
void IN_OPERATION_State::start(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void IN_OPERATION_State::run(Converter* converter)
{
	if (Syslog::faults() != 0)
	{
		emergencyStop(converter);
	}
}


///
///
///
void IN_OPERATION_State::stop(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void IN_OPERATION_State::emergencyStop(Converter* converter)
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
void STOPPING_State::start(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STOPPING_State::run(Converter* converter)
{
	if (Syslog::faults() != 0)
	{
		emergencyStop(converter);
	}
}


///
///
///
void STOPPING_State::stop(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STOPPING_State::emergencyStop(Converter* converter)
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
void POWERDOWN_State::start(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void POWERDOWN_State::run(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void POWERDOWN_State::stop(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void POWERDOWN_State::emergencyStop(Converter* converter)
{
	// TODO /* DO NOTHING */
}


} // namespace fuelcell


