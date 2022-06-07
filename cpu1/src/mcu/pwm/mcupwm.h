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
	PWM_ONE_PHASE = 1,	///< ONE_PHASE
	PWM_THREE_PHASE = 3,	///< THREE_PHASE
	PWM_SIX_PHASE = 6	///< SIX_PHASE
};


/// PWM clock divider
enum PwmClockDivider
{
	PWM_CLOCK_DIVIDER_1 = EPWM_CLOCK_DIVIDER_1,		//!< Divide clock by 1
	PWM_CLOCK_DIVIDER_2 = EPWM_CLOCK_DIVIDER_2,		//!< Divide clock by 2
	PWM_CLOCK_DIVIDER_4 = EPWM_CLOCK_DIVIDER_4,		//!< Divide clock by 4
	PWM_CLOCK_DIVIDER_8 = EPWM_CLOCK_DIVIDER_8,		//!< Divide clock by 8
	PWM_CLOCK_DIVIDER_16 = EPWM_CLOCK_DIVIDER_16,		//!< Divide clock by 16
	PWM_CLOCK_DIVIDER_32 = EPWM_CLOCK_DIVIDER_32,		//!< Divide clock by 32
	PWM_CLOCK_DIVIDER_64 = EPWM_CLOCK_DIVIDER_64,		//!< Divide clock by 64
	PWM_CLOCK_DIVIDER_128 = EPWM_CLOCK_DIVIDER_128		//!< Divide clock by 128
};


/// Pwm hs clock divider
enum PwmHsClockDivider
{
	PWM_HSCLOCK_DIVIDER_1 = EPWM_HSCLOCK_DIVIDER_1,		//!< Divide clock by 1
	PWM_HSCLOCK_DIVIDER_2 = EPWM_HSCLOCK_DIVIDER_2,		//!< Divide clock by 2
	PWM_HSCLOCK_DIVIDER_4 = EPWM_HSCLOCK_DIVIDER_4,		//!< Divide clock by 4
	PWM_HSCLOCK_DIVIDER_6 = EPWM_HSCLOCK_DIVIDER_6,		//!< Divide clock by 6
	PWM_HSCLOCK_DIVIDER_8 = EPWM_HSCLOCK_DIVIDER_8,		//!< Divide clock by 8
	PWM_HSCLOCK_DIVIDER_10 = EPWM_HSCLOCK_DIVIDER_10,	//!< Divide clock by 10
	PWM_HSCLOCK_DIVIDER_12 = EPWM_HSCLOCK_DIVIDER_12,	//!< Divide clock by 12
	PWM_HSCLOCK_DIVIDER_14 = EPWM_HSCLOCK_DIVIDER_14	//!< Divide clock by 14
};


/// Pwm mode (waveform)
enum PwmMode
{
	ACTIVE_HIGH_COMPLEMENTARY,
	ACTIVE_LOW_COMPLEMENTARY
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
	uint32_t clockPrescaler;	// must be the product of clkDivider and hsclkDivider
	PwmClockDivider clkDivider;
	PwmHsClockDivider hsclkDivider;
	PwmMode mode;
};


namespace detail {


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


extern const uint32_t pwmBases[12];
extern const uint32_t pwmPieIntNos[12];
extern const uint32_t pwmPieTripIntNos[12];
extern const uint32_t pwmPinOutAConfigs[12];
extern const uint32_t pwmPinOutBConfigs[12];


} // namespace detail


/**
 * @brief PWM unit class.
 */
template <PwmPhaseCount PhaseCount>
class PwmUnit
{
private:
	// there is a divider ( EPWMCLKDIV ) of the system clock
	// which defaults to EPWMCLK = SYSCLKOUT/2, fclk(epwm)max = 100 MHz
	static const uint32_t PWMCLK_FREQ = DEVICE_SYSCLK_FREQ / 2;
	static const uint32_t PWMCLK_CYCLE_NS = 1000000000 / PWMCLK_FREQ;
	const uint32_t TBCLK_FREQ;
	const uint32_t TBCLK_CYCLE_NS;

