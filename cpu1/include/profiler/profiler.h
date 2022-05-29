///
#pragma once


#include "driverlib.h"
#include "device.h"
#include "emb/emb_common.h"
#include "mcu/gpio/mcugpio.h"


enum LoggerPinMode
{
	LOGGER_ON_OFF,
	LOGGER_TOGGLE
};

/**
 * @brief
 */
template <LoggerPinMode Mode>
class DurationLoggerPin
{
private:
	const uint32_t m_pin;
public:
	explicit DurationLoggerPin(const mcu::GpioPin& pin)
	: m_pin(pin.no)
	{
		switch (Mode)
		{
		case LOGGER_ON_OFF:
			GPIO_writePin(m_pin, 1);
			break;
		case LOGGER_TOGGLE:
			GPIO_togglePin(m_pin);
			NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
			GPIO_togglePin(m_pin);
			break;
		}

	}

	~DurationLoggerPin()
	{
		switch (Mode)
		{
		case LOGGER_ON_OFF:
			GPIO_writePin(m_pin, 0);
			break;
		case LOGGER_TOGGLE:
			GPIO_togglePin(m_pin);
			break;
		}
	}
};

/**
 * @brief
 * @param pin
 * @param masterCore
 */
inline void initDurationLoggerPin(const mcu::GpioPin& pin, GPIO_CoreSelect masterCore)
{
	mcu::initGpioPin(pin);
	mcu::setGpioPinMasterCore(pin, masterCore);
}


#define LOG_DURATION_VIA_PIN_ONOFF(pin) \
		volatile DurationLoggerPin<LOGGER_ON_OFF> EMB_UNIQ_ID(__LINE__)(pin);

#define LOG_DURATION_VIA_PIN_TOGGLE(pin) \
		volatile DurationLoggerPin<LOGGER_TOGGLE> EMB_UNIQ_ID(__LINE__)(pin);



