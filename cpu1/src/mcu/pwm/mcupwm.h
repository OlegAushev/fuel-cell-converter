/**
 * @defgroup mcu_pwm PWM
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_pwm
 */


#pragma once


#include "driverlib.h"
#include "device.h"

#include "mcu/gpio/mcugpio.h"
#include "emb/emb_common.h"
#include "emb/emb_array.h"
#include <math.h>


namespace mcu {
/// @addtogroup mcu_pwm
/// @{

/// PWM modules
enum PwmModule
{
	PWM1,
	PWM2,
	PWM3,
	PWM4,
	PWM5,
	PWM6,
	PWM7,
	PWM8,
	PWM9,
	PWM10,
	PWM11,
	PWM12
};

/// PWM phases count
enum PwmPhaseCount
{
	ONE_PHASE = 1,		///< ONE_PHASE
	THREE_PHASE = 3,	///< THREE_PHASE
	SIX_PHASE = 6		///< SIX_PHASE
};

/**
 * @brief PWM module implementation.
 */
template <PwmPhaseCount PhaseCount>
struct PwmModuleImpl
{
	PwmModule instance[PhaseCount];
	uint32_t base[PhaseCount];
	uint32_t pieIntNo;
	uint32_t pieTripIntNo;
};

/**
 * @brief PWM config.
 */
template <PwmPhaseCount PhaseCount>
struct PwmConfig
{
	PwmModule module[PhaseCount];
	float switchingFreq;
	float deadtime_ns;
};

/**
 * @brief PWM unit class.
 */
template <PwmPhaseCount PhaseCount>
class PwmUnit
{
private:
	// there is a divider ( EPWMCLKDIV ) of the system clock
	// which defaults to EPWMCLK = SYSCLKOUT/2, fclk(epwm)max = 100 MHz
	static const uint32_t CLK_FREQ = DEVICE_SYSCLK_FREQ / 2;
	static const uint32_t CLK_CYCLE_NS = 1000000000 / CLK_FREQ;

	PwmModuleImpl<PhaseCount> m_module;
	float m_switchingFreq;
	uint16_t m_deadtimeCycles;

	uint16_t m_period;			// TBPRD register value
	uint16_t m_phaseShift[PhaseCount];	// TBPHS registers values

	PwmUnit(const PwmUnit& other);			// no copy constructor
	PwmUnit& operator=(const PwmUnit& other);	// no copy assignment operator

public:
	/**
	 * @brief Initializes MCU PWM unit.
	 * @param config - PWM config
	 * @param (none)
	 */
	PwmUnit(const PwmConfig<PhaseCount>& cfg);

#ifdef CPU1
	/**
	 * @brief Initializes PWM trip-zone submodule.
	 * @param pin - trip input
	 * @return (none)
	 */
	void initTzSubmodule(const mcu::GpioPin& pin, XBAR_InputNum xbarInput);
#endif

#ifdef CPU1
	/**
	 * @brief Transfers control over PWM unit to CPU2.
	 * @param (none)
	 * @return (none)
	 */
	void transferControlToCpu2();
#endif

	/**
	 * @brief Enables PWM and trip-zone interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts() const
	{
		EPWM_enableInterrupt(m_module.base[0]);
		EPWM_enableTripZoneInterrupt(m_module.base[0], EPWM_TZ_INTERRUPT_OST);
	}

	/**
	 * @brief Returns PWM frequency.
	 * @param (none)
	 * @return PWM frequency.
	 */
	float freq() const { return m_switchingFreq; }

	/**
	 * @brief Returns PWM time-base period value.
	 * @param (none)
	 * @return TBPRD register value.
	 */
	uint16_t period() const { return m_period; }

	/**
	 * @brief Updates counter compare values.
	 * @param cmpValues - Compare values array pointer
	 * @return (none)
	 */
	void updateCompareValues(const uint16_t cmpValues[]) const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(m_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            cmpValues[i]);
		}
	}

	/**
	 * @overload
	 */
	void updateCompareValues(const emb::Array<uint16_t, PhaseCount>& cmpValues) const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(m_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            cmpValues[i]);
		}
	}

	/**
	 * @brief Starts PWM.
	 * @param (none)
	 * @return (none)
	 */
	void start() const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_clearTripZoneFlag(m_module.base[i], EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_OST);
		}
	}

	/**
	 * @brief Stops PWM.
	 * @param (none)
	 * @return (none)
	 */
	void stop() const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_forceTripZoneEvent(m_module.base[i], EPWM_TZ_FORCE_EVENT_OST);
		}
	}

	/**
	 * @brief Returns base of PWM-unit.
	 * @param (none)
	 * @return Base of PWM-unit.
	 */
	uint32_t base() const { return m_module.base[0]; }

/*============================================================================*/
/*============================ Interrupts ====================================*/
/*============================================================================*/
	/**
	 * @param Registers time-base interrupt handler
	 * @param handler - pointer to handler
	 * @return (none)
	 */
	void registerInterruptHandler(void (*handler)(void)) const
	{
		Interrupt_register(m_module.pieIntNo, handler);
		Interrupt_enable(m_module.pieIntNo);
	}

	/**
	 * @param Registers trip event interrupt handler
	 * @param handler - pointer to handler
	 * @return (none)
	 */
	void registerTripInterruptHandler(void (*handler)(void)) const
	{
		Interrupt_register(m_module.pieTripIntNo, handler);
		Interrupt_enable(m_module.pieTripIntNo);
	}

	/**
	 * @brief Acknowledges time-base interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeInterrupt() const
	{
		EPWM_clearEventTriggerInterruptFlag(m_module.base[0]);
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
	}

	/**
	 * @brief Acknowledges trip event interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeTripInterrupt() const
	{
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP2);
	}
};


/// @}
} /* namespace mcu */


