///
#define CAN_BY_GPIO

#ifdef CAN_BY_GPIO
#warning "CAN_BY_GPIO test build."
#endif


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


#pragma DATA_SECTION("SHARED_CONVERTER")
unsigned char converterobj_loc[sizeof(BoostConverter)];
BoostConverter* converter;


uint16_t testRxData[8] = {0};
unsigned int frameId;


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
	Syslog::IpcSignals syslogIpcSignals =
	{
		.reset = mcu::IpcFlagPair(10),
		.addMessage = mcu::IpcFlagPair(11),
		.popMessage = mcu::IpcFlagPair(12)
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
#ifdef CAN_BY_GPIO
	mcu::GpioPinConfig canbygpioTxCfg(14, GPIO_14_GPIO14, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1, GPIO_CORE_CPU2);
	mcu::GpioPinConfig canbygpioRxCfg(10, GPIO_10_GPIO10, mcu::PIN_INPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_6SAMPLE, 1, GPIO_CORE_CPU2);
	mcu::GpioPinConfig canbygpioClkCfg(15, GPIO_15_GPIO15, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1, GPIO_CORE_CPU2);
	mcu::GpioPin canbygpioTx(canbygpioTxCfg);
	mcu::GpioPin canbygpioRx(canbygpioRxCfg);
	mcu::GpioPin canbygpioClk(canbygpioClkCfg);

	canbygpio::Transceiver cbgTransceiver(canbygpioTx, canbygpioRx, canbygpioClk, 125000,
			canbygpio::tag::enable_bit_stuffing());
#endif
	FuelCellController fuelcellController(converter, canbygpioTx, canbygpioRx, canbygpioClk, 125000);

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

#ifdef CAN_BY_GPIO
		int nBytes = cbgTransceiver.recv(frameId, testRxData);
		if (nBytes < 0)
		{
			for (size_t i = 0; i < 10; ++i)
			{
				mcu::toggleLed(mcu::LED_RED);
				mcu::delay_us(50000);
			}
		}
		else if (nBytes == 8)
		{
			if (cbgTransceiver.send(frameId, testRxData, nBytes) == 8)
			{
				mcu::delay_us(10000);
			}
		}
#endif
	}
}

















