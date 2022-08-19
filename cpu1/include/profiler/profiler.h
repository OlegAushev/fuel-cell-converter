///
#pragma once


#include "driverlib.h"
#include "device.h"
#include "emb/emb_common.h"
#include "mcu_c28x/gpio/mcu_gpio.h"


//#define PIN_PROFILER_ENABLED


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
	explicit DurationLoggerPin(const mcu::GpioOutput& pin)
	: m_pin(pin.config().no)
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

	explicit DurationLoggerPin(uint32_t pinNo)
		: m_pin(pinNo)
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


#ifdef CPU1
/**
 * @brief
 * @param pin
 * @param masterCore
 */
inline void initDurationLoggerPin(mcu::GpioOutput& pin, GPIO_CoreSelect masterCore)
{
	pin.setMasterCore(masterCore);
}
#endif


#ifdef PIN_PROFILER_ENABLED
#define LOG_DURATION_VIA_PIN_ONOFF(pin) \
		volatile DurationLoggerPin<LOGGER_ON_OFF> EMB_UNIQ_ID(__LINE__)(pin);


#define LOG_DURATION_VIA_PIN_TOGGLE(pin) \
		volatile DurationLoggerPin<LOGGER_TOGGLE> EMB_UNIQ_ID(__LINE__)(pin);
#else
#define LOG_DURATION_VIA_PIN_ONOFF(pin) ((void)0)
#define LOG_DURATION_VIA_PIN_TOGGLE(pin) ((void)0)
#endif


