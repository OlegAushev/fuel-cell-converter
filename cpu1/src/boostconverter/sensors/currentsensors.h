/**
 * @defgroup boost_converter_sensors Sensors
 * @ingroup boost_converter
 *
 * @file
 * @ingroup boost_converter boost_converter_sensors
 */


#pragma once


#include "mcu/adc/mcuadc.h"
#include "emb/emb_common.h"


/// @addtogroup boost_converter_sensors
/// @{


/**
 * @brief Input current sensor class.
 */
class InCurrentSensor
{
public:
	enum Measurement
	{
		FIRST,
		SECOND
	};

	mcu::AdcUnit* adcUnit;
private:
	bool m_completed;
	float m_zeroError;
	static const float PHASE_CALIBRATION_THRESHOLD = 50;
	static const size_t CALIBRATION_CYCLES = 1000;

	InCurrentSensor(const InCurrentSensor& other);			// no copy constructor
	InCurrentSensor& operator=(const InCurrentSensor& other);	// no copy assignment operator
public:
	/**
	 * @brief Constructs a new InCurrentSensor object and calibrates current sensor.
	 * @param (none)
	 */
	InCurrentSensor()
		: adcUnit(mcu::AdcUnit::instance())
	{
		m_completed = false;
		m_zeroError = 0;
		calibrate();
	}

	/**
	 * @brief Starts ADC conversion.
	 * @param (none)
	 * @return (none)
	 */
	void convert()
	{
		resetCompleted();
		adcUnit->convertCurrentIn();
	}

	/**
	 * @brief Returns current value.
	 * @param phase - phase
	 * @return Current value.
	 */
	float read(Measurement no) const
	{
		uint16_t rawData;
		switch (no)
		{
		case FIRST:
			rawData = adcUnit->result(mcu::ADC_CURRENT_IN_FIRST);
			break;
		case SECOND:
			rawData = adcUnit->result(mcu::ADC_CURRENT_IN_SECOND);
			break;
		}
#ifdef CRD300
		return 800.f * (float(rawData) / 4095.f) - 400.f - m_zeroError;
#else
		return float(rawData) * 0.102564f - 200 - m_zeroError;
#endif
	}

	/**
	 * @brief Checks if all current measurements are done.
	 * @param (none)
	 * @return \c true if all current measurements are done, \c false otherwise.
	 */
	bool completed() const
	{
		return m_completed;
	}

	/**
	 * @brief Sets completed-flag.
	 * @param phase - phase
	 * @return (none)
	 */
	void setCompleted()
	{
		m_completed = true;
	}

	/**
	 * @brief Resets completed-flags.
	 * @param (none)
	 * @return (none)
	 */
	void resetCompleted()
	{
		m_completed = false;
	}

private:
	void calibrate()
	{
		float sum = 0;

		for (size_t i = 0; i < CALIBRATION_CYCLES; ++i)
		{
			convert();
			while (!adcUnit->interruptPending(mcu::ADC_IRQ_CURRENT_IN_FIRST))
			{  /* WAIT */ }
			sum += read(FIRST);
			adcUnit->clearInterruptStatus(mcu::ADC_IRQ_CURRENT_IN_FIRST);
			mcu::delay_us(10);
		}

		m_zeroError = sum / CALIBRATION_CYCLES;
	}
};



/// @}

