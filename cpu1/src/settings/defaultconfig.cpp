/**
 * @file
 * @ingroup setup_manager
 */


#include "settings.h"


///
///
///
#pragma DATA_SECTION("SHARED_SYSTEMCONFIG")
Settings::SystemConfig Settings::SYSTEM_CONFIG;


///
///
///
const Settings::SystemConfig Settings::DEFAULT_CONFIG = {
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/*#############*/
/*# CONVERTER #*/
/*#############*/
.CONVERTER_CONFIG =
{
	.uvpIn = 2,
	.ovpIn = 350,
	.ucpIn = 0.3,
	.ocpIn = 45,
	.otpJunction = 105,
	.otpCase = 90,
	.fanTempThOn = 65,
	.fanTempThOff = 55,

	.kP_dutycycle = 0.001,
	.kI_dutycucle = 0.1,
	.kP_current = 1,
	.kI_current = 10,

	.cvIn = 160,
	.ccIn = 10,
	.cvOut = 365.0,

	.fltPin = mcu::GpioPin(),
},

/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/*#######*/
/*# PWM #*/
/*#######*/
.PWM_CONFIG =
{
	.module = {mcu::PWM1},
	.switchingFreq = 20000,
	.deadtime_ns = 1000,
	.clockPrescaler = 1,
	.clkDivider = mcu::PWM_CLOCK_DIVIDER_1,
	.hsclkDivider = mcu::PWM_HSCLOCK_DIVIDER_1,
	.operatingMode = mcu::PWM_ACTIVE_HIGH_COMPLEMENTARY,
	.counterMode = mcu::PWM_COUNTER_MODE_UP,
#ifdef CRD300
	.outputSwap = mcu::PWM_OUTPUT_SWAP,
#else
	.outputSwap = mcu::PWM_OUTPUT_NO_SWAP,
#endif
	.eventInterruptSource = EPWM_INT_TBCTR_ZERO,
	.adcTriggerEnable = {true, true},
	.adcTriggerSource = {EPWM_SOC_TBCTR_ZERO, EPWM_SOC_TBCTR_U_CMPA}
},


};


