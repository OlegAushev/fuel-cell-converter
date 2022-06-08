/**
 * @defgroup boost_converter Boost-Converter
 *
 * @file
 * @ingroup boost_converter
 */


#pragma once


#include "emb/emb_filter.h"
#include "emb/emb_pair.h"
#include "mcu/pwm/mcupwm.h"

#include "sensors/currentsensors.h"
#include "sensors/voltagesensors.h"


/// @addtogroup boost_converter
/// @{


/**
 * @brief Converter config.
 */
struct BoostConverterConfig
{
	float uvpIn;
	float ovpIn;
	float ocpIn;
	float otpJunction;
	float otpCase;
	float fanTempThOn;
	float fanTempThOff;
	mcu::GpioPin fltPin;
};


/**
 * @brief Converter class.
 */
class BoostConverter
{
private:
	const mcu::GpioPin FLT_PIN;
public:
	/// Converter states
	enum BoostConverterState
	{
		CONVERTER_OFF,
		CONVERTER_ON
	};

	const float OVP_IN_BOUND;
	const float UVP_IN_BOUND;
	const float OCP_IN_BOUND;
	const float OTP_MODULE_BOUND;
	const float OTP_CASE_BOUND;
	const float FAN_TEMP_TH_ON;
	const float FAN_TEMP_TH_OFF;

	mcu::PwmUnit<mcu::PWM_ONE_PHASE> pwmUnit;

private:
	BoostConverterState m_state;

	static const float VDC_SMOOTH_FACTOR = 0.1;
	emb::ExponentialFilter<float> m_voltageIn;
	emb::ExponentialFilter<float> m_voltageOut;
	emb::Pair<float, float> m_currentIn;	// inductor current measured twice

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
		m_state = CONVERTER_ON;
		pwmUnit.start();
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

protected:
	static __interrupt void onPwmInterrupt();
	static __interrupt void onPwmTripInterrupt();
	static __interrupt void onAdcVoltageInInterrupt();
	static __interrupt void onAdcVoltageOutInterrupt();
	static __interrupt void onAdcCurrentInInterrupt();

};























/// @}

