/**
 * @file
 * @ingroup mcu mcu_cpu_timers
 */


#include "mcu_cputimers.h"


namespace mcu {


volatile uint64_t SystemClock::m_time;

uint64_t SystemClock::m_taskPeriods[SystemClock::TASK_COUNT];
uint64_t SystemClock::m_taskTimestamps[SystemClock::TASK_COUNT];
ClockTaskStatus (*SystemClock::m_tasks[SystemClock::TASK_COUNT])();

bool SystemClock::m_watchdogEnabled;
uint64_t SystemClock::m_watchdogTimer;
uint64_t SystemClock::m_watchdogPeriod;
bool SystemClock::m_watchdogTimeoutDetected;
ClockTaskStatus (*SystemClock::m_watchdogTask)();

uint64_t SystemClock::m_delayedTaskStart;
uint64_t SystemClock::m_delayedTaskDelay;
void (*SystemClock::m_delayedTask)();


///
///
///
void SystemClock::init()
{
	if (initialized()) return;

	m_time = 0;

	m_watchdogEnabled = false;
	m_watchdogTimer = 0;
	m_watchdogPeriod = 0;
	m_watchdogTimeoutDetected = false;

	m_delayedTaskStart = 0;
	m_delayedTaskDelay = 0;

	Interrupt_register(INT_TIMER0, SystemClock::onInterrupt);

	CPUTimer_stopTimer(CPUTIMER0_BASE);		// Make sure timer is stopped
	CPUTimer_setPeriod(CPUTIMER0_BASE, 0xFFFFFFFF);	// Initialize timer period to maximum
	CPUTimer_setPreScaler(CPUTIMER0_BASE, 0);	// Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
	CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);	// Reload counter register with period value

	uint32_t tmp = (uint32_t)((mcu::sysclkFreq() / 1000) * TIME_STEP);
	CPUTimer_setPeriod(CPUTIMER0_BASE, tmp - 1);
	CPUTimer_setEmulationMode(CPUTIMER0_BASE, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);

	for (size_t i = 0; i < TASK_COUNT; ++i)
	{
		m_taskPeriods[i] = 0x0;
		m_taskTimestamps[i] = 0x0;
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
void SystemClock::runTasks()
{
	for (size_t i = 0; i < TASK_COUNT; ++i)
	{
		if (m_taskPeriods[i] != 0)
		{
			if (now() >= (m_taskTimestamps[i] + m_taskPeriods[i]))
			{
				if (m_tasks[i]() == CLOCK_TASK_SUCCESS)
				{
					m_taskTimestamps[i] = now();
				}
			}
		}
	}

	if (m_delayedTaskDelay != 0)
	{
		if (now() >= (m_delayedTaskStart + m_delayedTaskDelay))
		{
			m_delayedTask();
			m_delayedTaskDelay = 0;
		}
	}
}


///
///
///
__interrupt void SystemClock::onInterrupt()
{
	m_time += TIME_STEP;

	if (m_watchdogEnabled == true)
	{
		m_watchdogTimer += TIME_STEP;
		if (m_watchdogTimer >= m_watchdogPeriod)
		{
			m_watchdogTimeoutDetected = true;
			if (m_watchdogTask() == CLOCK_TASK_SUCCESS)
			{
				resetWatchdog();
			}
		}
	}

	Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}


/*####################################################################################################################*/


uint32_t HighResolutionClock::m_period;


///
///
///
void HighResolutionClock::init(uint32_t period_us)
{
	if (initialized()) return;

	CPUTimer_stopTimer(CPUTIMER1_BASE);             	// Make sure timer is stopped
	CPUTimer_setPeriod(CPUTIMER1_BASE, 0xFFFFFFFF); 	// Initialize timer period to maximum
	CPUTimer_setPreScaler(CPUTIMER1_BASE, 0);       	// Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
	CPUTimer_reloadTimerCounter(CPUTIMER1_BASE);    	// Reload counter register with period value

	m_period = (uint32_t)(mcu::sysclkFreq() / 1000000) * period_us - 1;
	CPUTimer_setPeriod(CPUTIMER1_BASE, m_period);
	CPUTimer_setEmulationMode(CPUTIMER1_BASE, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);

	setInitialized();
}


} // namespace mcu


