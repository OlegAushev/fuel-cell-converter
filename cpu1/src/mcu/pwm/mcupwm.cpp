/**
 * @file
 * @ingroup mcu mcu_pwm
 */


#include "mcupwm.h"


namespace mcu {

namespace detail {
	const uint32_t pwmBases[12] = {EPWM1_BASE, EPWM2_BASE, EPWM3_BASE, EPWM4_BASE,
			EPWM5_BASE, EPWM6_BASE, EPWM7_BASE, EPWM8_BASE,
			EPWM9_BASE, EPWM10_BASE, EPWM11_BASE, EPWM12_BASE};
	const uint32_t pwmPieIntNos[12] = {INT_EPWM1, INT_EPWM2, INT_EPWM3, INT_EPWM4,
			INT_EPWM5, INT_EPWM6, INT_EPWM7, INT_EPWM8,
			INT_EPWM9, INT_EPWM10, INT_EPWM11, INT_EPWM12};
	const uint32_t pwmPieTripIntNos[12] = {INT_EPWM1_TZ, INT_EPWM2_TZ, INT_EPWM3_TZ, INT_EPWM4_TZ,
			INT_EPWM5_TZ, INT_EPWM6_TZ, INT_EPWM7_TZ, INT_EPWM8_TZ,
			INT_EPWM9_TZ, INT_EPWM10_TZ, INT_EPWM11_TZ, INT_EPWM12_TZ};

	const uint32_t pwmPinOutAConfigs[12] = {GPIO_0_EPWM1A, GPIO_2_EPWM2A, GPIO_4_EPWM3A, GPIO_6_EPWM4A,
			GPIO_8_EPWM5A, GPIO_10_EPWM6A, GPIO_12_EPWM7A, GPIO_14_EPWM8A,
			GPIO_16_EPWM9A, GPIO_18_EPWM10A, GPIO_20_EPWM11A, GPIO_22_EPWM12A};
	const uint32_t pwmPinOutBConfigs[12] = {GPIO_1_EPWM1B, GPIO_3_EPWM2B, GPIO_5_EPWM3B, GPIO_7_EPWM4B,
			GPIO_9_EPWM5B, GPIO_11_EPWM6B, GPIO_13_EPWM7B, GPIO_15_EPWM8B,
			GPIO_17_EPWM9B, GPIO_19_EPWM10B, GPIO_21_EPWM11B, GPIO_23_EPWM12B};
}


///
///
///
template <PwmPhaseCount PhaseCount>
PwmUnit<PhaseCount>::PwmUnit(const PwmConfig<PhaseCount>& cfg)
	: m_switchingFreq(cfg.switchingFreq)
	, m_deadtimeCycles(cfg.deadtime_ns / CLK_CYCLE_NS)
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
	// Configure GPIO
	for (size_t i = 0; i < PhaseCount; ++i)
	{
		GPIO_setPadConfig(cfg.module[i] * 2, GPIO_PIN_TYPE_STD);
		GPIO_setPadConfig(cfg.module[i] * 2 + 1, GPIO_PIN_TYPE_STD);
		GPIO_setPinConfig(detail::pwmPinOutAConfigs[cfg.module[i]]);
		GPIO_setPinConfig(detail::pwmPinOutBConfigs[cfg.module[i]]);
	}
#else
	EMB_UNUSED(detail::pwmPinOutAConfigs);
	EMB_UNUSED(detail::pwmPinOutBConfigs);
#endif

	// Configure EPWM modules
	SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);	// Disable sync(Freeze clock to PWM as well)

	// Set-up TBCLK
	m_period = (CLK_FREQ / m_switchingFreq) / 2;	// UP-DOWN count - divide by 2

	for (size_t i = 0; i < PhaseCount; ++i)
	{
		EPWM_setTimeBasePeriod(m_module.base[i], m_period);
		EPWM_setTimeBaseCounter(m_module.base[i], 0);

		/* ========================================================================== */
		// Clock prescaler
		EPWM_setClockPrescaler(m_module.base[i], EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);

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
		if (PhaseCount != ONE_PHASE)
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
		case THREE_PHASE: case SIX_PHASE:
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

		case ONE_PHASE:
			EPWM_disablePhaseShiftLoad(m_module.base[i]);	// ignore the synchronization input pulse
			break;
		}

		/* ========================================================================== */
		// Configure shadowing
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

#ifndef PWM_OUT_POLARITY_ACTIVE_LOW
		EPWM_setDeadBandDelayPolarity(m_module.base[i], EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH);
		EPWM_setDeadBandDelayPolarity(m_module.base[i], EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);
