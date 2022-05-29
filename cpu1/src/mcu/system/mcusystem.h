/**
 * @defgroup mcu MCU
 *
 * @defgroup mcu_system System
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_system
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "../gpio/mcugpio.h"
#include "emb/emb_common.h"


namespace mcu {
/// @addtogroup mcu_system
/// @{

/**
 * @brief Initializes MCU device.
 * @return (none)
 */
inline void initDevice()
{
#ifdef CPU1
	Device_init();			// Initialize device clock and peripherals
	Device_initGPIO();		// Disable pin locks and enable internal pull-ups
	Interrupt_initModule();		// Initialize PIE and clear PIE registers. Disable CPU interrupts
	Interrupt_initVectorTable();	// Initialize the PIE vector table with pointers to the shell Interrupt Service Routines (ISR)
#endif
#ifdef CPU2
	Device_init();
	Interrupt_initModule();
	Interrupt_initVectorTable();
#endif
}

/**
 * @brief Delays for a number of nanoseconds.
 * @param nsDelay - delay in nanoseconds
 * @return (none)
 */
inline void delay_ns(uint32_t nsDelay)
{
	const uint32_t CYCLE_NS = 1000000000 / DEVICE_SYSCLK_FREQ;
	const uint32_t LOOP_NS = 5 * CYCLE_NS;
	const uint32_t OVERHEAD_NS = 9 * CYCLE_NS;

	if (nsDelay < LOOP_NS + OVERHEAD_NS)
	{
		SysCtl_delay(1);
	}
	else
	{
		SysCtl_delay((nsDelay - OVERHEAD_NS) / LOOP_NS);
	}
}

/**
 * @brief Delays for a number of microseconds.
 * @param usDelay - delay in microseconds
 * @return (none)
 */
inline void delay_us(uint32_t usDelay)
{
	DEVICE_DELAY_US(usDelay);
}


//******************************************************************************
#ifdef CPU1
/**
 * @brief Starts CPU2 boot process.
 * @return (none)
 */
inline void bootCpu2() { Device_bootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH); }
#endif
//******************************************************************************

/**
 * @brief Enables maskable interrupts.
 * @return (none)
 */
inline void enableMaskableInterrupts() { EINT; }

/**
 * @brief Disables maskable interrupts.
 * @return (none)
 */
inline void disableMaskableInterrupts() { DINT; }

/**
 * @brief Enables debug events.
 * @return (none)
 */
inline void enableDebugEvents() { ERTM; }

/**
 * @brief Disables debug events.
 * @return (none)
 */
inline void disableDebugEvents() { DRTM; }

/**
 * @brief Resets device.
 * @return (none)
 */
inline void resetDevice() { SysCtl_resetDevice(); }

/**
 * @brief Critical Section class.
 */
class CriticalSection
{
public:
	CriticalSection() { DINT; }	// disable maskable interrupts
	~CriticalSection() { EINT; }	// enable maskable interrupts
	static void enter() { DINT; }
	static void leave() { EINT; }
};
#define CRITICAL_SECTION CriticalSection EMB_UNIQ_ID(__LINE__);


//******************************************************************************
#if defined(_LAUNCHXL_F28379D)
/// LaunchPad LEDs.
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
	GpioPin ledBlue(31, GPIO_31_GPIO31, mcu::PIN_OUTPUT, mcu::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);
	GpioPin ledRed(34, GPIO_34_GPIO34, mcu::PIN_OUTPUT, mcu::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);

	initGpioPin(ledBlue);
	GPIO_setMasterCore(ledBlue.no, blueLedCore);

	initGpioPin(ledRed);
	GPIO_setMasterCore(ledRed.no, redLedCore);
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
	LED_RED = 34/**< LED_RED */
};

/**
 * @brief Configures controlCARD LEDs.
 * @param ledRedCore - LED RED1 controlling CPU
 * @return (none)
 */
inline void configureControlCardLeds(GPIO_CoreSelect ledRedCore)
{
	GpioPin ledRed(34, GPIO_34_GPIO34, mcu::PIN_OUTPUT, mcu::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);

	initGpioPin(ledRed);
	GPIO_setMasterCore(ledRed.no, ledRedCore);
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




