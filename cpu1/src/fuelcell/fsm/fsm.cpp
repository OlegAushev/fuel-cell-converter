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
STARTUP_State STARTUP_State::s_instance;
READY_State READY_State::s_instance;
STARTCHARGING_State STARTCHARGING_State::s_instance;
INOPERATION_State INOPERATION_State::s_instance;
STOPCHARGING_State STOPCHARGING_State::s_instance;
SHUTDOWN_State SHUTDOWN_State::s_instance;


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
void STANDBY_State::startup(Converter* converter)
{
	if ((!Syslog::faults()) && (!Controller::fault()))
	{
		Controller::start();
		changeState(converter, STARTUP_State::instance());
	}
}


///
///
///
void STANDBY_State::shutdown(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void STANDBY_State::startCharging(Converter* converter)
{
	startup(converter);
}


///
///
///
void STANDBY_State::run(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void STANDBY_State::stopCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void STANDBY_State::emergencyShutdown(Converter* converter)
{
	/* DO NOTHING */
}


/* ################################################################################################################## */
/* ######################### */
/* ##### STARTUP state ##### */
/* ######################### */
///
///
///
void STARTUP_State::startup(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void STARTUP_State::shutdown(Converter* converter)
{
	Controller::stop();
	changeState(converter, STANDBY_State::instance());
}


///
///
///
void STARTUP_State::startCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void STARTUP_State::run(Converter* converter)
{
	static float voltPrev = 0;
	float voltDiff = fabsf(converter->voltageOut() - voltPrev);

	if ((fuelcell::Controller::inOperation()) && (voltDiff < 0.05))
	{
		changeState(converter, READY_State::instance());
	}
	else if (mcu::SystemClock::now() - timestamp() > 15000)
	{
		Controller::stop();
		Syslog::setFault(sys::Fault::FUELCELL_STARTUP_FAILED);
		changeState(converter, STANDBY_State::instance());
	}
}


///
///
///
void STARTUP_State::stopCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void STARTUP_State::emergencyShutdown(Converter* converter)
{
	shutdown(converter);
}


/* ################################################################################################################## */
/* ####################### */
/* ##### READY state ##### */
/* ####################### */
///
///
///
void READY_State::startup(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void READY_State::shutdown(Converter* converter)
{
	Controller::stop();
	changeState(converter, SHUTDOWN_State::instance());
}


///
///
///
void READY_State::startCharging(Converter* converter)
{
	if ((Syslog::faults() == 0) && (!Syslog::hasWarning(sys::Warning::BATTERY_CHARGED)))
	{
		converter->start();
		changeState(converter, STARTCHARGING_State::instance());
	}
}


///
///
///
void READY_State::run(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void READY_State::stopCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void READY_State::emergencyShutdown(Converter* converter)
{
	shutdown(converter);
}

/* ################################################################################################################## */
/* ############################### */
/* ##### STARTCHARGING state ##### */
/* ############################### */
///
///
///
void STARTCHARGING_State::startup(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void STARTCHARGING_State::shutdown(Converter* converter)
{
	converter->stop();
	Controller::stop();
	resetState();
	changeState(converter, SHUTDOWN_State::instance());
}


///
///
///
void STARTCHARGING_State::startCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void STARTCHARGING_State::run(Converter* converter)
{
	if (Syslog::faults() != 0)
	{
		stopCharging(converter);
	}

	float currRefDiff = (converter->congig().currentInMax - converter->congig().currentInMin) /
			(30 * converter->pwm.freq());
	m_currentInRef = emb::clamp(m_currentInRef + currRefDiff,
			converter->congig().currentInMin, converter->congig().currentInMax);

	if (m_currentInRef >= converter->congig().currentInMax)
	{
		resetState();
		changeState(converter, INOPERATION_State::instance());
	}

}


///
///
///
void STARTCHARGING_State::stopCharging(Converter* converter)
{
	converter->stop();
	resetState();
	changeState(converter, READY_State::instance());
}


///
///
///
void STARTCHARGING_State::emergencyShutdown(Converter* converter)
{
	shutdown(converter);
}


/* ################################################################################################################## */
/* ############################# */
/* ##### INOPERATION state ##### */
/* ############################# */
///
///
///
void INOPERATION_State::startup(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void INOPERATION_State::shutdown(Converter* converter)
{
	converter->stop();
	Controller::stop();
	changeState(converter, SHUTDOWN_State::instance());
}


///
///
///
void INOPERATION_State::startCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void INOPERATION_State::run(Converter* converter)
{
	if (Syslog::faults() != 0)
	{
		stopCharging(converter);
	}
}


///
///
///
void INOPERATION_State::stopCharging(Converter* converter)
{
	converter->stop();
	changeState(converter, READY_State::instance());
}


///
///
///
void INOPERATION_State::emergencyShutdown(Converter* converter)
{
	shutdown(converter);
}


/* ################################################################################################################## */
/* ############################## */
/* ##### STOPCHARGING state ##### */
/* ############################## */
///
///
///
void STOPCHARGING_State::startup(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STOPCHARGING_State::shutdown(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STOPCHARGING_State::startCharging(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STOPCHARGING_State::run(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STOPCHARGING_State::stopCharging(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void STOPCHARGING_State::emergencyShutdown(Converter* converter)
{
	// TODO /* DO NOTHING */
}


/* ################################################################################################################## */
/* ########################### */
/* ##### SHUTDOWN state ##### */
/* ########################### */
///
///
///
void SHUTDOWN_State::startup(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void SHUTDOWN_State::shutdown(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void SHUTDOWN_State::startCharging(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void SHUTDOWN_State::run(Converter* converter)
{
	if (Controller::standby())
	{
		changeState(converter, STANDBY_State::instance());
	}
	else if (mcu::SystemClock::now() - timestamp() > 30000)
	{
		Syslog::setFault(sys::Fault::FUELCELL_SHUTDOWN_FAILED);
		changeState(converter, STANDBY_State::instance());
	}
}


///
///
///
void SHUTDOWN_State::stopCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void SHUTDOWN_State::emergencyShutdown(Converter* converter)
{
	/* DO NOTHING */
}


/* ################################################################################################################## */
/* ###################### */
/* ##### IDLE state ##### */
/* ###################### */
///
///
///
void IDLE_State::startup(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void IDLE_State::shutdown(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void IDLE_State::startCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void IDLE_State::run(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void IDLE_State::stopCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void IDLE_State::emergencyShutdown(Converter* converter)
{
	/* DO NOTHING */
}


} // namespace fuelcell


