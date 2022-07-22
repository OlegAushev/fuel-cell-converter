/**
 * @file
 * @ingroup boost_converter boost_converter_sensors
 */


#pragma once


#include "mcu/adc/mcu_adc.h"
#include "emb/emb_common.h"


/// @addtogroup boost_converter_sensors
/// @{


/**
 * Input voltage sensor class.
 */
class InVoltageSensor
{
public:
	mcu::AdcChannel adcChannel;
private:
	bool m_ready;
	InVoltageSensor(const InVoltageSensor& other);			// no copy constructor
	InVoltageSensor& operator=(const InVoltageSensor& other);	// no copy assignment operator
public:
	/**
	 * @brief Constructs a new VoltageInSensor object.
	 * @param (none)
	 */
	InVoltageSensor()
		: adcChannel(mcu::ADC_CURRENT_IN_FIRST)
	{
		m_ready = false;
	}

	/**
	 * @brief Starts ADC conversion.
	 * @param (none)
	 * @return (none)
	 */
	void run()
	{
		adcChannel.start();
	}

	/**
	 * @brief Returns DC-voltage value.
	 * @param (none)
	 * @return DC-voltage value.
	 */
	float read() const
	{
		uint16_t rawData = adcChannel.read();
#ifdef CRD300
		return 2400.f * (float(rawData) / 4095.f) - 1200.f;
#else
		return 3.f * (float(rawData) / 4095.f) * 206.f;
#endif
	}

	/**
	 * @brief Checks if DC-voltage measurement is done.
	 * @param (none)
	 * @return \c true if DC-voltage measurement is done, \c false otherwise.
	 */
	bool ready() const
	{
		return m_ready;
	}

	/**
	 * @brief Sets ready-flag.
	 * @param (none)
	 * @return (none)
	 */
	void setReady()
	{
		m_ready = true;
	}

	/**
	 * @brief Resets ready-flag.
	 * @param (none)
	 * @return (none)
	 */
	void resetReady()
	{
		m_ready = false;
	}
};


/**
 * Output voltage sensor class.
 */
class OutVoltageSensor
{
public:
	mcu::AdcChannel adcChannel;
private:
	bool m_ready;
	OutVoltageSensor(const OutVoltageSensor& other);			// no copy constructor
	OutVoltageSensor& operator=(const OutVoltageSensor& other);	// no copy assignment operator
public:
	/**
	 * @brief Constructs a new VoltageInSensor object.
	 * @param (none)
	 */
	OutVoltageSensor()
		: adcChannel(mcu::ADC_VOLTAGE_OUT)
	{
		m_ready = false;
	}

	/**
	 * @brief Starts ADC conversion.
	 * @param (none)
	 * @return (none)
	 */
	void run() const
	{
		adcChannel.start();
	}

	/**
	 * @brief Returns DC-voltage value.
	 * @param (none)
	 * @return DC-voltage value.
	 */
	float read() const
	{
		uint16_t rawData = adcChannel.read();
#ifdef CRD300
		return 1200.f * (float(rawData) / 4095.f);
#else
		return 3.f * (float(rawData) / 4095.f) * 250.f;
#endif
	}

	/**
	 * @brief Checks if DC-voltage measurement is done.
	 * @param (none)
	 * @return \c true if DC-voltage measurement is done, \c false otherwise.
	 */
	bool ready() const
	{
		return m_ready;
	}

	/**
	 * @brief Sets ready-flag.
	 * @param (none)
	 * @return (none)
	 */
	void setReady()
	{
		m_ready = true;
	}

	/**
	 * @brief Resets ready-flag.
	 * @param (none)
	 * @return (none)
	 */
	void resetReady()
	{
		m_ready = false;
	}
};





/// @}

