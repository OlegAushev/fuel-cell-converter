/**
 * @file
 * @ingroup fuel_cell_fsm
 */


#include "fsm.h"
#include "../converter/fuelcell_converter.h"
#include "../controller/fuelcell_controller.h"
#include "mcu_c28x/cputimers/mcu_cputimers.h"


namespace fuelcell {


uint64_t IState::s_timestamp = 0;


POWERUP_State POWERUP_State::s_instance;
STANDBY_State STANDBY_State::s_instance;
STARTUP_State STARTUP_State::s_instance;
READY_State READY_State::s_instance;
CHARGING_START_State CHARGING_START_State::s_instance;
CHARGING_State CHARGING_State::s_instance;
CHARGING_STOP_State CHARGING_STOP_State::s_instance;
SHUTDOWN_State SHUTDOWN_State::s_instance;
WAIT_State WAIT_State::s_instance;


const uint64_t ERROR_ENABLING_DELAY = 30000;
const uint64_t FUELCELL_STARTUP_MAX_DURATION = 45000;
const uint64_t STARTUP_TO_READY_DELAY = 5000;
const uint64_t DELAY_BEFORE_SHUTDOWN = 2000;
const uint64_t POWERUP_TO_STANDBY_DELAY = 5000;


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


///
///
///
void changeStateAfterWait(Converter* converter, IState* nextState, uint64_t delay)
{
	WAIT_State::instance()->m_waitTime = delay;
	WAIT_State::instance()->m_nextState = nextState;
	WAIT_State::instance()->m_waitStart = mcu::SystemClock::now();
	IState::changeState(converter, WAIT_State::instance());
}


/* ################################################################################################################## */
/* ######################### */
/* ##### POWERUP state ##### */
/* ######################### */
///
///
///
void POWERUP_State::startup(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void POWERUP_State::shutdown(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void POWERUP_State::startCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void POWERUP_State::run(Converter* converter)
{
	if (Controller::isConnectionOk())
	{
		changeStateAfterWait(converter, STANDBY_State::instance(), POWERUP_TO_STANDBY_DELAY);
	}
}


///
///
///
void POWERUP_State::stopCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void POWERUP_State::emergencyShutdown(Converter* converter)
{
	/* DO NOTHING */
}


/* ################################################################################################################## */
/* ######################### */
/* ##### STANDBY state ##### */
/* ######################### */
///
///
///
void STANDBY_State::startup(Converter* converter)
{
	if (!Syslog::errors() && !Syslog::hasWarning(sys::Warning::BATTERY_CHARGED))
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
	float voltDiff = fabsf(converter->voltageIn() - voltPrev);
	voltPrev = converter->voltageIn();

	// TODO Controller::start(); // may be needed here to reset errors at fuel cells (multiple start signal sending)

	if (mcu::SystemClock::now() - timestamp() > ERROR_ENABLING_DELAY)
	{
		Controller::enableErrors();
	}

	if ((fuelcell::Controller::isRunning()) && (voltDiff < 0.1))
	{
		Controller::enableErrors();
		converter->turnRelayOn();
		changeStateAfterWait(converter, READY_State::instance(), STARTUP_TO_READY_DELAY);
	}
	else if (mcu::SystemClock::now() - timestamp() > FUELCELL_STARTUP_MAX_DURATION)
	{
		Syslog::setError(sys::Error::FUELCELL_STARTUP_FAILED);
	}

	if (Syslog::errors() != 0)
	{
		shutdown(converter);
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
	changeStateAfterWait(converter, SHUTDOWN_State::instance(), DELAY_BEFORE_SHUTDOWN);
}


///
///
///
void READY_State::startCharging(Converter* converter)
{
	if ((Syslog::errors() == 0) && (!Syslog::hasWarning(sys::Warning::BATTERY_CHARGED)))
	{
		converter->start();
		changeState(converter, CHARGING_START_State::instance());
	}
}


///
///
///
void READY_State::run(Converter* converter)
{
	if (Syslog::errors() != 0)
	{
		shutdown(converter);
	}
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
void CHARGING_START_State::startup(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void CHARGING_START_State::shutdown(Converter* converter)
{
	converter->stop();
	Controller::stop();
	resetState();
	changeStateAfterWait(converter, SHUTDOWN_State::instance(), DELAY_BEFORE_SHUTDOWN);
}


///
///
///
void CHARGING_START_State::startCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void CHARGING_START_State::run(Converter* converter)
{
	if (Syslog::errors() != 0)
	{
		shutdown(converter);
	}

	float currRefDiff = (converter->config().currentInMax - converter->config().currentInMin) /
			(60 * converter->pwm.freq());
	m_currentInRef = emb::clamp(m_currentInRef + currRefDiff,
			converter->config().currentInMin, converter->config().currentInMax);

	converter->setCurrentIn(m_currentInRef);

	if (m_currentInRef >= converter->config().currentInMax)
	{
		resetState();
		changeState(converter, CHARGING_State::instance());
	}

}


///
///
///
void CHARGING_START_State::stopCharging(Converter* converter)
{
	converter->stop();
	resetState();
	changeState(converter, READY_State::instance());
}


///
///
///
void CHARGING_START_State::emergencyShutdown(Converter* converter)
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
void CHARGING_State::startup(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void CHARGING_State::shutdown(Converter* converter)
{
	converter->stop();
	Controller::stop();
	changeStateAfterWait(converter, SHUTDOWN_State::instance(), DELAY_BEFORE_SHUTDOWN);
}


///
///
///
void CHARGING_State::startCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void CHARGING_State::run(Converter* converter)
{
	if (Syslog::errors() != 0)
	{
		shutdown(converter);
	}
}


///
///
///
void CHARGING_State::stopCharging(Converter* converter)
{
	converter->stop();
	changeState(converter, READY_State::instance());
}


///
///
///
void CHARGING_State::emergencyShutdown(Converter* converter)
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
void CHARGING_STOP_State::startup(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void CHARGING_STOP_State::shutdown(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void CHARGING_STOP_State::startCharging(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void CHARGING_STOP_State::run(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void CHARGING_STOP_State::stopCharging(Converter* converter)
{
	// TODO /* DO NOTHING */
}


///
///
///
void CHARGING_STOP_State::emergencyShutdown(Converter* converter)
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
	converter->turnRelayOff();
	changeState(converter, STANDBY_State::instance());
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
void WAIT_State::startup(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void WAIT_State::shutdown(Converter* converter)
{
	converter->stop();
	/* DO NOTHING */
}


///
///
///
void WAIT_State::startCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void WAIT_State::run(Converter* converter)
{
	if (mcu::SystemClock::now() - m_waitStart > m_waitTime)
	{
		if (m_nextState != static_cast<IState*>(NULL))
		{
			changeState(converter, m_nextState);
		}
		else
		{
			converter->stop();
			Controller::stop();
			changeState(converter, SHUTDOWN_State::instance());
		}
	}

	if (Syslog::errors() != 0)
	{
		converter->stop();
	}
}


///
///
///
void WAIT_State::stopCharging(Converter* converter)
{
	/* DO NOTHING */
}


///
///
///
void WAIT_State::emergencyShutdown(Converter* converter)
{
	shutdown(converter);
}


} // namespace fuelcell


