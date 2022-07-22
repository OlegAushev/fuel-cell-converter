/**
 * @defgroup boost_converter_sensors Sensors
 * @ingroup boost_converter
 *
 * @file
 * @ingroup boost_converter boost_converter_sensors
 */


#pragma once


#include "mcu/adc/mcu_adc.h"
#include "emb/emb_common.h"
#include "emb/emb_array.h"


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

	emb::Array<mcu::AdcChannel, 2> adcChannel;
private:
	bool m_ready;
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
	{
		adcChannel[FIRST].init(mcu::ADC_CURRENT_IN_FIRST);
		adcChannel[SECOND].init(mcu::ADC_CURRENT_IN_SECOND);

		m_ready = false;
		m_zeroError = 0;
		calibrate();
	}

	/**
	 * @brief Starts ADC conversion.
	 * @param no - current measurement number
	 * @return (none)
	 */
	void run(Measurement no) const
	{
		adcChannel[no].start();
	}

	/**
	 * @brief Returns current value.
	 * @param no - current measurement number
	 * @return Current value.
	 */
	float read(Measurement no) const
	{
		uint16_t rawData = adcChannel[no].read();
#ifdef CRD300
		return 800.f * (float(rawData) / 4095.f) - 400.f - m_zeroError;
#else
		return float(rawData) * 0.09524f - 200 - m_zeroError;
#endif
	}

	/**
	 * @brief Checks if all current measurements are done.
	 * @param (none)
	 * @return \c true if all current measurements are done, \c false otherwise.
	 */
	bool ready() const
	{
		return m_ready;
	}

	/**
	 * @brief Sets ready-flag.
	 * @param phase - phase
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

private:
	void calibrate()
	{
		float sum = 0;

		for (size_t i = 0; i < CALIBRATION_CYCLES; ++i)
		{
			run(FIRST);
			while (!mcu::Adc::instance()->interruptPending(mcu::ADC_IRQ_CURRENT_IN_FIRST))
			{  /* WAIT */ }
			sum += read(FIRST);
			mcu::Adc::instance()->clearInterruptStatus(mcu::ADC_IRQ_CURRENT_IN_FIRST);
			mcu::delay_us(10);
		}

		m_zeroError = sum / CALIBRATION_CYCLES;
	}
};



/// @}

