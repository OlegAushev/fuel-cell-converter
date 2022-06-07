/**
 * @defgroup mcu_cpu_timers CPU Timers
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_cpu_timers
 */


#pragma once


#include "F2837xD_Ipc_drivers.h"
#include "driverlib.h"
#include "device.h"
#include "emb/emb_common.h"
#include "../system/mcusystem.h"


namespace mcu {
/// @addtogroup mcu_cpu_timers
/// @{


/// Clock task statuses
enum ClockTaskStatus
{
	CLOCK_TASK_SUCCESS = 0,
	CLOCK_TASK_FAIL = 1
};


/**
 * @brief Clock class. Based on CPU-Timer0.
 */
class Clock : public emb::Monostate<Clock>
{
private:
	static volatile uint64_t m_time_ms;
	static const uint32_t TIME_STEP_ms = 1;

/* ========================================================================== */
/* = Periodic Tasks = */
/* ========================================================================== */
private:
	static const size_t TASK_COUNT = 4;
	static uint64_t m_taskPeriods[TASK_COUNT];
	static uint64_t m_taskTimestamps[TASK_COUNT];	// timestamp of executed task
	static bool m_taskFlags[TASK_COUNT];
	static ClockTaskStatus (*m_tasks[TASK_COUNT])();

	static ClockTaskStatus emptyTask() { return CLOCK_TASK_SUCCESS; }

	static bool isTaskFlagSet(size_t index) { return m_taskFlags[index]; }
	static void setTaskFlag(size_t index) { m_taskFlags[index] = true; }
	static void resetTaskFlag(size_t index) { m_taskFlags[index] = false; }

public:
	/**
	 * @brief Set task period.
	 * @param index - task index
	 * @param period - task period in milliseconds
	 * @return (none)
	 */
	static void setTaskPeriod(size_t index, uint64_t period) { m_taskPeriods[index] = period; }

	/**
	 * @brief Registers periodic task.
	 * @param index - task period
	 * @param task - pointer to task function
	 * @return (none)
	 */
	static void registerTask(size_t index, ClockTaskStatus (*task)()) { m_tasks[index] = task; }

	/**
	 * @brief Checks and runs periodic tasks.
	 * @param (none)
	 * @return (none)
	 */
	static void runPeriodicTasks()
	{
		for (size_t i = 0; i < TASK_COUNT; ++i)
		{
			if (isTaskFlagSet(i))
			{
				if (m_tasks[i]() == CLOCK_TASK_SUCCESS)
				{
					resetTaskFlag(i);
					m_taskTimestamps[i] = now();
				}
			}
		}
	}

/* ========================================================================== */
/* = Watchdog = */
/* ========================================================================== */
private:
	static bool m_watchdogEnabled;
	static uint64_t m_watchdogTimerMs;
	static uint64_t m_watchdogBoundMs;
	static bool m_watchdogTimeoutDetected;
	static ClockTaskStatus (*m_watchdogTask)();

public:
	/**
	 * @brief Enable watchdog.
	 * @param (none)
	 * @return (none)
	 */
	static void enableWatchdog() { m_watchdogEnabled = true; }

	/**
	 * @brief Disables watchdog.
	 * @param (none)
	 * @return (none)
	 */
	static void disableWatchdog() {m_watchdogEnabled = false; }

	/**
	 * @brief Sets watchdog bound.
	 * @param watchdogBoundMsec - bound in milliseconds
	 * @return (none)
	 */
	static void setWatchdogBound(uint64_t watchdogBoundMsec) { m_watchdogBoundMs = watchdogBoundMsec; }

	/**
	 * @brief Resets watchdog timer.
	 * @param (none)
	 * @return (none)
	 */
	static void resetWatchdogTimer() { m_watchdogTimerMs = 0; }

	/**
	 * @brief Checks if watchdog timeout is detected.
	 * @param (none)
	 * @return \c true if timeout is detected, \c false otherwise.
	 */
	static bool watchdogTimeoutDetected() { return m_watchdogTimeoutDetected; }

	/**
	 * @brief Resets watchdog.
	 * @param (none)
	 * @return (none)
	 */
	static void resetWatchdog()
	{
		m_watchdogTimeoutDetected = false;
		resetWatchdogTimer();
	}

	/**
	 * @brief Registers watchdog timeout task.
	 * @param task - pointer to task function
	 * @return (none)
	 */
	static void registerWatchdogTask(ClockTaskStatus (*task)()) { m_watchdogTask = task; }

/* ========================================================================== */
/* = Delayed Task = */
/* ========================================================================== */
private:
	static uint64_t m_delayedTaskStart;
	static uint64_t m_delayedTaskDelay;
	static void (*m_delayedTask)();
	static void emptyDelayedTask() {}
public:
	/**
	 * @brief Registers delayed task.
	 * @param task - pointer to delayed task function
	 * @return (none)
	 */
	static void registerDelayedTask(void (*task)(), uint64_t delay)
	{
		m_delayedTask = task;
		m_delayedTaskDelay = delay;
		m_delayedTaskStart = now();
	}

private:
	Clock();				// no constructor
	Clock(const Clock& other);		// no copy constructor
	Clock& operator=(const Clock& other);	// no copy assignment operator