#else
#warning "Active Low Complementary"
		EPWM_setDeadBandDelayPolarity(m_module.base[i], EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_LOW);
		EPWM_setDeadBandDelayPolarity(m_module.base[i], EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_HIGH);
#endif

		EPWM_setRisingEdgeDeadBandDelayInput(m_module.base[i], EPWM_DB_INPUT_EPWMA);
		EPWM_setFallingEdgeDeadBandDelayInput(m_module.base[i], EPWM_DB_INPUT_EPWMA);
		EPWM_setRisingEdgeDelayCount(m_module.base[i], m_deadtimeCycles);
		EPWM_setFallingEdgeDelayCount(m_module.base[i], m_deadtimeCycles);
		EPWM_setDeadBandCounterClock(m_module.base[i], EPWM_DB_COUNTER_CLOCK_FULL_CYCLE);

		/* ========================================================================== */
		// Configure trip-zone actions
#ifndef PWM_OUT_POLARITY_ACTIVE_LOW
		EPWM_setTripZoneAction(m_module.base[i], EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
		EPWM_setTripZoneAction(m_module.base[i], EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);
#else
#warning "Active Low Complementary"
		EPWM_setTripZoneAction(m_module.base[i], EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_HIGH);
		EPWM_setTripZoneAction(m_module.base[i], EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_HIGH);
#endif

		EPWM_clearOneShotTripZoneFlag(m_module.base[i], EPWM_TZ_OST_FLAG_OST1);
		EPWM_clearTripZoneFlag(m_module.base[i], EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_OST);
	}

	/* ========================================================================== */
	// Configure interrupts
	switch (PhaseCount)
	{
	case SIX_PHASE:
	case THREE_PHASE:
		// Configure interrupt to change the Compare Values, only phase A interrupt is required
		EPWM_setInterruptSource(m_module.base[0], EPWM_INT_TBCTR_ZERO_OR_PERIOD);
		EPWM_setInterruptEventCount(m_module.base[0], 1U);
		break;

	case ONE_PHASE:
		EPWM_setInterruptSource(m_module.base[0], EPWM_INT_TBCTR_ZERO);
		EPWM_setInterruptEventCount(m_module.base[0], 1U);
		break;
	}

	stop();
	SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);	// Enable sync and clock to PWM
}
// Explicit instantiation
template PwmUnit<ONE_PHASE>::PwmUnit(const PwmConfig<ONE_PHASE>& config);
template PwmUnit<THREE_PHASE>::PwmUnit(const PwmConfig<THREE_PHASE>& config);
template PwmUnit<SIX_PHASE>::PwmUnit(const PwmConfig<SIX_PHASE>& config);

#ifdef CPU1
///
///
///
template <PwmPhaseCount PhaseCount>
void PwmUnit<PhaseCount>::initTzSubmodule(const mcu::GpioPin& pin, XBAR_InputNum xbarInput)
{
	assert(static_cast<uint32_t>(xbarInput) <= static_cast<uint32_t>(XBAR_INPUT3));

	switch (pin.activeState)
	{
	case mcu::ACTIVE_LOW:
		GPIO_setPadConfig(pin.no, GPIO_PIN_TYPE_PULLUP);
		break;
	case mcu::ACTIVE_HIGH:
		GPIO_setPadConfig(pin.no, GPIO_PIN_TYPE_INVERT);
		break;
	}

	GPIO_setPinConfig(pin.mux);
	GPIO_setDirectionMode(pin.no, GPIO_DIR_MODE_IN);
	GPIO_setQualificationMode(pin.no, GPIO_QUAL_ASYNC);

	XBAR_setInputPin(xbarInput, pin.no);
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
// Explicit instantiation
template void PwmUnit<ONE_PHASE>::initTzSubmodule(const mcu::GpioPin& pin, XBAR_InputNum xbarInput);
template void PwmUnit<THREE_PHASE>::initTzSubmodule(const mcu::GpioPin& pin, XBAR_InputNum xbarInput);
template void PwmUnit<SIX_PHASE>::initTzSubmodule(const mcu::GpioPin& pin, XBAR_InputNum xbarInput);
#endif

#ifdef CPU1
///
///
///
template <PwmPhaseCount PhaseCount>
void PwmUnit<PhaseCount>::transferControlToCpu2()
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
// Explicit instantiation
template void PwmUnit<ONE_PHASE>::transferControlToCpu2();
template void PwmUnit<THREE_PHASE>::transferControlToCpu2();
template void PwmUnit<SIX_PHASE>::transferControlToCpu2();
#endif

} // namespace mcu


