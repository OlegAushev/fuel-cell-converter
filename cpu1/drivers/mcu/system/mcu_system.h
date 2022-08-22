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
#include "emb/emb_common.h"


namespace mcu {
/// @addtogroup mcu_system
/// @{


/// Tags for tag dispatching
namespace tag {

struct not_configured {};
struct use_i2ca {};
struct use_i2cb {};

} // namespace tag


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


/**
 * @brief Returns device SYSCLK frequency.
 * @param (none)
 * @return Device SYSCLK frequency.
 */
inline uint32_t sysclkFreq()
{
	return DEVICE_SYSCLK_FREQ;
}


/// @}
} // namespace mcu


