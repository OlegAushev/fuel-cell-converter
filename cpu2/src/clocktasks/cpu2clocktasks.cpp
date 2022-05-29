/*
 * timer_callbacks.cpp
 *
 *  Created on: 14 июн. 2021 г.
 *      Author: Oleg Aushev
 */

#include "cpu2clocktasks.h"


///
///
///
mcu::ClockTaskStatus taskToggleLed()
{
	const uint64_t periods[4] = {100, 100, 100, 1000};
	static size_t index = 0;

	mcu::Clock::setTaskPeriod(0, periods[index]);
	if ((index % 2) == 0)
	{
		mcu::turnLedOff(mcu::LED_RED);
	}
	else
	{
		mcu::turnLedOn(mcu::LED_RED);
	}
	index = (index + 1) % 4;
	return mcu::CLOCK_TASK_SUCCESS;
}



