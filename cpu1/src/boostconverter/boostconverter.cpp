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
	: emb::c28x::Singleton<BoostConverter>(this)
	, FLT_PIN(converterConfig.fltPin)
	, UVP_IN_BOUND(converterConfig.uvpIn)
	, OVP_IN_BOUND(converterConfig.ovpIn)
	, UCP_IN_BOUND(converterConfig.ucpIn)
	, OCP_IN_BOUND(converterConfig.ocpIn)
	, OTP_MODULE_BOUND(converterConfig.otpJunction)
	, OTP_CASE_BOUND(converterConfig.otpCase)
	, FAN_TEMP_TH_ON(converterConfig.fanTempThOn)
	, FAN_TEMP_TH_OFF(converterConfig.fanTempThOff)
	, CV_IN_BOUND(converterConfig.cvIn)
	, CC_IN_BOUND(converterConfig.ccIn)
	, CV_OUT_BOUND(converterConfig.cvOut)
	, m_state(CONVERTER_OFF)
	, pwmUnit(pwmConfig)
	, m_voltageIn(VDC_SMOOTH_FACTOR)
	, m_voltageOut(VDC_SMOOTH_FACTOR)
	, m_currentIn(0, 0)
	, m_dutycycleController(converterConfig.kP_dutycycle, converterConfig.kI_dutycucle,
			1 / pwmConfig.switchingFreq, 0, 0.9f)
	, m_currentController(converterConfig.kP_current, converterConfig.kI_current,
			1 / pwmConfig.switchingFreq, converterConfig.ucpIn, converterConfig.ccIn)
{
#ifdef CRD300
	pwmUnit.initTzSubmodule(FLT_PIN, XBAR_INPUT1);
#endif
	pwmUnit.registerEventInterruptHandler(onPwmEventInterrupt);
	pwmUnit.registerTripInterruptHandler(onPwmTripInterrupt);

	inVoltageSensor.adcUnit->registerInterruptHandler(mcu::ADC_IRQ_VOLTAGE_IN, onAdcVoltageInInterrupt);
	outVoltageSensor.adcUnit->registerInterruptHandler(mcu::ADC_IRQ_VOLTAGE_OUT, onAdcVoltageOutInterrupt);
	inCurrentSensor.adcUnit->registerInterruptHandler(mcu::ADC_IRQ_CURRENT_IN_FIRST, onAdcCurrentInFirstInterrupt);
	inCurrentSensor.adcUnit->registerInterruptHandler(mcu::ADC_IRQ_CURRENT_IN_SECOND, onAdcCurrentInSecondInterrupt);
}


///
///
///
void BoostConverter::reset()
{

}



///
///
///
__interrupt void BoostConverter::onPwmEventInterrupt()
{
	LOG_DURATION_VIA_PIN_ONOFF(22);
	//BoostConverter::instance()->inCurrentSensor.convert();
	//BoostConverter::instance()->inVoltageSensor.convert();
	//BoostConverter::instance()->outVoltageSensor.convert();

	BoostConverter::instance()->pwmUnit.acknowledgeInterrupt();
}


///
///
///
__interrupt void BoostConverter::onPwmTripInterrupt()
{
	BoostConverter::instance()->pwmUnit.acknowledgeTripInterrupt();
}


///
///
///
__interrupt void BoostConverter::onAdcVoltageInInterrupt()
{
	LOG_DURATION_VIA_PIN_ONOFF(61);
	BoostConverter* converter = BoostConverter::instance();
	converter->m_voltageIn.process(converter->inVoltageSensor.reading());

	converter->inVoltageSensor.adcUnit->acknowledgeInterrupt(mcu::ADC_IRQ_VOLTAGE_IN);
}


///
///
///
__interrupt void BoostConverter::onAdcVoltageOutInterrupt()
{
	LOG_DURATION_VIA_PIN_ONOFF(111)
	BoostConverter* converter = BoostConverter::instance();
	converter->m_voltageOut.process(converter->outVoltageSensor.reading());

	converter->outVoltageSensor.adcUnit->acknowledgeInterrupt(mcu::ADC_IRQ_VOLTAGE_OUT);
}


///
///
///
__interrupt void BoostConverter::onAdcCurrentInFirstInterrupt()
{
	LOG_DURATION_VIA_PIN_ONOFF(123);
	BoostConverter* converter = BoostConverter::instance();
	converter->m_currentIn.first = -1.f * converter->inCurrentSensor.reading(InCurrentSensor::FIRST);

	converter->inCurrentSensor.adcUnit->acknowledgeInterrupt(mcu::ADC_IRQ_CURRENT_IN_FIRST);
}

float VIN = 0;
float IIN = 0;
///
///
///
__interrupt void BoostConverter::onAdcCurrentInSecondInterrupt()
{
	LOG_DURATION_VIA_PIN_ONOFF(122);
	BoostConverter* converter = BoostConverter::instance();
	converter->m_currentIn.second = -1.f * converter->inCurrentSensor.reading(InCurrentSensor::SECOND);
	converter->m_currentInAvg = (converter->m_currentIn.first + converter->m_currentIn.second) / 2;

	// TEST
	//converter->m_voltageIn.setOutput(VIN);
	//converter->m_currentInAvg = IIN;
	// TEST

	converter->m_currentController.process(converter->CV_IN_BOUND, converter->m_voltageIn.output());
	converter->m_dutycycleController.process(converter->m_currentController.output(),
			converter->m_currentInAvg);
	converter->pwmUnit.setDutyCycle(converter->m_dutycycleController.output());

	converter->inCurrentSensor.adcUnit->acknowledgeInterrupt(mcu::ADC_IRQ_CURRENT_IN_SECOND);
}


















