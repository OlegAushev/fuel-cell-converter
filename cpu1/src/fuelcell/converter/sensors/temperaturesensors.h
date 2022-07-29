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
 * Temperature sensor class
 */
class TemperatureSensor
{
public:
	mcu::AdcChannel adcChannel;
private:
	bool m_ready;
	TemperatureSensor(const TemperatureSensor& other);		// no copy constructor
	TemperatureSensor& operator=(const TemperatureSensor& other);	// no copy assignment operator
public:
	TemperatureSensor()
		: adcChannel(mcu::ADC_TEMP_HEATSINK)
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
	 * @brief Returns temperature value.
	 * @param (none)
	 * @return Temperature value.
	 */
	float read() const
	{
		uint16_t rawData = adcChannel.read();
#ifdef CRD300
		// return 2400.f * (float(rawData) / 4095.f) - 1200.f;
#else
		return 3.f * (float(rawData) / 4095.f) / 0.01f;	// 10mV = 1C
#endif
	}

	/**
	 * @brief Checks if temperature measurement is done.
	 * @param (none)
	 * @return \c true if temperature measurement is done, \c false otherwise.
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


