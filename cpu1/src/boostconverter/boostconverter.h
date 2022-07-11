/**
 * @defgroup boost_converter Boost-Converter
 *
 * @file
 * @ingroup boost_converter
 */


#pragma once


#include "emb/emb_filter.h"
#include "emb/emb_pair.h"
#include "emb/emb_picontroller.h"
#include "mcu/pwm/mcupwm.h"
#include "sensors/currentsensors.h"
#include "sensors/voltagesensors.h"
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
	float otpTempCase;
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

	mcu::GpioPin fltPin;
};


/**
 * @brief Converter class.
 */
class BoostConverter : emb::c28x::Singleton<BoostConverter>
{
public:
	/// Converter states
	enum BoostConverterState
	{
		CONVERTER_OFF,
		CONVERTER_ON
	};

private:
	BoostConverterConfig m_config;
	BoostConverterState m_state;

	static const float VDC_SMOOTH_FACTOR = 0.001;
	emb::ExponentialMedianFilter<float, 3> m_voltageIn;
	emb::ExponentialMedianFilter<float, 3> m_voltageOut;
	emb::Pair<float, float> m_currentIn;	// inductor current measured twice per PWM period
	float m_currentInAvg;

	emb::PiControllerCl<emb::CONTROLLER_DIRECT> m_dutycycleController;
	emb::PiControllerCl<emb::CONTROLLER_INVERSE> m_currentController;

	const mcu::GpioPin FLT_PIN;
	const mcu::GpioPin RST_PIN;
	const mcu::GpioPin ERR_PIN;
	const mcu::GpioPin REL_PIN;
public:
	mcu::PwmUnit<mcu::PWM_ONE_PHASE> pwmUnit;
	InVoltageSensor inVoltageSensor;
	OutVoltageSensor outVoltageSensor;
	InCurrentSensor inCurrentSensor;

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
	 * @brief Returns converter state.
	 * @param (none)
	 * @return Converter state.
	 */
	BoostConverterState state() const { return m_state; }

	/**
	 * @brief Starts converter.
	 * @param (none)
	 * @return (none)
	 */
	void start()
	{
		if ((Syslog::faults() == 0)
				&& (!Syslog::hasWarning(Warning::BATTERY_CHARGED)
				&& (m_state == CONVERTER_OFF)))
		{
			m_state = CONVERTER_ON;
			pwmUnit.start();
		}
	}

	/**
	 * @brief Stops converter.
	 * @param (none)
	 * @return (none)
	 */
	void stop()
	{
		pwmUnit.stop();
		m_state = CONVERTER_OFF;
	}

	/**
	 * @brief Resets controller.
	 * @param (none)
	 * @return (none)
	 */
	void reset();

	float voltageIn() const { return m_voltageIn.output(); }
	float voltageOut() const { return m_voltageOut.output(); }
	float currentIn() const { return m_currentInAvg; }
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

protected:
	static __interrupt void onPwmEventInterrupt();
	static __interrupt void onPwmTripInterrupt();
	static __interrupt void onAdcVoltageInInterrupt();
	static __interrupt void onAdcVoltageOutInterrupt();
	static __interrupt void onAdcCurrentInFirstInterrupt();
	static __interrupt void onAdcCurrentInSecondInterrupt();
};























/// @}

