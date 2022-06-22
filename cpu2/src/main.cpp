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
#include "canbygpio/canbygpio.h"


#pragma DATA_SECTION("SHARED_CONVERTER")
unsigned char converterobj_loc[sizeof(BoostConverter)];
BoostConverter* converter;


uint16_t testTxData[8] = {0xD,0xE,0xA,0xD,0xB,0xE,0xE,0xF};
uint16_t testRxData[8] = {0};
int dataLen, frameId;


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
#ifdef CAN_BY_GPIO
	mcu::GpioPinConfig canbygpioTxCfg(14, GPIO_14_GPIO14, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1, GPIO_CORE_CPU2);
	mcu::GpioPinConfig canbygpioRxCfg(10, GPIO_10_GPIO10, mcu::PIN_INPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_6SAMPLE, 1, GPIO_CORE_CPU2);
	mcu::GpioPinConfig canbygpioClkCfg(15, GPIO_15_GPIO15, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1, GPIO_CORE_CPU2);
	mcu::GpioPin canbygpioTx(canbygpioTxCfg);
	mcu::GpioPin canbygpioRx(canbygpioRxCfg);
	mcu::GpioPin canbygpioClk(canbygpioClkCfg);

	canbygpio::Transceiver cbgTranceiver(canbygpioTx, canbygpioRx, canbygpioClk, 125000);
#endif

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

#ifdef CAN_BY_GPIO
		uint64_t testDataRaw;
		emb::c28x::from_8bit_bytes(testDataRaw, testTxData);

		if (cbgTranceiver.send<uint64_t>(0x200, testDataRaw) == 8)
		{
			mcu::delay_us(10000);
		}

		if (cbgTranceiver.recv(frameId, testRxData) != 8)
		{
			for (size_t i = 0; i < 10; ++i)
			{
				mcu::toggleLed(mcu::LED_RED);
				mcu::delay_us(50000);
			}
		}

		mcu::delay_us(10000);
#endif
	}
}

















