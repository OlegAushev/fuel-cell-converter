/**
 * @defgroup mcu_gpio GPIO
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_gpio
 */


#pragma once


#include "driverlib.h"
#include "device.h"


namespace mcu {
/// @addtogroup mcu_gpio
/// @{


#define PIN_GPIO_CFG_PINMUX_IMPL(pin) GPIO_##pin##_GPIO##pin
#define PIN_GPIO_CFG_PINMUX(pin) PIN_GPIO_CFG_PINMUX_IMPL(pin)

/// Pin active states
enum PinActiveState
{
	ACTIVE_LOW = 0,
	ACTIVE_HIGH = 1
};

/// Pin states
enum PinState
{
	PIN_INACTIVE = 0,
	PIN_ACTIVE = 1
};

/// Pin types
enum PinType
{
	PIN_STD = GPIO_PIN_TYPE_STD,
	PIN_PULLUP = GPIO_PIN_TYPE_PULLUP,
	PIN_INVERT = GPIO_PIN_TYPE_INVERT,
	PIN_OPENDRAIN = GPIO_PIN_TYPE_OD
};

/// Pin directions
enum PinDirection
{
	PIN_INPUT = GPIO_DIR_MODE_IN,
	PIN_OUTPUT = GPIO_DIR_MODE_OUT
};

/// Pin qualification modes
enum PinQualMode
{
	PIN_QUAL_SYNC = GPIO_QUAL_SYNC,
	PIN_QUAL_3SAMPLE = GPIO_QUAL_3SAMPLE,
	PIN_QUAL_6SAMPLE = GPIO_QUAL_6SAMPLE,
	PIN_QUAL_ASYNC = GPIO_QUAL_ASYNC
};



/**
 * @brief GPIO pin class.
 */
struct GpioPin
{
	uint32_t no;
	uint32_t mux;
	PinDirection direction;
	PinActiveState activeState;
	PinType type;
	PinQualMode qualMode;
	uint32_t qualPeriod;
	GPIO_CoreSelect masterCore;

	/**
	 * @brief Constructs default GPIO pin.
	 * @param (none)
	 */
	GpioPin() {}

	/**
	 * @brief Constructs GPIO pin.
	 * @param _no - pin number
	 * @param _mux - pin mux
	 * @param _direction - pin direction
	 * @param _activeState - pin active state
	 * @param _type - pin type
	 * @param _qualMode - pin qualification mode
	 * @param _qualPeriod - pin qualification period (divider)
	 */
	GpioPin(uint32_t _no, uint32_t _mux, PinDirection _direction, PinActiveState _activeState,
			PinType _type, PinQualMode _qualMode, uint32_t _qualPeriod,
			GPIO_CoreSelect _masterCore = GPIO_CORE_CPU1)
		: no(_no)
		, mux(_mux)
		, direction(_direction)
		, activeState(_activeState)
		, type(_type)
		, qualMode(_qualMode)
		, masterCore(_masterCore)
	{
		if (_qualPeriod >=1 && _qualPeriod <= 510)
		{
			qualPeriod = _qualPeriod;
		}
		else
		{
			while (true) {}
		}
	}
};

/**
 * @brief Initializes GPIO pin.
 * @param pin - GPIO pin structure
 * @return (none)
 */
void initGpioPin(const GpioPin& pin);

/**
 * @brief Sets the master core of a specified pin.
 * @param pin - GPIO pin structure
 * @return (none)
 */
void setGpioPinMasterCore(const GpioPin& pin, GPIO_CoreSelect masterCore);

/**
 * @brief Reads pin state.
 * @param pin - pin
 * @return Pin state.
 */
inline PinState readPinState(const GpioPin& pin)
{
	return static_cast<PinState>(1
			- (GPIO_readPin(pin.no) ^ static_cast<uint32_t>(pin.activeState)));
}

/**
 * @brief Sets pin state.
 * @param pin - pin
 * @return (none)
 */
inline void setPinState(const GpioPin& pin, PinState state)
{
	GPIO_writePin(pin.no, 1 - (static_cast<uint32_t>(state) ^ static_cast<uint32_t>(pin.activeState)));
}

/**
 * @brief Toggles pin state.
 * @param pin - pin
 * @return (none)
 */
inline void togglePinState(const GpioPin& pin)
{
	GPIO_togglePin(pin.no);
}

/**
 * @brief Pin debouncing class.
 */
class PinDebouncer
{
public:
	/**
	 * @brief Debouncer constructor.
	 * @param pin - GPIO pin struct
	 * @param acqPeriod - acquisition period (msec)
	 * @param actMsec - time(msec) before registering active state
	 * @param inactMsec - time(msec) before registering inactive state
	 */
	PinDebouncer(const GpioPin& pin, unsigned int acqPeriod, unsigned int actMsec, unsigned int inactMsec)
		: m_pin(pin)
		, ACQ_PERIOD_MSEC(acqPeriod)
		, ACTIVE_DEBOUNCE_MSEC(actMsec)
		, INACTIVE_DEBOUNCE_MSEC(inactMsec)
		, ACTIVE_DEBOUNCE_COUNT(actMsec / acqPeriod)
		, INACTIVE_DEBOUNCE_COUNT(inactMsec / acqPeriod)
		, m_state(PIN_INACTIVE)
		, m_stateChanged(false)
	{
		m_count = ACTIVE_DEBOUNCE_COUNT;
	}

	/**
	 * @brief Service routine called every ACQ_PERIOD_MSEC to debounce both edges.
	 * @param (none)
	 * @return (none)
	 */
	void debounce()
	{
		m_stateChanged = false;
		PinState rawState = readPinState(m_pin);

		if (rawState == m_state)
		{
			if (m_state == PIN_ACTIVE)
			{
				m_count = INACTIVE_DEBOUNCE_COUNT;
			}
			else
			{
				m_count = ACTIVE_DEBOUNCE_COUNT;
			}
		}
		else
		{
			if (--m_count == 0)
			{
				m_state = rawState;
				m_stateChanged = true;
				if (m_state == PIN_ACTIVE)
				{
					m_count = INACTIVE_DEBOUNCE_COUNT;
				}
				else
				{
					m_count = ACTIVE_DEBOUNCE_COUNT;
				}
			}
		}
	}

	/**
	 * @brief Returns debounced state of pin.
	 * @param (none)
	 * @return Debounced state of pin.
	 */
	PinState state() const { return m_state; };

	/**
	 * @brief Checks if state has changed at last debounce() routine run.
	 * @param (none)
	 * @return \c true if state has changed at last debounce(), \c false otherwise.
	 */
	bool stateChanged() const { return m_stateChanged; };

private:
	const GpioPin m_pin;
public:
	const unsigned int ACQ_PERIOD_MSEC;
	const unsigned int ACTIVE_DEBOUNCE_MSEC;
	const unsigned int INACTIVE_DEBOUNCE_MSEC;
private:
	const unsigned int ACTIVE_DEBOUNCE_COUNT;
	const unsigned int INACTIVE_DEBOUNCE_COUNT;
	unsigned int m_count;
	PinState m_state;
	bool m_stateChanged;
};


/// @}
} // namespace mcu



