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
#include <assert.h>

#include "emb/emb_interfaces/emb_gpio.h"
#include "../system/mcu_system.h"


namespace mcu {
/// @addtogroup mcu_gpio
/// @{


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


namespace detail {


extern const uint32_t PIE_XINT_NUMBERS[5];
extern const uint16_t PIE_XINT_GROUPS[5];


} // namespace detail


/**
 * @brief GPIO pin config.
 */
struct GpioConfig
{
	bool valid;
	uint32_t no;
	uint32_t mux;
	PinDirection direction;
	emb::PinActiveState activeState;
	PinType type;
	PinQualMode qualMode;
	uint32_t qualPeriod;
	GPIO_CoreSelect masterCore;

	/**
	 * @brief Constructs default GPIO pin config.
	 * @param (none)
	 */
	GpioConfig() : valid(false) {}

	/**
	 * @brief Constructs GPIO pin config.
	 * @param _no - pin number
	 * @param _mux - pin mux
	 * @param _direction - pin direction
	 * @param _activeState - pin active state
	 * @param _type - pin type
	 * @param _qualMode - pin qualification mode
	 * @param _qualPeriod - pin qualification period (divider)
	 * @param _masterCore - master core
	 */
	GpioConfig(uint32_t _no, uint32_t _mux, PinDirection _direction, emb::PinActiveState _activeState,
			PinType _type, PinQualMode _qualMode, uint32_t _qualPeriod,
			GPIO_CoreSelect _masterCore = GPIO_CORE_CPU1)
		: valid(true)
		, no(_no)
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

	/**
	 * @brief Constructs GPIO pin config for muxed pins.
	 * @param _no - pin number
	 * @param _mux - pin mux
	 */
	GpioConfig(uint32_t _no, uint32_t _mux)
		: valid(true)
		, no(_no)
		, mux(_mux)
	{}

	/**
	 * @brief Constructs config for pin which must not be configured.
	 */
	GpioConfig(tag::not_configured) : valid(false) {}
};


/**
 * @brief GPIO pin class.
 */
class Gpio : public emb::IGpio
{
private:
	GpioConfig m_cfg;
	bool m_initialized;

	GPIO_ExternalIntNum m_intNum;
public:
	/**
	 * @brief Gpio pin default constructor.
	 * @param (none)
	 */
	Gpio()
		: m_initialized(false)
	{}

	/**
	 * @brief Constructs GPIO pin.
	 * @param cfg - pin config
	 */
	Gpio(const GpioConfig& cfg)
		: m_initialized(false)
	{
		init(cfg);
	}

	/**
	 * @brief Initializes GPIO pin.
	 * @param cfg - pin config
	 * @return (none)
	 */
	void init(const GpioConfig& cfg)
	{
		m_cfg = cfg;
		if (m_cfg.valid)
		{
#ifdef CPU1
			_init();
#endif
			m_initialized = true;
		}
	}

	/**
	 * @brief Sets the master core.
	 * @param _masterCore - master core
	 * @return (none)
	 */
	void setMasterCore(GPIO_CoreSelect masterCore)
	{
		assert(m_initialized);
		m_cfg.masterCore = masterCore;
#ifdef CPU1
		GPIO_setMasterCore(m_cfg.no, masterCore);
#endif
	}

	/**
	 * @brief Reads pin state.
	 * @param (none)
	 * @return Pin state.
	 */
	virtual emb::PinState read() const
	{
		assert(m_initialized);
		return static_cast<emb::PinState>(1
				- (GPIO_readPin(m_cfg.no) ^ static_cast<uint32_t>(m_cfg.activeState)));
	}

	/**
	 * @brief Sets pin state.
	 * @param state - pin state
	 * @return (none)
	 */
	virtual void set(emb::PinState state = emb::PIN_ACTIVE) const
	{
		assert(m_initialized);
		GPIO_writePin(m_cfg.no, 1
				- (static_cast<uint32_t>(state) ^ static_cast<uint32_t>(m_cfg.activeState)));
	}

	/**
	 * @brief Sets pin state to INACTIVE.
	 * @param (none)
	 * @return (none)
	 */
	virtual void reset() const
	{
		assert(m_initialized);
		set(emb::PIN_INACTIVE);
	}

	/**
	 * @brief Toggles pin state.
	 * @param (none)
	 * @return (none)
	 */
	virtual void toggle() const
	{
		assert(m_initialized);
		GPIO_togglePin(m_cfg.no);
	}

	/**
	 * @brief Returns reference to pin config.
	 * @param (none)
	 * @return Reference to pin config.
	 */
	const GpioConfig& config() const
	{
		return m_cfg;
	}

