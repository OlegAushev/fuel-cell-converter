/**
 * @defgroup mcu_cap CAP
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_cap
 */


#pragma once


#include "driverlib.h"
#include "device.h"

#include "mcu/gpio/mcugpio.h"
#include "emb/emb_common.h"
#include "emb/emb_array.h"

namespace mcu {
/// @addtogroup mcu_cap
/// @{


/// CAP modules
enum CapModule
{
	CAP1,
	CAP2,
	CAP3
};

/**
 * @brief CAP module implementation.
 */
struct CapModuleImpl
{
	const uint32_t base;
	uint32_t inputPin;
	uint32_t inputPinMux;
	const XBAR_InputNum xbarInput;
	const uint32_t pieIntNo;
};

/**
 * @brief CAP unit class.
 */
class CapUnit : public emb::c28x::Singleton<CapUnit>
{
private:
	CapUnit(const CapUnit& other);			// no copy constructor
	CapUnit& operator=(const CapUnit& other);	// no copy assignment operator
public:
	/// CAP modules
	static CapModuleImpl module[3];

	/**
	 * @brief Initializes MCU CAP unit.
	 * @param pins - array of 3 capture pins
	 */
	CapUnit(const emb::Array<mcu::GpioPin, 3>& pins);

	/**
	 * @brief Rearms unit.
	 * @param (none)
	 * @return (none)
	 */
	void rearm() const
	{
		for (int i = 0; i < 3; ++i)
		{
			ECAP_reArm(module[i].base);
		}
	}

	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts() const
	{
		for (int i = 0; i < 3; ++i)
		{
			ECAP_enableInterrupt(module[i].base, ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
			ECAP_enableInterrupt(module[i].base, ECAP_ISR_SOURCE_CAPTURE_EVENT_2);
			ECAP_enableInterrupt(module[i].base, ECAP_ISR_SOURCE_COUNTER_OVERFLOW);
		}
	}

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts() const
	{
		for (int i = 0; i < 3; ++i)
		{
			ECAP_disableInterrupt(module[i].base, ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
			ECAP_disableInterrupt(module[i].base, ECAP_ISR_SOURCE_CAPTURE_EVENT_2);
			ECAP_disableInterrupt(module[i].base, ECAP_ISR_SOURCE_COUNTER_OVERFLOW);
		}
	}

	/**
	 * @brief Registers interrupt handler.
	 * @param iCap - capture module
	 * @param handler - interrupt handler
	 * @return (none)
	 */
	void registerInterruptHandler(CapModule iCap, void (*handler)(void)) const
	{
		Interrupt_register(module[iCap].pieIntNo, handler);
		Interrupt_enable(module[iCap].pieIntNo);
	}
};


/// @}
} // namespace mcu


