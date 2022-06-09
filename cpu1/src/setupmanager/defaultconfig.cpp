/**
 * @file
 * @ingroup setup_manager
 */


#include "setupmanager.h"


///
///
///
#pragma DATA_SECTION("SHARED_SYSTEMCONFIG")
SetupManager::SystemConfig SetupManager::SYSTEM_CONFIG;


///
///
///
const SetupManager::SystemConfig SetupManager::DEFAULT_CONFIG = {
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/*#############*/
/*# CONVERTER #*/
/*#############*/
.CONVERTER_CONFIG =
{
	.uvpIn = 0,
	.ovpIn = 400,
	.ocpIn = 400,
	.otpJunction = 105,
	.otpCase = 90,
	.fanTempThOn = 65,
	.fanTempThOff = 55,
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
	.switchingFreq = 10000,
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


