/**
 * @defgroup mcu_support Support
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_support
 */


#pragma once


#include "../gpio/mcu_gpio.h"


namespace mcu {
/// @addtogroup mcu_system
/// @{


#if defined(_LAUNCHXL_F28379D)
/// LaunchPad LEDs
enum LaunchPadLed
{
	LED_BLUE = 31,
	LED_RED = 34
};


/**
 * @brief Configures LaunchPad LEDs.
 * @param blueLedCore - blue LED controlling CPU
 * @param redLedCore - red LED controlling CPU
 * @return (none)
 */
inline void configureLaunchPadLeds(GPIO_CoreSelect blueLedCore, GPIO_CoreSelect redLedCore)
{
	GpioConfig ledBlueCfg(31, GPIO_31_GPIO31, mcu::PIN_OUTPUT, emb::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);
	GpioConfig ledRedCfg(34, GPIO_34_GPIO34, mcu::PIN_OUTPUT, emb::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);

	Gpio ledBlue(ledBlueCfg);
	ledBlue.setMasterCore(blueLedCore);

	Gpio ledRed(ledRedCfg);
	ledRed.setMasterCore(redLedCore);
}


/**
 * @brief Turns specified LaunchPad LED on.
 * @param led - LaunchPad LED
 * @return (none)
 */
inline void turnLedOn(LaunchPadLed led) { GPIO_writePin(led, 0); }


/**
 * @brief Turns specified LaunchPad LED off.
 * @param led - LaunchPad LED
 * @return (none)
 */
inline void turnLedOff(LaunchPadLed led) { GPIO_writePin(led, 1); }


/**
 * @brief Toggles specified LaunchPad LED.
 * @param led - LaunchPad LED
 * @return (none)
 */
inline void toggleLed(LaunchPadLed led) { GPIO_togglePin(led); }


#elif defined(CONTROLCARD)


enum ControlCardLed
{
	LED_RED = 34
};


/**
 * @brief Configures controlCARD LEDs.
 * @param ledRedCore - LED RED1 controlling CPU
 * @return (none)
 */
inline void configureControlCardLeds(GPIO_CoreSelect ledRedCore)
{
	GpioConfig ledRedCfg(34, GPIO_34_GPIO34, mcu::PIN_OUTPUT, emb::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);
	Gpio ledRed(ledRedCfg);
	ledRed.setMasterCore(ledRedCore);
}


/**
 * @brief Turns specified LaunchPad LED on.
 * @param led - LaunchPad LED
 * @return (none)
 */
inline void turnLedOn(ControlCardLed led) { GPIO_writePin(led, 0); }


/**
 * @brief Turns specified LaunchPad LED off.
 * @param led - LaunchPad LED
 * @return (none)
 */
inline void turnLedOff(ControlCardLed led) { GPIO_writePin(led, 1); }


/**
 * @brief Toggles specified LaunchPad LED.
 * @param led - LaunchPad LED
 * @return (none)
 */
inline void toggleLed(ControlCardLed led) { GPIO_togglePin(led); }


#endif
//******************************************************************************


/// @}
} // namespace mcu