	/**
	 * @brief
	 */
	static void tick()
	{
		m_time_ms += TIME_STEP_ms;

		for (size_t i = 0; i < TASK_COUNT; ++i)
		{
			if ((m_taskPeriods[i] != 0)
				&& (now() >= (m_taskTimestamps[i] + m_taskPeriods[i])))
			{
				setTaskFlag(i);
			}
		}

		if ((m_watchdogEnabled == true) && (m_watchdogTimerMs < m_watchdogBoundMs))
		{
			m_watchdogTimerMs += TIME_STEP_ms;
			if (m_watchdogTimerMs >= m_watchdogBoundMs)
			{
				m_watchdogTimeoutDetected = true;
			}
		}

		if (watchdogTimeoutDetected())
		{
			if (m_watchdogTask() == CLOCK_TASK_SUCCESS)
			{
				resetWatchdog();
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

	/**
	 * @brief
	 */
	static __interrupt void onInterrupt()
	{
		tick();
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
	}

public:
	/**
	 * @brief Initializes clock.
	 * @param (none)
	 * @return (none)
	 */
	static void init();

	/**
	 * @brief Returns a time point representing the current point in time.
	 * @param (none)
	 * @return A time point representing the current time in milliseconds.
	 */
	static uint64_t now() { return m_time_ms; }

	/**
	 * @brief Returns clock step.
	 * @param (none)
	 * @return Clock step in milliseconds.
	 */
	static uint32_t step() { return TIME_STEP_ms; }

	/**
	 * @brief Resets clock.
	 * @param (none)
	 * @return (none)
	 */
	static void reset()
	{
		m_time_ms = 0;
		for (size_t i = 0; i < TASK_COUNT; ++i)
		{
			resetTaskFlag(i);
		}
	}
};


/*####################################################################################################################*/
/**
 * @brief Systick timer class. Based on CPU-Timer1.
 */
class SystickTimer
{
private:
	static uint32_t m_period;
	static const uint32_t DEVICE_SYSCLK_PERIOD_NS = 1000000000 / DEVICE_SYSCLK_FREQ;
public:
	/**
	 * @brief Initializes systick timer.
	 * @param (none)
	 * @return (none)
	 */
	static void init(uint32_t period_us)
	{
		CPUTimer_setPeriod(CPUTIMER1_BASE, 0xFFFFFFFF); 	// Initialize timer period to maximum
		CPUTimer_setPreScaler(CPUTIMER1_BASE, 0);       	// Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
		CPUTimer_stopTimer(CPUTIMER1_BASE);             	// Make sure timer is stopped
		CPUTimer_reloadTimerCounter(CPUTIMER1_BASE);    	// Reload counter register with period value

		m_period = (uint32_t)(mcu::sysclkFreq() / 1000000) * period_us - 1;
		CPUTimer_setPeriod(CPUTIMER1_BASE, m_period);
		CPUTimer_setEmulationMode(CPUTIMER1_BASE, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);
	}

	/**
	 * @brief Returns systick timer counter value.
	 * @param (none)
	 * @return Systick timer counter value.
	 */
	static uint32_t counter() { return CPUTimer_getTimerCount(CPUTIMER1_BASE); };

	/**
	 * @brief Returns a time point representing the current point in time.
	 * @param (none)
	 * @return A time point representing the current time in ns.
	 */
	static uint64_t now()
	{
		return static_cast<uint64_t>(m_period - counter()) * DEVICE_SYSCLK_PERIOD_NS;
	}

	/**
	 * @brief Starts systick timer.
	 * @param (none)
	 * @return (none)
	 */
	static void start() { CPUTimer_startTimer(CPUTIMER1_BASE); }

	/**
	 * @brief Stops systick timer.
	 * @param (none)
	 * @return (none)
	 */
	static void stop() { CPUTimer_stopTimer(CPUTIMER1_BASE); }

	/**
	 * @brief Registers systick timer interrupt handler.
	 * @param handler - pointer to handler
	 * @return (none)
	 */
	static void registerInterruptHandler(void (*handler)(void))
	{
		Interrupt_register(INT_TIMER1, handler);
		CPUTimer_enableInterrupt(CPUTIMER1_BASE);
		Interrupt_enable(INT_TIMER1);
	}

private:
	static __interrupt void onInterrupt();
};


/*####################################################################################################################*/
/**
 * @brief Timeout watchdog class.
 */
class TimeoutWatchdog
{
private:
	const uint64_t m_timeout;
	volatile uint64_t m_start;
public:
	/**
	 * @brief
	 * @param timeout - timeout in milliseconds
	 * @return (none)
	 */
	TimeoutWatchdog(uint64_t timeout = 0)
		: m_timeout(timeout)
		, m_start(Clock::now())
	{}

	/**
	 * @brief Check if timeout is expired.
	 * @param (none)
	 * @return \c true if timeout is expired, \c false otherwise.
	 */
	bool triggered() volatile
	{
		if (m_timeout == 0)
		{
			return false;
		}
		if ((Clock::now() - m_start) > m_timeout)
		{
			return true;
		}
		return false;
	}

	/**
	 * @brief Resets timeout watchdog.
	 * @param (none)
	 * @return (none)
	 */
	void reset() volatile
	{
		m_start = Clock::now();
	}
};


/// @}
} // namespace mcu


