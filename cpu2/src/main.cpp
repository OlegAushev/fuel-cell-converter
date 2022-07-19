///


#include "F28x_Project.h"
#include "device.h"

#include "profiler/profiler.h"

#include "mcu/system/mcusystem.h"
#include "mcu/support/mcusupport.h"
#include "mcu/ipc/mcuipc.h"
#include "mcu/cputimers/mcucputimers.h"
#include "mcoserver/mcoserver.h"
#include "mcu/spi/mcuspi.h"
#include "mcu/dac/mcudac.h"

#include "syslog/syslog.h"
#include "clocktasks/cpu2clocktasks.h"
#include "fuelcellcontroller/fuelcellcontroller.h"


unsigned char converterobj_loc[sizeof(BoostConverter)] __attribute__((section("SHARED_CONVERTER")));
BoostConverter* converter;


/* ========================================================================== */
/* ================================ MAIN ==================================== */
/* ========================================================================== */
/**
 * @brief main()
 * @param None
 * @return None
 */
void main()
{
	/*##########*/
	/*# SYSTEM #*/
	/*##########*/
	mcu::initDevice();
	mcu::setLocalIpcFlag(CPU2_BOOTED);

/*####################################################################################################################*/
	/*##########*/
	/*# SYSLOG #*/
	/*##########*/
	Syslog::IpcFlags syslogIpcFlags =
	{
		.RESET = mcu::IpcFlag(10),
		.ADD_MESSAGE = mcu::IpcFlag(11),
		.POP_MESSAGE = mcu::IpcFlag(12)
	};
	Syslog::init(syslogIpcFlags);

/*####################################################################################################################*/
	/*#########*/
	/*# CLOCK #*/
	/*#########*/
	mcu::SystemClock::init();
	mcu::SystemClock::setTaskPeriod(0, 1000);	// Led toggle period
	mcu::SystemClock::registerTask(0, taskToggleLed);

/*####################################################################################################################*/
	/*#################*/
	/*# POWERUP DELAY #*/
	/*#################*/
	for (size_t i = 0; i < 10; ++i)
	{
		mcu::toggleLed(mcu::LED_RED);
		mcu::delay_us(100000);
	}

/*####################################################################################################################*/
	/*#####################*/
	/*# CONVERTER #*/
	/*#####################*/
	converter = reinterpret_cast<BoostConverter*>(converterobj_loc);

/*####################################################################################################################*/
	/*###############*/
	/*# CAN BY GPIO #*/
	/*###############*/
	mcu::GpioPinConfig canbygpioTxCfg(14, GPIO_14_GPIO14, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1, GPIO_CORE_CPU2);
	mcu::GpioPinConfig canbygpioRxCfg(10, GPIO_10_GPIO10, mcu::PIN_INPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_6SAMPLE, 1, GPIO_CORE_CPU2);
	mcu::GpioPinConfig canbygpioClkCfg(15, GPIO_15_GPIO15, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1, GPIO_CORE_CPU2);
	mcu::GpioPin canbygpioTx(canbygpioTxCfg);
	mcu::GpioPin canbygpioRx(canbygpioRxCfg);
	mcu::GpioPin canbygpioClk(canbygpioClkCfg);
	fuelcell::Controller fcController(converter, canbygpioTx, canbygpioRx, canbygpioClk);

/*####################################################################################################################*/
	/*##################*/
	/*# IPC INTERRUPTS #*/
	/*##################*/
	// NONE

	mcu::setLocalIpcFlag(CPU2_PERIPHERY_CONFIGURED);
	mcu::waitForRemoteIpcFlag(CPU1_PERIPHERY_CONFIGURED);

	// END of CPU2 PERIPHERY CONFIGURATION and OBJECTS CREATION
/*####################################################################################################################*/

	mcu::SystemClock::reset();
	mcu::enableMaskableInterrupts();
	mcu::enableDebugEvents();

	while (true)
	{
		mcu::SystemClock::runTasks();
		fcController.run();
	}
}

















