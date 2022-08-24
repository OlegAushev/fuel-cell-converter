/**
 * @defgroup crd600 CRD600
 *
 * @file
 * @ingroup crd600
 */


#pragma once


#ifdef CRD600
#define USE_SD_PINS_FOR_PROFILING


#include "driverlib.h"
#include "device.h"
#include "mcu/system/mcu_system.h"
#include "mcu/gpio/mcu_gpio.h"
#include "emb/emb_common.h"
#include "emb/emb_array.h"
#include <math.h>


namespace crd600 {
/// @addtogroup crd600
/// @{


const size_t PHASE_COUNT = 6;


/// Phase names.
enum Phase
{
	PHASE_U,
	PHASE_V,
	PHASE_W,
	PHASE_X,
	PHASE_Y,
	PHASE_Z
};


/// Leds.
enum Led
{
	LED_RED,
	LED_YELLOW,
	LED_GREEN
};


/**
 * @brief CRD600 Controller board support class.
 */
class Controller : public emb::c28x::Singleton<Controller>
{
private:
	Controller(const Controller& other); 		// no copy constructor
	Controller& operator=(const Controller& other);	// no copy assignment operator

	static const emb::Array<mcu::GpioConfig, 6> PSDIS_PINS_CONFIG;
	static const emb::Array<mcu::GpioConfig, 6> LEN_PINS_CONFIG;
	static const emb::Array<mcu::GpioConfig, 6> OCEN_PINS_CONFIG;
	static const emb::Array<mcu::GpioConfig, 6> FAULT_PINS_CONFIG;
	static const mcu::GpioConfig FAULT_PIN_UVW_CONFIG;
	static const mcu::GpioConfig FAULT_PIN_XYZ_CONFIG;
	static const mcu::GpioConfig FAULT_PIN_ALL_CONFIG;
	static const mcu::GpioConfig SHUTDOWN_POS15V_PIN_CONFIG;
	static const emb::Array<mcu::GpioConfig, 3> LED_PINS_CONFIG;
#ifdef USE_SD_PINS_FOR_PROFILING
	static const mcu::GpioConfig PROFILER_PIN_C1_CONFIG;
	static const mcu::GpioConfig PROFILER_PIN_C2_CONFIG;
	static const mcu::GpioConfig PROFILER_PIN_C3_CONFIG;
	static const mcu::GpioConfig PROFILER_PIN_D1_CONFIG;
	static const mcu::GpioConfig PROFILER_PIN_D2_CONFIG;
	static const mcu::GpioConfig PROFILER_PIN_D3_CONFIG;
#endif

	emb::Array<mcu::GpioOutput, 6> psdisPins;
	emb::Array<mcu::GpioOutput, 6> lenPins;
	emb::Array<mcu::GpioOutput, 6> ocenPins;
	emb::Array<mcu::GpioInput, 6> faultPins;
	mcu::GpioOutput shutdownPos15VPin;
	emb::Array<mcu::GpioOutput, 3> ledPins;

public:
	mcu::GpioInput faultPinUVW;
	mcu::GpioInput faultPinXYZ;
	mcu::GpioInput faultPinALL;
#ifdef USE_SD_PINS_FOR_PROFILING
	mcu::GpioOutput profilerPinC1;
	mcu::GpioOutput profilerPinC2;
	mcu::GpioOutput profilerPinC3;
	mcu::GpioOutput profilerPinD1;
//	mcu::GpioPin profilerPinD2;
	mcu::GpioOutput profilerPinD3;
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
		enableDriverPS(PHASE_U);
		enableDriverPS(PHASE_V);
		enableDriverPS(PHASE_W);
		enableDriverPS(PHASE_X);
		enableDriverPS(PHASE_Y);
		enableDriverPS(PHASE_Z);
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
		disableDriverPS(PHASE_U);
		disableDriverPS(PHASE_V);
		disableDriverPS(PHASE_W);
		disableDriverPS(PHASE_X);
		disableDriverPS(PHASE_Y);
		disableDriverPS(PHASE_Z);
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
		enableDriverLogic(PHASE_U);
		enableDriverLogic(PHASE_V);
		enableDriverLogic(PHASE_W);
		enableDriverLogic(PHASE_X);
		enableDriverLogic(PHASE_Y);
		enableDriverLogic(PHASE_Z);
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
		disableDriverLogic(PHASE_U);
		disableDriverLogic(PHASE_V);
		disableDriverLogic(PHASE_W);
		disableDriverLogic(PHASE_X);
		disableDriverLogic(PHASE_Y);
		disableDriverLogic(PHASE_Z);
	}

	/**
	 * @brief Clears gate driver fault. mcu::delay_us() function is used.
	 * @param (none)
	 * @return (none)
	 */
	void resetAllDrivers();

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
} // namespace crd600


#endif // #ifdef CRD600


