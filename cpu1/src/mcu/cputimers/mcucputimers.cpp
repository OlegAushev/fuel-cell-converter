/**
 * @file
 * @ingroup mcu mcu_cpu_timers
 */


#include "mcucputimers.h"


namespace mcu {

volatile uint64_t Clock::m_timeMs;

uint64_t Clock::m_taskPeriods[Clock::TASK_COUNT];
bool Clock::m_taskFlags[Clock::TASK_COUNT];
ClockTaskStatus (*Clock::m_tasks[Clock::TASK_COUNT])();

bool Clock::m_watchdogEnabled;
uint64_t Clock::m_watchdogTimerMs;
uint64_t Clock::m_watchdogBoundMs;
bool Clock::m_watchdogTimeoutDetected;
ClockTaskStatus (*Clock::m_watchdogTask)();

uint64_t Clock::m_delayedTaskStart;
uint64_t Clock::m_delayedTaskDelay;
void (*Clock::m_delayedTask)();


///
///
///
void Clock::init()
{
	if (initialized())
	{
		return;
	}

	m_timeMs = 0;

	m_watchdogEnabled = false;
	m_watchdogTimerMs = 0;
	m_watchdogBoundMs = 0;
	m_watchdogTimeoutDetected = false;

	m_delayedTaskStart = 0;
	m_delayedTaskDelay = 0;

	Interrupt_register(INT_TIMER0, Clock::onInterrupt);

	CPUTimer_setPeriod(CPUTIMER0_BASE, 0xFFFFFFFF);	// Initialize timer period to maximum
	CPUTimer_setPreScaler(CPUTIMER0_BASE, 0);	// Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
	CPUTimer_stopTimer(CPUTIMER0_BASE);		// Make sure timer is stopped
	CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);	// Reload counter register with period value

	uint32_t tmp = (uint32_t)(DEVICE_SYSCLK_FREQ / 1000 * TIME_STEP_MS);
	CPUTimer_setPeriod(CPUTIMER0_BASE, tmp - 1);
	CPUTimer_setEmulationMode(CPUTIMER0_BASE, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);

	for (size_t i = 0; i < TASK_COUNT; ++i)
	{
		m_taskPeriods[i] = 0x0;
	}
	for (size_t i = 0; i < TASK_COUNT; ++i)
	{
		m_tasks[i] = emptyTask;
	}

	m_watchdogTask = emptyTask;
	m_delayedTask = emptyDelayedTask;

	CPUTimer_enableInterrupt(CPUTIMER0_BASE);
	Interrupt_enable(INT_TIMER0);
	CPUTimer_startTimer(CPUTIMER0_BASE);

	setInitialized();
}

///
///
///
uint32_t SystickTimer::m_period;




} /* namespace mcu */

