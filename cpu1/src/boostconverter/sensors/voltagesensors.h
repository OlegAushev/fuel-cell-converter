/**
 * @file
 * @ingroup boost_converter boost_converter_sensors
 */


#pragma once


#include "mcu/adc/mcuadc.h"
#include "emb/emb_common.h"


/// @addtogroup boost_converter_sensors
/// @{


/**
 * Input voltage sensor class.
 */
class InVoltageSensor
{
public:
	mcu::AdcUnit* adcUnit;
private:
	bool m_completed;
	InVoltageSensor(const InVoltageSensor& other);			// no copy constructor
	InVoltageSensor& operator=(const InVoltageSensor& other);	// no copy assignment operator
public:
	/**
	 * @brief Constructs a new VoltageInSensor object.
	 * @param (none)
	 */
	InVoltageSensor()
		: adcUnit(mcu::AdcUnit::instance())
	{
		m_completed = false;
	}

	/**
	 * @brief Starts ADC conversion.
	 * @param (none)
	 * @return (none)
	 */
	void convert()
	{
		resetCompleted();
		adcUnit->startVoltageIn();
	}

	/**
	 * @brief Returns DC-voltage value.
	 * @param (none)
	 * @return DC-voltage value.
	 */
	float reading() const
	{
		uint16_t rawData = adcUnit->voltageIn();
#ifdef CRD300
		return 2400.f * (float(rawData) / 4095.f) - 1200.f;
#else
		return 3.f * (float(rawData) / 4095.f) * 108.f;
#endif
	}

	/**
	 * @brief Checks if DC-voltage measurement is done.
	 * @param (none)
	 * @return \c true if DC-voltage measurement is done, \c false otherwise.
	 */
	bool completed() const
	{
		return m_completed;
	}

	/**
	 * @brief Sets completed-flag.
	 * @param (none)
	 * @return (none)
	 */
	void setCompleted()
	{
		m_completed = true;
	}

	/**
	 * @brief Resets completed-flag.
	 * @param (none)
	 * @return (none)
	 */
	void resetCompleted()
	{
		m_completed = false;
	}
};


/**
 * Output voltage sensor class.
 */
class OutVoltageSensor
{
public:
	mcu::AdcUnit* adcUnit;
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
		: adcUnit(mcu::AdcUnit::instance())
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
		adcUnit->startVoltageOut();
	}

	/**
	 * @brief Returns DC-voltage value.
	 * @param (none)
	 * @return DC-voltage value.
	 */
	float read() const
	{
		uint16_t rawData = adcUnit->voltageOut();
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