	detail::PwmModuleImpl<PhaseCount> m_module;
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
	PwmUnit(const PwmConfig<PhaseCount>& cfg)
		: TBCLK_FREQ(PWMCLK_FREQ / cfg.clockPrescaler)
		, TBCLK_CYCLE_NS(PWMCLK_CYCLE_NS * cfg.clockPrescaler)
		, m_switchingFreq(cfg.switchingFreq)
		, m_deadtimeCycles(cfg.deadtime_ns / TBCLK_CYCLE_NS)
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			m_module.instance[i] = cfg.module[i];
			m_module.base[i] = detail::pwmBases[cfg.module[i]];
		}
		m_module.pieIntNo = detail::pwmPieIntNos[cfg.module[0]];
		m_module.pieTripIntNo = detail::pwmPieTripIntNos[cfg.module[0]];

		for (size_t i = 0; i < PhaseCount; ++i)
		{
			m_phaseShift[i] = 0;
		}

#ifdef CPU1
		_initPins(cfg);
#else
		EMB_UNUSED(detail::pwmPinOutAConfigs);
		EMB_UNUSED(detail::pwmPinOutBConfigs);
#endif

		// Configure EPWM modules
		SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);	// Disable sync(Freeze clock to PWM as well)

		// Set-up TBCLK
		m_period = (TBCLK_FREQ / m_switchingFreq) / 2;	// UP-DOWN count - divide by 2

		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setTimeBasePeriod(m_module.base[i], m_period);
			EPWM_setTimeBaseCounter(m_module.base[i], 0);

			/* ========================================================================== */
			// Clock prescaler
			EPWM_setClockPrescaler(m_module.base[i],
					static_cast<EPWM_ClockDivider>(cfg.clkDivider),
					static_cast<EPWM_HSClockDivider>(cfg.hsclkDivider));

			/* ========================================================================== */
			// Set compare values
			EPWM_setCounterCompareValue(m_module.base[i], EPWM_COUNTER_COMPARE_A, 0);

			/* ========================================================================== */
			// Set up counter mode
			EPWM_setTimeBaseCounterMode(m_module.base[i], EPWM_COUNTER_MODE_UP_DOWN);

#ifdef CPU1
			/* ========================================================================== */
			//Configure the sync input source for the EPWM signals
			switch (m_module.base[i])
			{
			case EPWM4_BASE:
				SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM4, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
				break;
			case EPWM7_BASE:
				SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM7, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
				break;
			case EPWM10_BASE:
				SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM10, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
				break;
			default:
				break;
			}
