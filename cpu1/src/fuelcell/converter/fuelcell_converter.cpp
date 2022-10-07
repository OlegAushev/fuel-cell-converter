/**
 * @file
 * @ingroup fuel_cell_converter
 */


#include "fuelcell_converter.h"
#include "../controller/fuelcell_controller.h"


namespace fuelcell {


#ifndef CRD300
const mcu::GpioConfig REL_PIN_CFG(125, GPIO_125_GPIO125, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);

#if HARDWARE_REVISION == 1
const mcu::GpioConfig RST_PIN_CFG(24, GPIO_24_GPIO24, mcu::PIN_OUTPUT, emb::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioConfig ERR_PIN_CFG(16, GPIO_16_GPIO16, mcu::PIN_OUTPUT, emb::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);

#elif HARDWARE_REVISION == 2
const mcu::GpioConfig FLT_PIN_CFG(16, GPIO_16_GPIO16, mcu::PIN_INPUT, emb::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioConfig RDY_PIN_CFG(29, GPIO_29_GPIO29, mcu::PIN_INPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioConfig RST_PIN_CFG(24, GPIO_24_GPIO24, mcu::PIN_OUTPUT, emb::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
#endif

#else
const mcu::GpioConfig FLT_PIN_CFG(15, GPIO_15_GPIO15, mcu::PIN_INPUT, emb::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
#endif


///
///
///
Converter::Converter(const ConverterConfig& converterConfig,
		const mcu::PwmConfig<mcu::PWM_ONE_PHASE>& pwmConfig)
	: emb::c28x::Singleton<Converter>(this)
	, m_state(POWERUP_State::instance())
	, m_stateId(CONVERTER_POWERUP)
	, m_config(converterConfig)
	, m_voltageInFilter(VDC_SMOOTH_FACTOR)
	, m_voltageOutFilter(VDC_SMOOTH_FACTOR)
	, m_tempHeatsinkFilter(TEMP_SMOOTH_FACTOR)
	, m_currentIn(0, 0)
	, m_currentInFilter(IDC_SMOOTH_FACTOR)
	, m_dutycycleController(converterConfig.kP_dutycycle, converterConfig.kI_dutycucle,
			1 / pwmConfig.switchingFreq, 0, 0.7f)
	, m_currentController(converterConfig.kP_current, converterConfig.kI_current,
			1 / pwmConfig.switchingFreq, converterConfig.currentInMin, converterConfig.currentInMax)
#ifndef CRD300
	, REL_PIN(REL_PIN_CFG)
#if HARDWARE_REVISION == 1
	, RST_PIN(RST_PIN_CFG)
	, ERR_PIN(ERR_PIN_CFG)
#elif HARDWARE_REVISION == 2
	, FLT_PIN(FLT_PIN_CFG)
	, RDY_PIN(RDY_PIN_CFG)
	, RST_PIN(RST_PIN_CFG)
#endif
#else
	, FLT_PIN(FLT_PIN_CFG)
#endif
	, pwm(pwmConfig)
{
#if defined(CRD300) || HARDWARE_REVISION == 2
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
#if HARDWARE_REVISION == 1
	RST_PIN.reset();
	ERR_PIN.reset();
	mcu::delay_us(100000);
#elif HARDWARE_REVISION == 2
	RST_PIN.reset();
	mcu::delay_us(100000);
#endif
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
		Syslog::setError(sys::Error::OVP_IN);
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
	if (vOut > converter->m_config.ovpVoltageOut)
	{
		Syslog::setError(sys::Error::OVP_OUT);
	}

	converter->m_voltageOutFilter.push(vOut);
	if (converter->m_voltageOutFilter.output() > converter->m_config.batteryMaxVoltage)
	{
		Syslog::setWarning(sys::Warning::BATTERY_CHARGED);
		converter->shutdown();
	}
	else if (converter->m_voltageOutFilter.output() < converter->m_config.batteryMinVoltage)
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
		Syslog::setError(sys::Error::OCP_IN);
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
		Syslog::setError(sys::Error::OCP_IN);
	}

	// calculate average inductor current
	converter->m_currentInFilter.push((converter->m_currentIn.first + converter->m_currentIn.second) / 2);

	if (converter->pwm.state() == mcu::PWM_ON)
	{
		// OLD ALGO
		// run current controller to achieve cvVoltageIn
		//converter->m_currentController.update(
		//		converter->m_config.cvVoltageIn,
		//		converter->m_voltageInFilter.output());

		converter->m_currentController.update(
				Controller::MIN_OPERATING_VOLTAGE,
				Controller::minCellVoltage());

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
			Syslog::setError(sys::Error::HEATSINK_OVERTEMP);
		}
	}
}


} // namespace fuelcell


