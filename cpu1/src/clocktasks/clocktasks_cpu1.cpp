///
#include "clocktasks/clocktasks_cpu1.h"


///
///
///
mcu::ClockTaskStatus taskWatchdogTimeout()
{
	Syslog::setError(sys::Error::CAN_CONNECTION_LOST);
	return mcu::CLOCK_TASK_SUCCESS;
}


///
///
///
mcu::ClockTaskStatus taskToggleLed()
{
	const uint64_t periods[4] = {100, 100, 100, 1700};
	static size_t index = 0;

	mcu::SystemClock::setTaskPeriod(0, periods[index]);
	if ((index % 2) == 0)
	{
		mcu::turnLedOn(mcu::LED_BLUE);
	}
	else
	{
		mcu::turnLedOff(mcu::LED_BLUE);
	}
	index = (index + 1) % 4;
	return mcu::CLOCK_TASK_SUCCESS;
}


///
///
///
mcu::ClockTaskStatus taskStartTempSensors()
{
	fuelcell::Converter::instance()->tempSensor.run();
	return mcu::CLOCK_TASK_SUCCESS;
}


///
///
///
mcu::ClockTaskStatus taskCheckFuelcellErrors()
{
	fuelcell::Controller::checkErrors();
	return mcu::CLOCK_TASK_SUCCESS;
}


