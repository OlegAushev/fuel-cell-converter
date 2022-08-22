/**
 * @defgroup mcu_cpu_timers CPU Timers
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_cpu_timers
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "emb/emb_common.h"
#include "../system/mcu_system.h"


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
 * @brief System clock class. Based on CPU-Timer0.
 */
class SystemClock : public emb::Monostate<SystemClock>
{
private:
	static volatile uint64_t m_time;
	static const uint32_t TIME_STEP = 1;
	static const size_t TASK_COUNT = 4;

/* ========================================================================== */
/* = Periodic Tasks = */
/* ========================================================================== */
private:
	static uint64_t m_taskPeriods[TASK_COUNT];
	static uint64_t m_taskTimestamps[TASK_COUNT];	// timestamp of executed task
	static ClockTaskStatus (*m_tasks[TASK_COUNT])();
	static ClockTaskStatus emptyTask() { return CLOCK_TASK_SUCCESS; }
public:
	/**
	 * @brief Set task period.
	 * @param index - task index
	 * @param period - task period in milliseconds
	 * @return (none)
	 */
	static void setTaskPeriod(size_t index, uint64_t period)
	{
		m_taskPeriods[index] = period;
	}

	/**
	 * @brief Registers periodic task.
	 * @param index - task period
	 * @param task - pointer to task function
	 * @return (none)
	 */
	static void registerTask(size_t index, ClockTaskStatus (*task)())
	{
		m_tasks[index] = task;
	}

/* ========================================================================== */
/* = Watchdog = */
/* ========================================================================== */
private:
	static bool m_watchdogEnabled;
	static uint64_t m_watchdogTimer;
	static uint64_t m_watchdogPeriod;
	static bool m_watchdogTimeoutDetected;
	static ClockTaskStatus (*m_watchdogTask)();
public:
	/**
	 * @brief Enable watchdog.
	 * @param (none)
	 * @return (none)
	 */
	static void enableWatchdog()
	{
		m_watchdogEnabled = true;
	}

	/**
	 * @brief Disables watchdog.
	 * @param (none)
	 * @return (none)
	 */
	static void disableWatchdog()
	{
		m_watchdogEnabled = false;
	}

	/**
	 * @brief Sets watchdog bound.
	 * @param watchdogBoundMsec - bound in milliseconds
	 * @return (none)
	 */
	static void setWatchdogPeriod(uint64_t watchdogBound_ms)
	{
		m_watchdogPeriod = watchdogBound_ms;
	}

	/**
	 * @brief Resets watchdog timer.
	 * @param (none)
	 * @return (none)
	 */
	static void resetWatchdogTimer()
	{
		m_watchdogTimer = 0;
	}

	/**
	 * @brief Checks if watchdog timeout is detected.
	 * @param (none)
	 * @return \c true if timeout is detected, \c false otherwise.
	 */
	static bool watchdogTimeoutDetected()
	{
		return m_watchdogTimeoutDetected;
	}

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
	static void registerWatchdogTask(ClockTaskStatus (*task)())
	{
		m_watchdogTask = task;
	}

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
	SystemClock();						// no constructor
	SystemClock(const SystemClock& other);			// no copy constructor
	SystemClock& operator=(const SystemClock& other);	// no copy assignment operator
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
	static uint64_t now()
	{
		return m_time;
	}

	/**
	 * @brief Returns clock step.
	 * @param (none)
	 * @return Clock step in milliseconds.
	 */
	static uint32_t step()
	{
		return TIME_STEP;
	}

	/**
	 * @brief Resets clock.
	 * @param (none)
	 * @return (none)
	 */
	static void reset()
	{
		m_time = 0;
		for (size_t i = 0; i < TASK_COUNT; ++i)
		{
			m_taskTimestamps[i] = now();
		}
	}

	/**
	 * @brief Checks and runs periodic and delayed tasks.
	 * @param (none)
	 * @return (none)
	 */
	static void runTasks();

protected:
	/**
	 * @brief Main system clock ISR.
	 * @param (none)
	 * @return (none)
	 */
	static __interrupt void onInterrupt();
};


/*####################################################################################################################*/
/**
 * @brief High resolution clock class. Based on CPU-Timer1.
 */
class HighResolutionClock : public emb::Monostate<HighResolutionClock>
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
	static void init(uint32_t period_us);

	/**
	 * @brief Returns systick timer counter value.
	 * @param (none)
	 * @return Systick timer counter value.
	 */
	static uint32_t counter()
	{
		return CPUTimer_getTimerCount(CPUTIMER1_BASE);
	}

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
	static void start()
	{
		CPUTimer_startTimer(CPUTIMER1_BASE);
	}

	/**
	 * @brief Stops systick timer.
	 * @param (none)
	 * @return (none)
	 */
	static void stop()
	{
		CPUTimer_stopTimer(CPUTIMER1_BASE);
	}

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
class Timeout
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
	Timeout(uint64_t timeout = 0)
		: m_timeout(timeout)
		, m_start(SystemClock::now())
	{}

	/**
	 * @brief Check if timeout is expired.
	 * @param (none)
	 * @return \c true if timeout is expired, \c false otherwise.
	 */
	bool expired() volatile
	{
		if (m_timeout == 0)
		{
			return false;
		}
		if ((SystemClock::now() - m_start) > m_timeout)
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
		m_start = SystemClock::now();
	}
};


/// @}
} // namespace mcu


