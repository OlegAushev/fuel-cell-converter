///
//#define CAN_BY_GPIO


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


#pragma DATA_SECTION("SHARED_CONVERTER")
unsigned char converterobj_loc[sizeof(BoostConverter)];
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
	mcu::sendIpcSignal(CPU2_BOOTED);

/*####################################################################################################################*/
	/*##########*/
	/*# SYSLOG #*/
	/*##########*/
	Syslog::IpcSignals syslogIpcSignals =
	{
		.reset = mcu::IpcSignalPair(10),
		.addMessage = mcu::IpcSignalPair(11),
		.popMessage = mcu::IpcSignalPair(12)
	};
	Syslog::init(syslogIpcSignals);

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


/*####################################################################################################################*/
	/*##################*/
	/*# IPC INTERRUPTS #*/
	/*##################*/
	// NONE

	mcu::sendIpcSignal(CPU2_PERIPHERY_CONFIGURED);
	mcu::waitForIpcSignal(CPU1_PERIPHERY_CONFIGURED);

	// END of CPU2 PERIPHERY CONFIGURATION and OBJECTS CREATION
/*####################################################################################################################*/

	mcu::SystemClock::reset();
	mcu::enableMaskableInterrupts();
	mcu::enableDebugEvents();

	while (true)
	{
		mcu::SystemClock::runTasks();
	}
}

















