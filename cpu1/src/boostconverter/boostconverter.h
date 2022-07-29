/**
 * @file
 * @ingroup boost_converter
 */


#pragma once


#include "emb/emb_filter.h"
#include "emb/emb_pair.h"
#include "emb/emb_picontroller.h"
#include "mcu/pwm/mcu_pwm.h"
#include "boostconverterdef.h"
#include "fsm/fsm.h"
#include "sensors/currentsensors.h"
#include "sensors/voltagesensors.h"
#include "sensors/temperaturesensors.h"
#include "syslog/syslog.h"

#include "profiler/profiler.h"


/// @addtogroup boost_converter
/// @{


/**
 * @brief Converter config.
 */
struct BoostConverterConfig
{
	float uvpVoltageIn;
	float ovpVoltageIn;
	float ovpVoltageOut;
	float ocpCurrentIn;

	float otpTempJunction;
	float otpTempHeatsink;
	float fanTempThOn;
	float fanTempThOff;

	float kP_dutycycle;
	float kI_dutycucle;
	float kP_current;
	float kI_current;

	float fuelCellVoltageMin;
	float cvVoltageIn;
	float currentInMin;
	float currentInMax;
	float batteryChargedVoltage;

	mcu::Gpio fltPin;
};


/**
 * @brief Converter class.
 */
class BoostConverter : public emb::c28x::Singleton<BoostConverter>
{
private:



	BoostConverterConfig m_config;


	static const float VDC_SMOOTH_FACTOR = 0.001;
	emb::ExponentialMedianFilter<float, 3> m_voltageInFilter;
	emb::ExponentialMedianFilter<float, 3> m_voltageOutFilter;
	emb::Pair<float, float> m_currentIn;	// inductor current measured twice per PWM period
	static const float IDC_SMOOTH_FACTOR = 0.1;
	emb::ExponentialMedianFilter<float, 3> m_currentInFilter;
	static const float TEMP_SMOOTH_FACTOR = 0.05;
	emb::ExponentialMedianFilter<float, 5> m_tempHeatsinkFilter;

	emb::PiControllerCl<emb::CONTROLLER_DIRECT> m_dutycycleController;
	emb::PiControllerCl<emb::CONTROLLER_INVERSE> m_currentController;

	const mcu::Gpio FLT_PIN;
	const mcu::Gpio RST_PIN;
	const mcu::Gpio ERR_PIN;
	const mcu::Gpio REL_PIN;
public:
	mcu::Pwm<mcu::PWM_ONE_PHASE> pwm;
	InVoltageSensor inVoltageSensor;
	OutVoltageSensor outVoltageSensor;
	InCurrentSensor inCurrentSensor;
	TemperatureSensor tempSensor;

private:
	BoostConverter(const BoostConverter& other);		// no copy constructor
	BoostConverter& operator=(const BoostConverter& other);	// no copy assignment operator
public:
	/**
	 * @brief Constructs a new Converter object.
	 * @param converterConfig - converter config
	 * @param pwmConfig - PWM config
	 */
	BoostConverter(const BoostConverterConfig& converterConfig,
			const mcu::PwmConfig<mcu::PWM_ONE_PHASE>& pwmConfig);

	/**
	 * @brief Starts converter.
	 * @param (none)
	 * @return (none)
	 */
	void start()
	{
		if ((Syslog::faults() == 0)
				&& (!Syslog::hasWarning(Warning::BATTERY_CHARGED)
				&& (pwm.state() == mcu::PWM_OFF)))
		{
			pwm.start();
		}
	}

	/**
	 * @brief Stops converter.
	 * @param (none)
	 * @return (none)
	 */
	void stop()
	{
		pwm.stop();
		m_currentController.reset();
		m_dutycycleController.reset();
	}

	/**
	 * @brief Resets controller.
	 * @param (none)
	 * @return (none)
	 */
	void reset();

	float voltageIn() const { return m_voltageInFilter.output(); }
	float voltageOut() const { return m_voltageOutFilter.output(); }
	float currentIn() const { return m_currentInFilter.output(); }
	float tempHeatsink() const { return m_tempHeatsinkFilter.output(); }

	void setCurrentIn(float value)
	{
		if ((value >= m_config.currentInMin) && (value <= m_config.currentInMax))
		{
			m_currentController.setOutputMax(value);
			m_currentController.reset();
		}
	}

	void relOn() const
	{
#ifndef CRD300
		REL_PIN.set();
#endif
	}
	void relOff() const
	{
#ifndef CRD300
		REL_PIN.reset();
#endif
	}

	/**
	 * @brief Checks if temperature measurements are completed and processes them.
	 * @param (none)
	 * @return (none)
	 */
	void processTemperatureMeasurements();

protected:
	static __interrupt void onPwmEventInterrupt();
	static __interrupt void onPwmTripInterrupt();
	static __interrupt void onAdcVoltageInInterrupt();
	static __interrupt void onAdcVoltageOutInterrupt();
	static __interrupt void onAdcCurrentInFirstInterrupt();
	static __interrupt void onAdcCurrentInSecondInterrupt();
	static __interrupt void onAdcTempHeatsinkInterrupt();
};























/// @}

