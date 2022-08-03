/**
 * @file
 * @ingroup fuel_cell_converter
 */


#include "fuelcell_converter.h"


namespace fuelcell {


#ifndef CRD300
const mcu::GpioConfig rstPinCfg(24, GPIO_24_GPIO24, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioConfig errPinCfg(16, GPIO_16_GPIO16, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioConfig relPinCfg(125, GPIO_125_GPIO125, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
#else
const mcu::GpioConfig rstPinCfg;
const mcu::GpioConfig errPinCfg;
const mcu::GpioConfig relPinCfg;
#endif


///
///
///
Converter::Converter(const ConverterConfig& converterConfig,
		const mcu::PwmConfig<mcu::PWM_ONE_PHASE>& pwmConfig)
	: emb::c28x::Singleton<Converter>(this)
	, m_state(STANDBY_State::instance())
	, m_stateId(CONVERTER_STANDBY)
	, m_config(converterConfig)
	, m_voltageInFilter(VDC_SMOOTH_FACTOR)
	, m_voltageOutFilter(VDC_SMOOTH_FACTOR)
	, m_tempHeatsinkFilter(TEMP_SMOOTH_FACTOR)
	, m_currentIn(0, 0)
	, m_currentInFilter(IDC_SMOOTH_FACTOR)
	, m_dutycycleController(converterConfig.kP_dutycycle, converterConfig.kI_dutycucle,
			1 / pwmConfig.switchingFreq, 0, 0.55f)
	, m_currentController(converterConfig.kP_current, converterConfig.kI_current,
			1 / pwmConfig.switchingFreq, converterConfig.currentInMin, converterConfig.currentInMax)
	, FLT_PIN(converterConfig.fltPin)
	, RST_PIN(rstPinCfg)
	, ERR_PIN(errPinCfg)
	, REL_PIN(relPinCfg)
	, pwm(pwmConfig)
{
#ifdef CRD300
	pwm.initTzSubmodule(FLT_PIN, XBAR_INPUT1);
#endif
	pwm.registerEventInterruptHandler(onPwmEventInterrupt);
	pwm.registerTripInterruptHandler(onPwmTripInterrupt);

	mcu::Adc::instance()->registerInterruptHandler(mcu::ADC_IRQ_VOLTAGE_IN, onAdcVoltageInInterrupt);
	mcu::Adc::instance()->registerInterruptHandler(mcu::ADC_IRQ_VOLTAGE_OUT, onAdcVoltageOutInterrupt);
	mcu::Adc::instance()->registerInterruptHandler(mcu::ADC_IRQ_CURRENT_IN_FIRST, onAdcCurrentInFirstInterrupt);
	mcu::Adc::instance()->registerInterruptHandler(mcu::ADC_IRQ_CURRENT_IN_SECOND, onAdcCurrentInSecondInterrupt);
	mcu::Adc::instance()->registerInterruptHandler(mcu::ADC_IRQ_TEMP_HEATSINK, onAdcTempHeatsinkInterrupt);

#ifndef CRD300
	RST_PIN.set();
	ERR_PIN.set();
	mcu::delay_us(100000);
#endif
}


///
///
///
void Converter::reset()
{

}


///
///
///
__interrupt void Converter::onPwmEventInterrupt()
{
	LOG_DURATION_VIA_PIN_ONOFF(22);
	Converter::instance()->run();
	Converter::instance()->pwm.acknowledgeEventInterrupt();
}


///
///
///
__interrupt void Converter::onPwmTripInterrupt()
{
	Converter::instance()->pwm.acknowledgeTripInterrupt();
}


///
///
///
__interrupt void Converter::onAdcVoltageInInterrupt()
{
	LOG_DURATION_VIA_PIN_ONOFF(61);
	Converter* converter = Converter::instance();

	float vIn = converter->inVoltageSensor.read();
	converter->m_voltageInFilter.push(vIn);

	if (converter->m_voltageInFilter.output() > converter->m_config.ovpVoltageIn)
	{
		Syslog::setFault(sys::Fault::OVP_IN);
	}
	else if (converter->m_voltageInFilter.output() < converter->m_config.uvpVoltageIn)
	{
		// TODO Syslog::setFault(sys::Fault::UVP_IN);
	}

	mcu::Adc::instance()->acknowledgeInterrupt(mcu::ADC_IRQ_VOLTAGE_IN);
}


///
///
///
__interrupt void Converter::onAdcVoltageOutInterrupt()
{
	LOG_DURATION_VIA_PIN_ONOFF(111);
	Converter* converter = Converter::instance();

	float vOut = converter->outVoltageSensor.read();
	converter->m_voltageOutFilter.push(vOut);

	if (converter->m_voltageOutFilter.output() > converter->m_config.ovpVoltageOut)
	{
		Syslog::setFault(sys::Fault::OVP_OUT);
	}

	if (converter->m_voltageOutFilter.output() > converter->m_config.batteryChargedVoltage)
	{
		Syslog::setWarning(sys::Warning::BATTERY_CHARGED);
		converter->stop();
	}
	else if (converter->m_voltageOutFilter.output() < converter->m_config.batteryChargedVoltage - 10)
	{
		Syslog::resetWarning(sys::Warning::BATTERY_CHARGED);
	}

	mcu::Adc::instance()->acknowledgeInterrupt(mcu::ADC_IRQ_VOLTAGE_OUT);
}


///
///
///
__interrupt void Converter::onAdcCurrentInFirstInterrupt()
{
	LOG_DURATION_VIA_PIN_ONOFF(123);
	Converter* converter = Converter::instance();

#ifdef CRD300
	converter->m_currentIn.first = -1.f * converter->inCurrentSensor.read(InCurrentSensor::FIRST);
#else
	converter->m_currentIn.first = converter->inCurrentSensor.read(InCurrentSensor::FIRST);
#endif

	if (converter->m_currentIn.first > converter->m_config.ocpCurrentIn)
	{
		Syslog::setFault(sys::Fault::OCP_IN);
	}

	mcu::Adc::instance()->acknowledgeInterrupt(mcu::ADC_IRQ_CURRENT_IN_FIRST);
}


///
///
///
__interrupt void Converter::onAdcCurrentInSecondInterrupt()
{
	LOG_DURATION_VIA_PIN_ONOFF(122);
	Converter* converter = Converter::instance();

#ifdef CRD300
	converter->m_currentIn.second = -1.f * converter->inCurrentSensor.read(InCurrentSensor::SECOND);
#else
	converter->m_currentIn.second = converter->inCurrentSensor.read(InCurrentSensor::SECOND);
#endif

	if (converter->m_currentIn.second > converter->m_config.ocpCurrentIn)
	{
		Syslog::setFault(sys::Fault::OCP_IN);
	}

	// calculate average inductor current
	converter->m_currentInFilter.push((converter->m_currentIn.first + converter->m_currentIn.second) / 2);

	if (converter->pwm.state() == mcu::PWM_ON)
	{
		// run current controller to achieve cvVoltageIn
		converter->m_currentController.update(
				converter->m_config.cvVoltageIn,
				converter->m_voltageInFilter.output());

		// run duty cycle controller to achieve needed current
		converter->m_dutycycleController.update(
				converter->m_currentController.output(),
				converter->m_currentInFilter.output());

		converter->pwm.setDutyCycle(converter->m_dutycycleController.output());
	}

	mcu::Adc::instance()->acknowledgeInterrupt(mcu::ADC_IRQ_CURRENT_IN_SECOND);
}


///
///
///
__interrupt void Converter::onAdcTempHeatsinkInterrupt()
{
	Converter::instance()->tempSensor.setReady();
	mcu::Adc::instance()->acknowledgeInterrupt(mcu::ADC_IRQ_TEMP_HEATSINK);
}


///
///
///
void Converter::processTemperatureMeasurements()
{
	if (tempSensor.ready())
	{
		m_tempHeatsinkFilter.push(tempSensor.read());
		tempSensor.resetReady();

		if (m_tempHeatsinkFilter.output() > m_config.otpTempHeatsink)
		{
			Syslog::setFault(sys::Fault::HEATSINK_OVERTEMP);
		}
	}
}


} // namespace fuelcell


