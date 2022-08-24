/**
 * @defgroup crd300 CRD300
 *
 * @file
 * @ingroup crd300
 */


#pragma once


#ifdef CRD300
#define CRD300_LEDS_NOT_AVAILABLE


#include "driverlib.h"
#include "device.h"
#include "mcu/system/mcu_system.h"
#include "mcu/gpio/mcu_gpio.h"
#include "emb/emb_common.h"
#include "emb/emb_array.h"
#include <math.h>


namespace crd300 {
/// @addtogroup crd300
/// @{


const size_t PHASE_COUNT = 3;


/// Phase names.
enum Phase
{
	PHASE_A,
	PHASE_B,
	PHASE_C
};


/// Leds.
enum Led
{
	LED_RED,
	LED_YELLOW,
	LED_GREEN,
	LED_CASE
};


/**
 * @brief CRD300 Controller board support class.
 */
class Controller : public emb::c28x::Singleton<Controller>
{
private:
	Controller(const Controller& other); 		// no copy constructor
	Controller& operator=(const Controller& other);	// no copy assignment operator

	static const emb::Array<mcu::GpioConfig, 3> PSDIS_PINS_CONFIG;
	static const emb::Array<mcu::GpioConfig, 3> LEN_PINS_CONFIG;
	static const emb::Array<mcu::GpioConfig, 3> OCEN_PINS_CONFIG;
	static const emb::Array<mcu::GpioConfig, 3> FAULT_PINS_CONFIG;
	static const mcu::GpioConfig SHUTDOWN_POS15V_PIN_CONFIG;
	static const mcu::GpioConfig SHUTDOWN_NEG15V_PIN_CONFIG;
	static const emb::Array<mcu::GpioConfig, 4> LED_PINS_CONFIG;

	static const mcu::GpioConfig PROFILER_PIN_P22_CONFIG;
	static const mcu::GpioConfig PROFILER_PIN_P111_CONFIG;
	static const mcu::GpioConfig PROFILER_PIN_P56_CONFIG;

	emb::Array<mcu::GpioOutput, 3> psdisPins;
	emb::Array<mcu::GpioOutput, 3> lenPins;
	emb::Array<mcu::GpioOutput, 3> ocenPins;
	emb::Array<mcu::GpioInput, 3> faultPins;
	mcu::GpioOutput shutdownPos15VPin;
	mcu::GpioOutput shutdownNeg15VPin;
	emb::Array<mcu::GpioOutput, 4> ledPins;

public:

#ifdef USE_LAUNCHXL_PINS_FOR_PROFILING
	mcu::GpioPin profilerPinP22;
	mcu::GpioPin profilerPinP111;
	mcu::GpioPin profilerPinP56;
#endif

	/**
	 * @brief Initializes controller board support.
	 * @param (none)
	 */
	Controller();

	/**
	 * @brief Enables driver power supply for a specified phase.
	 * @param phase - phase
	 * @return (none)
	 */
	void enableDriverPS(Phase phase) { psdisPins[phase].set(emb::PIN_INACTIVE); }

	/**
	 * @brief Enables driver power supply for all phases.
	 * @param (none)
	 * @return (none)
	 */
	void enableDriverPS()
	{
		enableDriverPS(PHASE_A);
		enableDriverPS(PHASE_B);
		enableDriverPS(PHASE_C);
	}

	/**
	 * @brief Disables driver power supply for a specified phase.
	 * @param phase - phase
	 * @return (none)
	 */
	void disableDriverPS(Phase phase) { psdisPins[phase].set(emb::PIN_ACTIVE); }

	/**
	 * @brief  Disables driver power supply for all phases.
	 * @param (none)
	 * @return (none)
	 */
	void disableDriverPS()
	{
		disableDriverPS(PHASE_A);
		disableDriverPS(PHASE_B);
		disableDriverPS(PHASE_C);
	}

	/**
	 * @brief Enables driver logic for a specified phase.
	 * @param phase - phase
	 * @return (none)
	 */
	void enableDriverLogic(Phase phase) { lenPins[phase].set(emb::PIN_ACTIVE); }

	/**
	 * @brief Enables driver logic for all phases.
	 * @param (none)
	 * @return (none)
	 */
	void enableDriverLogic()
	{
		enableDriverLogic(PHASE_A);
		enableDriverLogic(PHASE_B);
		enableDriverLogic(PHASE_C);
	}

	/**
	 * @brief Disables driver logic for a specified phase.
	 * @param phase - phase
	 * @return (none)
	 */
	void disableDriverLogic(Phase phase) { lenPins[phase].set(emb::PIN_INACTIVE); }

	/**
	 * @brief Disables driver logic for all phases.
	 * @param (none)
	 * @return (none)
	 */
	void disableDriverLogic()
	{
		disableDriverLogic(PHASE_A);
		disableDriverLogic(PHASE_B);
		disableDriverLogic(PHASE_C);
	}

	/**
	 * @brief Clears gate driver fault. mcu::delay_us() function is used.
	 * @param (none)
	 * @return (none)
	 */
	void resetDrivers();

	/**
	 * @brief Checks if driver of specified phase is in fault.
	 * @param phase - phase
	 * @return \c true if driver is in fault, \c false otherwise.
	 */
	bool hasDriverFault(Phase phase)
	{
		if (GPIO_readPin(faultPins[phase].no()) == 0)
		{
			return true;
		}
		return false;
	}

	/**
	 * @brief Enables +15V power supply.
	 * @param (none)
	 * @return (none)
	 */
	void enablePos15V() { shutdownPos15VPin.set(emb::PIN_INACTIVE); }

	/**
	 * @brief Disables +15V power supply.
	 * @param (none)
	 * @return (none)
	 */
	void disablePos15V() { shutdownPos15VPin.set(emb::PIN_ACTIVE); }

	/**
	 * @brief Enables -15V power supply.
	 * @param (none)
	 * @return (none)
	 */
	void enableNeg15V() { shutdownNeg15VPin.set(emb::PIN_INACTIVE); }

	/**
	 * @brief Disables -15V power supply.
	 * @param (none)
	 * @return (none)
	 */
	void disableNeg15V() { shutdownNeg15VPin.set(emb::PIN_ACTIVE); }

	/**
	 * @brief Turns specified LED on.
	 * @param led - LED
	 * @return (none)
	 */
	void turnLedOn(Led led) { ledPins[led].set(emb::PIN_ACTIVE); }

	/**
	 * @brief Turns specified LED off.
	 * @param led - LED
	 * @return (none)
	 */
	void turnLedOff(Led led) { ledPins[led].set(emb::PIN_INACTIVE); }

	/**
	 * @brief Toggles specified LED.
	 * @param led - LED
	 * @return (none)
	 */
	void toggleLed(Led led) { ledPins[led].toggle(); }
};


/// @}
} // namespace crd300


#endif // #ifdef CRD300