#endif

			/* ========================================================================== */
			// Set up the sync out pulse event
			if (PhaseCount != PWM_ONE_PHASE)
			{
				if (i == 0)
				{
					// first module is master
					EPWM_setSyncOutPulseMode(m_module.base[i], EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO);
				}
				else
				{
					// other modules sync is pass-thru
					EPWM_setSyncOutPulseMode(m_module.base[i], EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);
				}
			}
			else
			{
				EPWM_setSyncOutPulseMode(m_module.base[i], EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);
			}

			/* ========================================================================== */
			// Time-base counter synchronization and phase shift
			switch (PhaseCount)
			{
			case PWM_THREE_PHASE: case PWM_SIX_PHASE:
				if ((i == 0) && (m_module.base[i] == EPWM1_BASE))
				{
					// EPWM1 is master, EPWM4,7,10 are synced to it if they are not ONE_PHASE
					EPWM_setPhaseShift(m_module.base[i], 0);
					EPWM_disablePhaseShiftLoad(m_module.base[i]);	// master has no phase shift
				}
				else
				{
					EPWM_enablePhaseShiftLoad(m_module.base[i]);
					EPWM_setCountModeAfterSync(m_module.base[i],
							EPWM_COUNT_MODE_UP_AFTER_SYNC);	// start counting up when sync occurs
					// 2 x EPWMCLK - delay from internal master module to slave modules, p.1876
					EPWM_setPhaseShift(m_module.base[i], 2 + m_phaseShift[i]);
				}
				break;

			case PWM_ONE_PHASE:
				EPWM_disablePhaseShiftLoad(m_module.base[i]);	// ignore the synchronization input pulse
				break;
			}

			/* ========================================================================== */
			// Configure shadowing
			EPWM_selectPeriodLoadEvent(m_module.base[i], EPWM_SHADOW_LOAD_MODE_COUNTER_ZERO);
			EPWM_setCounterCompareShadowLoadMode(m_module.base[i], EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);
			EPWM_setActionQualifierContSWForceShadowMode(m_module.base[i], EPWM_AQ_SW_IMMEDIATE_LOAD);

			/* ========================================================================== */
			// Set CMPA actions
			EPWM_setActionQualifierAction(m_module.base[i],
					EPWM_AQ_OUTPUT_A,
					EPWM_AQ_OUTPUT_HIGH,
					EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
			EPWM_setActionQualifierAction(m_module.base[i],
					EPWM_AQ_OUTPUT_A,
					EPWM_AQ_OUTPUT_LOW,
					EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
			EPWM_setActionQualifierAction(m_module.base[i],
					EPWM_AQ_OUTPUT_B,
					EPWM_AQ_OUTPUT_HIGH,
					EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
			EPWM_setActionQualifierAction(m_module.base[i],
					EPWM_AQ_OUTPUT_B,
					EPWM_AQ_OUTPUT_LOW,
					EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

			/* ========================================================================== */
			// Set Deadband Active High Complementary
			EPWM_setDeadBandControlShadowLoadMode(m_module.base[i], EPWM_DB_LOAD_ON_CNTR_ZERO);
			EPWM_setDeadBandDelayMode(m_module.base[i], EPWM_DB_FED, true);
			EPWM_setDeadBandDelayMode(m_module.base[i], EPWM_DB_RED, true);

			switch (cfg.mode)
			{
			case ACTIVE_HIGH_COMPLEMENTARY:
				EPWM_setDeadBandDelayPolarity(m_module.base[i], EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH);
				EPWM_setDeadBandDelayPolarity(m_module.base[i], EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);
				break;
			case ACTIVE_LOW_COMPLEMENTARY:
				EPWM_setDeadBandDelayPolarity(m_module.base[i], EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_LOW);
				EPWM_setDeadBandDelayPolarity(m_module.base[i], EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_HIGH);
				break;
			}

			EPWM_setRisingEdgeDeadBandDelayInput(m_module.base[i], EPWM_DB_INPUT_EPWMA);
			EPWM_setFallingEdgeDeadBandDelayInput(m_module.base[i], EPWM_DB_INPUT_EPWMA);
			EPWM_setRisingEdgeDelayCount(m_module.base[i], m_deadtimeCycles);
			EPWM_setFallingEdgeDelayCount(m_module.base[i], m_deadtimeCycles);
			EPWM_setDeadBandCounterClock(m_module.base[i], EPWM_DB_COUNTER_CLOCK_FULL_CYCLE);

			/* ========================================================================== */
			// Configure trip-zone actions
			switch (cfg.mode)
			{
			case ACTIVE_HIGH_COMPLEMENTARY:
				EPWM_setTripZoneAction(m_module.base[i], EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
				EPWM_setTripZoneAction(m_module.base[i], EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);
				break;
			case ACTIVE_LOW_COMPLEMENTARY:
				EPWM_setTripZoneAction(m_module.base[i], EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_HIGH);
				EPWM_setTripZoneAction(m_module.base[i], EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_HIGH);
				break;
			}

			EPWM_clearOneShotTripZoneFlag(m_module.base[i], EPWM_TZ_OST_FLAG_OST1);
			EPWM_clearTripZoneFlag(m_module.base[i], EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_OST);
		}

		/* ========================================================================== */
		// Configure interrupts
		switch (PhaseCount)
		{
		case PWM_SIX_PHASE:
		case PWM_THREE_PHASE:
			// Configure interrupt to change the Compare Values, only phase A interrupt is required
			EPWM_setInterruptSource(m_module.base[0], EPWM_INT_TBCTR_ZERO_OR_PERIOD);
			EPWM_setInterruptEventCount(m_module.base[0], 1U);
			break;

		case PWM_ONE_PHASE:
			EPWM_setInterruptSource(m_module.base[0], EPWM_INT_TBCTR_ZERO);
			EPWM_setInterruptEventCount(m_module.base[0], 1U);
			break;
		}

		stop();
		SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);	// Enable sync and clock to PWM
	}

#ifdef CPU1
	/**
	 * @brief Initializes PWM trip-zone submodule.
	 * @param pin - trip input
	 * @return (none)
	 */
	void initTzSubmodule(const mcu::GpioPin& pin, XBAR_InputNum xbarInput)
	{
		assert(static_cast<uint32_t>(xbarInput) <= static_cast<uint32_t>(XBAR_INPUT3));

		switch (pin.config().activeState)
		{
		case mcu::ACTIVE_LOW:
			GPIO_setPadConfig(pin.config().no, GPIO_PIN_TYPE_PULLUP);
			break;
		case mcu::ACTIVE_HIGH:
			GPIO_setPadConfig(pin.config().no, GPIO_PIN_TYPE_INVERT);
			break;
		}

		GPIO_setPinConfig(pin.config().mux);
		GPIO_setDirectionMode(pin.config().no, GPIO_DIR_MODE_IN);
		GPIO_setQualificationMode(pin.config().no, GPIO_QUAL_ASYNC);

		XBAR_setInputPin(xbarInput, pin.config().no);
		uint16_t tzSignal;
		switch (xbarInput)
		{
		case XBAR_INPUT1:
			tzSignal = EPWM_TZ_SIGNAL_OSHT1;
			break;
		case XBAR_INPUT2:
			tzSignal = EPWM_TZ_SIGNAL_OSHT2;
			break;

		case XBAR_INPUT3:
			tzSignal = EPWM_TZ_SIGNAL_OSHT3;
			break;
		default:
			tzSignal = EPWM_TZ_SIGNAL_OSHT3;
			break;
		}

		for (size_t i = 0; i < PhaseCount; ++i)
		{
			// Enable tzSignal as one shot trip source
			EPWM_enableTripZoneSignals(m_module.base[i], tzSignal);
		}
	}
#endif

#ifdef CPU1
	/**
	 * @brief Transfers control over PWM unit to CPU2.
	 * @param (none)
	 * @return (none)
	 */
	void transferControlToCpu2()
	{
		SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);	// Disable sync(Freeze clock to PWM as well)

		for (size_t i = 0; i < PhaseCount; ++i)
		{
			GPIO_setMasterCore(m_module.instance[i] * 2, GPIO_CORE_CPU2);
			GPIO_setMasterCore(m_module.instance[i] * 2 + 1, GPIO_CORE_CPU2);
		}

		for (size_t i = 0; i < PhaseCount; ++i)
		{
			SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL0_EPWM,
					static_cast<uint16_t>(m_module.instance[i])+1, SYSCTL_CPUSEL_CPU2);
		}

		SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);	// Enable sync and clock to PWM
	}
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
	 * @brief Returns base of PWM-unit.
	 * @param (none)
	 * @return Base of PWM-unit.
	 */
	uint32_t base() const { return m_module.base[0]; }

	/**
	 * @brief Returns PWM time-base period value.
	 * @param (none)
	 * @return TBPRD register value.
	 */
	uint16_t period() const { return m_period; }

	/**
	 * @brief Returns PWM frequency.
	 * @param (none)
	 * @return PWM frequency.
	 */
	float freq() const { return m_switchingFreq; }

	/**
	 * @brief Sets PWM frequency.
	 * @param freq - PWM frequency
	 * @return (none)
	 */
	void setFreq(float freq)
	{
		m_switchingFreq = freq;
		// UP-DOWN count - divide by 2
		m_period = (TBCLK_FREQ / m_switchingFreq) / 2;
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setTimeBasePeriod(m_module.base[i], m_period);
		}
	}

	/**
	 * @brief Sets counter compare values.
	 * @param cmpValues - compare values array pointer
	 * @return (none)
	 */
	void setCompareValues(const uint16_t cmpValues[]) const
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
	void setCompareValues(const emb::Array<uint16_t, PhaseCount>& cmpValues) const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(m_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            cmpValues[i]);
		}
	}

	/**
	 * @brief Sets one counter compare value for all modules.
	 * @param cmpValue - compare value
	 * @return (none)
	 */
	void setCompareValue(uint16_t cmpValue) const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(m_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            cmpValue);
		}
	}

	/**
	 * @brief Updates counter compare values.
	 * @param dutyCycles - reference to duty cycles array
	 * @return (none)
	 */
	void setDutyCycles(const emb::Array<float, PhaseCount>& dutyCycles) const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(m_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            static_cast<uint16_t>(dutyCycles[i] * m_period));
		}
	}

	/**
	 * @brief Updates counter compare values.
	 * @param dutyCycle - duty cycle value for all modules
	 * @return (none)
	 */
	void setDutyCycle(float dutyCycle) const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(m_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            static_cast<uint16_t>(dutyCycle * m_period));
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

protected:
#ifdef CPU1
	void _initPins(const PwmConfig<PhaseCount>& cfg)
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			GPIO_setPadConfig(cfg.module[i] * 2, GPIO_PIN_TYPE_STD);
			GPIO_setPadConfig(cfg.module[i] * 2 + 1, GPIO_PIN_TYPE_STD);
			GPIO_setPinConfig(detail::pwmPinOutAConfigs[cfg.module[i]]);
			GPIO_setPinConfig(detail::pwmPinOutBConfigs[cfg.module[i]]);
		}
	}
#endif
};


/// @}
} // namespace mcu


