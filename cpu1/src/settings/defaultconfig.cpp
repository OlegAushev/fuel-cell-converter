/**
 * @file
 * @ingroup setup_manager
 */


#include "settings.h"


///
///
///
Settings::SystemConfig Settings::SYSTEM_CONFIG __attribute__((section("SHARED_SYSTEMCONFIG")));


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
	.uvpVoltageIn = 2,
	.ovpVoltageIn = 350,
	.ovpVoltageOut = 500,
	.ocpCurrentIn = 45,

	.otpTempJunction = 105,
	.otpTempHeatsink = 95,
	.fanTempThOn = 65,
	.fanTempThOff = 55,

	.kP_dutycycle = 0.001,
	.kI_dutycucle = 0.1,
	.kP_current = 1,
	.kI_current = 0.1,//10 - for control by Vin

	.fuelCellVoltageMin = 120,
	.cvVoltageIn = 160,
	.currentInMin = 5,
	.currentInMax = 30,
	.batteryChargedVoltage = 375.0,

	.fltPin = mcu::GpioInput(),
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


