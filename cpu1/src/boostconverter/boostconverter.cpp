/**
 * @file
 * @ingroup boost_converter
 */


#include "boostconverter.h"


///
///
///
BoostConverter::BoostConverter(const BoostConverterConfig& converterConfig,
		const mcu::PwmConfig<mcu::PWM_ONE_PHASE>& pwmConfig)
	: FLT_PIN(converterConfig.fltPin)
	, m_state(CONVERTER_OFF)
	, m_voltageIn(VDC_SMOOTH_FACTOR)
	, m_voltageOut(VDC_SMOOTH_FACTOR)
	, m_currentIn(0, 0)
	, OVP_IN_BOUND(converterConfig.ovpIn)
	, UVP_IN_BOUND(converterConfig.uvpIn)
	, OCP_IN_BOUND(converterConfig.ocpIn)
	, OTP_MODULE_BOUND(converterConfig.otpJunction)
	, OTP_CASE_BOUND(converterConfig.otpCase)
	, FAN_TEMP_TH_ON(converterConfig.fanTempThOn)
	, FAN_TEMP_TH_OFF(converterConfig.fanTempThOff)
	, pwmUnit(pwmConfig)
{
#ifdef CRD300
	pwmUnit.initTzSubmodule(FLT_PIN, XBAR_INPUT1);
#endif
	pwmUnit.registerInterruptHandler(onPwmInterrupt);
}



