/**
 * @file
 * @ingroup fuel_cell_converter
 */


#pragma once


#include "mcu/adc/mcu_adc.h"
#include "emb/emb_common.h"


/// @addtogroup fuel_cell_converter
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
		: adcChannel(mcu::ADC_VOLTAGE_IN)
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
		//return 3.f * (float(rawData) / 4095.f) * 315.f;//275
		return (float(rawData) / 4095.f) * 1500.f - 750.f;
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
		//return 3.f * (float(rawData) / 4095.f) * 250.f;//275
		return (float(rawData) / 4095.f) * 1500.f - 750.f;
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