	/**
	 * @brief Returns pin number.
	 * @param (none)
	 * @return Pin number.
	 */
	uint32_t no() const
	{
		return m_cfg.no;
	}

private:
	/**
	 * @brief Initializes GPIO pin.
	 * @param (none)
	 * @return (none)
	 */
	void _init()
	{
		switch (m_cfg.direction)
		{
		case PIN_OUTPUT:
			GPIO_setPadConfig(m_cfg.no, m_cfg.type);
			//set() - is virtual, shouldn't be called in ctor
			GPIO_writePin(m_cfg.no, 1
					- (static_cast<uint32_t>(emb::PIN_INACTIVE) ^ static_cast<uint32_t>(m_cfg.activeState)));
			GPIO_setPinConfig(m_cfg.mux);
			GPIO_setDirectionMode(m_cfg.no, static_cast<GPIO_Direction>(m_cfg.direction));
			break;

		case PIN_INPUT:
			GPIO_setQualificationPeriod(m_cfg.no, m_cfg.qualPeriod);
			GPIO_setQualificationMode(m_cfg.no, static_cast<GPIO_QualificationMode>(m_cfg.qualMode));
			GPIO_setPadConfig(m_cfg.no, m_cfg.type);
			GPIO_setPinConfig(m_cfg.mux);
			GPIO_setDirectionMode(m_cfg.no, static_cast<GPIO_Direction>(m_cfg.direction));
			break;
		}

		GPIO_setMasterCore(m_cfg.no, m_cfg.masterCore);
	}

public:
	/**
	 * @brief Sets the pin for the specified external interrupt.
	 * @param intNum - interrupt number
	 * @return (none)
	 */
#ifdef CPU1
	void setInterrupt(GPIO_ExternalIntNum intNum)
	{
		GPIO_setInterruptPin(m_cfg.no, intNum);	// X-Bar may be configured on CPU1 only
	}
#endif

	/**
	 * @brief Registers interrupt handler.
	 * @param intNum - interrupt number
	 * @param intType - interrupt type
	 * @param handler - pointer to handler
	 */
	void registerInterruptHandler(GPIO_ExternalIntNum intNum, GPIO_IntType intType, void (*handler)(void))
	{
		m_intNum = intNum;
		GPIO_setInterruptType(intNum, intType);
		Interrupt_register(detail::PIE_XINT_NUMBERS[intNum], handler);
		Interrupt_enable(detail::PIE_XINT_NUMBERS[m_intNum]);
	}

	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts() const
	{
		GPIO_enableInterrupt(m_intNum);

	}

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts() const
	{
		GPIO_disableInterrupt(m_intNum);
	}

	/**
	 * @brief Acknowledges interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeInterrupt() const
	{
		Interrupt_clearACKGroup(detail::PIE_XINT_GROUPS[m_intNum]);
	}
};


/**
 * @brief GPIO pin debouncing class.
 */
class GpioDebouncer
{
private:
	const Gpio m_pin;
public:
	const unsigned int ACQ_PERIOD_MSEC;
	const unsigned int ACTIVE_DEBOUNCE_MSEC;
	const unsigned int INACTIVE_DEBOUNCE_MSEC;
private:
	const unsigned int ACTIVE_DEBOUNCE_COUNT;
	const unsigned int INACTIVE_DEBOUNCE_COUNT;
	unsigned int m_count;
	emb::PinState m_state;
	bool m_stateChanged;
public:
	/**
	 * @brief Debouncer constructor.
	 * @param pin - GPIO pin
	 * @param acqPeriod_msec - acquisition period (msec)
	 * @param act_msec - time(msec) before registering active state
	 * @param inact_msec - time(msec) before registering inactive state
	 */
	GpioDebouncer(const Gpio& pin, unsigned int acqPeriod_msec, unsigned int act_msec, unsigned int inact_msec)
		: m_pin(pin)
		, ACQ_PERIOD_MSEC(acqPeriod_msec)
		, ACTIVE_DEBOUNCE_MSEC(act_msec)
		, INACTIVE_DEBOUNCE_MSEC(inact_msec)
		, ACTIVE_DEBOUNCE_COUNT(act_msec / acqPeriod_msec)
		, INACTIVE_DEBOUNCE_COUNT(inact_msec / acqPeriod_msec)
		, m_state(emb::PIN_INACTIVE)
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
		emb::PinState rawState = m_pin.read();

		if (rawState == m_state)
		{
			if (m_state == emb::PIN_ACTIVE)
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
				if (m_state == emb::PIN_ACTIVE)
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
	emb::PinState state() const { return m_state; };

	/**
	 * @brief Checks if state has changed at last debounce() routine run.
	 * @param (none)
	 * @return \c true if state has changed at last debounce(), \c false otherwise.
	 */
	bool stateChanged() const { return m_stateChanged; };
};


/// @}
} // namespace mcu


