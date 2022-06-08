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
	}

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

};























/// @}

