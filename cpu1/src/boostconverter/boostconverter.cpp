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
	pwmUnit.registerTripInterruptHandler(onPwmTripInterrupt);

	inVoltageSensor.adcUnit->registerInterruptHandler(mcu::ADC_IRQ_VOLTAGE_IN, onAdcVoltageInInterrupt);
	outVoltageSensor.adcUnit->registerInterruptHandler(mcu::ADC_IRQ_VOLTAGE_OUT, onAdcVoltageOutInterrupt);
	inCurrentSensor.adcUnit->registerInterruptHandler(mcu::ADC_IRQ_CURRENT_IN_FIRST, onAdcCurrentInFirstInterrupt);
	inCurrentSensor.adcUnit->registerInterruptHandler(mcu::ADC_IRQ_CURRENT_IN_SECOND, onAdcCurrentInSecondInterrupt);
}


///
///
///
__interrupt void BoostConverter::onPwmInterrupt()
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
	BoostConverter* converter = BoostConverter::instance();
	converter->m_voltageIn.process(converter->outVoltageSensor.reading());
	converter->outVoltageSensor.adcUnit->acknowledgeInterrupt(mcu::ADC_IRQ_VOLTAGE_OUT);
}


///
///
///
__interrupt void BoostConverter::onAdcCurrentInFirstInterrupt()
{
	LOG_DURATION_VIA_PIN_ONOFF(123);
	BoostConverter* converter = BoostConverter::instance();
	converter->m_currentIn.first = converter->inCurrentSensor.reading(InCurrentSensor::FIRST);
	converter->inCurrentSensor.adcUnit->acknowledgeInterrupt(mcu::ADC_IRQ_CURRENT_IN_FIRST);
}


///
///
///
__interrupt void BoostConverter::onAdcCurrentInSecondInterrupt()
{
	LOG_DURATION_VIA_PIN_ONOFF(122);
	BoostConverter* converter = BoostConverter::instance();
	converter->m_currentIn.second = converter->inCurrentSensor.reading(InCurrentSensor::SECOND);
	converter->inCurrentSensor.adcUnit->acknowledgeInterrupt(mcu::ADC_IRQ_CURRENT_IN_SECOND);
}


















