///
#define FIRMWARE_VERSION_STRDEF "v22.07"
#define FIRMWARE_VERSION_NUMDEF 2207


//#define TEST_CAN_BY_GPIO
#ifdef TEST_CAN_BY_GPIO
#warning "TEST_CAN_BY_GPIO test build."
#endif

#ifdef CRD300
#warning "CRD300-build."
#endif

#ifndef DUALCORE
#warning "Singlecore-build."
#endif

#define NO_EXTERNAL_STORAGE
#ifdef NO_EXTERNAL_STORAGE
//#warning "External storage is disabled."
#endif


#include "F28x_Project.h"
#include "device.h"
#include <new>

#include "emb/emb_profiler/emb_profiler.h"
#include "profiler/profiler.h"

#include "mcu/system/mcu_system.h"
#include "mcu/support/mcu_support.h"
#include "mcu/ipc/mcu_ipc.h"
#include "mcu/cputimers/mcu_cputimers.h"
#include "mcu/adc/mcu_adc.h"
#include "mcu/can/mcu_can.h"
#include "ucanopen/ucanopen_server.h"
#include "mcu/spi/mcu_spi.h"
#include "mcu/dac/mcu_dac.h"

#include "sys/syslog/syslog.h"
#include "clocktasks/cpu1clocktasks.h"
#include "fuelcell/converter/fuelcell_converter.h"
#include "settings/settings.h"
#include "canbygpio/canbygpio.h"
#include "fuelcell/controller/fuelcell_controller.h"

#ifdef CRD300
#include "support/crd300/controller.h"
#endif

#ifdef TEST_BUILD
#include "emb/emb_testrunner/emb_testrunner.h"
#endif


unsigned char converterobj_loc[sizeof(fuelcell::Converter)] __attribute__((section("SHARED_CONVERTER")));
fuelcell::Converter* converter;

uint16_t dacaInput = 0;
uint16_t dacbInput = 0;


/* ========================================================================== */
/* ============================ SYSTEM INFO ================================= */
/* ========================================================================== */
const char* sys::DEVICE_NAME = "Fuel cell converter";
const char* sys::DEVICE_NAME_SHORT = "FCC";
const char* sys::FIRMWARE_VERSION = FIRMWARE_VERSION_STRDEF;
const uint32_t sys::FIRMWARE_VERSION_NUM = FIRMWARE_VERSION_NUMDEF;

#if defined(TEST_BUILD)
const char* sys::BUILD_CONFIGURATION = "TEST";
#elif defined(DEBUG)
const char* sys::BUILD_CONFIGURATION = "DBG";
#else
const char* sys::BUILD_CONFIGURATION = "RLS";
#endif
const char* sys::BUILD_CONFIGURATION_SHORT = sys::BUILD_CONFIGURATION;


/* ========================================================================== */
/* ============================ PROFILER PINS =============================== */
/* ========================================================================== */
const mcu::GpioConfig P61_CFG =
	mcu::GpioConfig(61, GPIO_61_GPIO61, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioConfig P123_CFG =
	mcu::GpioConfig(123, GPIO_123_GPIO123, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioConfig P122_CFG =
	mcu::GpioConfig(122, GPIO_122_GPIO122, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioConfig P22_CFG =
	mcu::GpioConfig(22, GPIO_22_GPIO22, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioConfig P111_CFG =
	mcu::GpioConfig(111, GPIO_111_GPIO111, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioConfig P56_CFG =
	mcu::GpioConfig(56, GPIO_56_GPIO56, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);


/* ========================================================================== */
/* ================================ MAIN ==================================== */
/* ========================================================================== */
/**
 * @brief main()
 */
void main()
{
	/*##########*/
	/*# SYSTEM #*/
	/*##########*/
	mcu::initDevice();
	MemCfg_setGSRAMMasterSel(MEMCFG_SECT_GS7, MEMCFG_GSRAMMASTER_CPU1);	// settings are placed here
	MemCfg_setGSRAMMasterSel(MEMCFG_SECT_GS8, MEMCFG_GSRAMMASTER_CPU2);	// CPU2 .bss is placed here
	MemCfg_setGSRAMMasterSel(MEMCFG_SECT_GS9, MEMCFG_GSRAMMASTER_CPU1);	// CPU1 to CPU2 data is placed here
	MemCfg_setGSRAMMasterSel(MEMCFG_SECT_GS10, MEMCFG_GSRAMMASTER_CPU2);	// CPU2 to CPU1 data is placed here

#ifdef _LAUNCHXL_F28379D
#ifdef DUALCORE
	mcu::configureLaunchPadLeds(GPIO_CORE_CPU1, GPIO_CORE_CPU2);
	mcu::turnLedOff(mcu::LED_BLUE);
#else
	mcu::configureLaunchPadLeds(GPIO_CORE_CPU1, GPIO_CORE_CPU1);
	mcu::turnLedOff(mcu::LED_BLUE);
	mcu::turnLedOff(mcu::LED_RED);
#endif
#endif

#ifdef TEST_BUILD
	RUN_TESTS();
#endif

#ifdef CRD300
	mcu::Gpio p61(P61_CFG);
	mcu::Gpio p123(P123_CFG);
	mcu::Gpio p122(P122_CFG);
	mcu::Gpio p22(P22_CFG);
	mcu::Gpio p111(P111_CFG);
	mcu::Gpio p56(P56_CFG);
#endif

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
	Syslog::addMessage(sys::Message::DEVICE_CPU1_BOOT_SUCCESS);

// BEGIN of CPU1 PERIPHERY CONFIGURATION and OBJECTS CREATION
/*####################################################################################################################*/
	/*#########*/
	/*# CLOCK #*/
	/*#########*/
	mcu::SystemClock::init();
	mcu::HighResolutionClock::init(1000000);
	mcu::HighResolutionClock::start();
	emb::DurationLogger_us::init(mcu::HighResolutionClock::now);
	emb::DurationLogger_clk::init(mcu::HighResolutionClock::counter);

	// ALL PERFORMANCE TESTS MUST BE PERFORMED AFTER THIS POINT!!!

	/*-------------------------*/
	/* PERFORMANCE TESTS BEGIN */
	/*-------------------------*/



	/*-----------------------*/
	/* PERFORMANCE TESTS END */
	/*-----------------------*/

/*####################################################################################################################*/
	/*###############*/
	/*# CAN BY GPIO #*/
	/*###############*/
	mcu::GpioConfig canbygpioRxCfg(14, GPIO_14_GPIO14, mcu::PIN_INPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_6SAMPLE, 1, GPIO_CORE_CPU2);
	mcu::GpioConfig canbygpioTxCfg(11, GPIO_11_GPIO11, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1, GPIO_CORE_CPU2);
	mcu::GpioConfig canbygpioClkCfg(15, GPIO_15_GPIO15, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1, GPIO_CORE_CPU2);
	mcu::Gpio canbygpioRx(canbygpioRxCfg);
	mcu::Gpio canbygpioTx(canbygpioTxCfg);
	mcu::Gpio canbygpioClk(canbygpioClkCfg);
	canbygpioRx.setInterrupt(GPIO_INT_XINT5);

#ifdef TEST_CAN_BY_GPIO
	ucanopen::IpcFlags canIpcSignalsTest =
	{
		.RPDO1_RECEIVED = mcu::IpcFlag(14),
		.RPDO2_RECEIVED = mcu::IpcFlag(15),
		.RPDO3_RECEIVED = mcu::IpcFlag(16),
		.RPDO4_RECEIVED = mcu::IpcFlag(17),
		.RSDO_RECEIVED = mcu::IpcFlag(18),
		.TSDO_READY = mcu::IpcFlag(19),
	};

	ucanopen::SdoService<mcu::CANB, mcu::IPC_MODE_SINGLECORE, emb::MODE_MASTER> sdoServiceTest(NULL);
	ucanopen::TpdoService<mcu::CANB, mcu::IPC_MODE_SINGLECORE, emb::MODE_MASTER> tpdoServiceTest(NULL);
	ucanopen::RpdoService<mcu::CANB, mcu::IPC_MODE_SINGLECORE, emb::MODE_MASTER> rpdoServiceTest(NULL);
	ucanopen::ucanopenServer<mcu::CANB, mcu::IPC_MODE_SINGLECORE, emb::MODE_MASTER> ucanopenServerTest(
			mcu::GpioPinConfig(6, GPIO_6_CANTXB),
			mcu::GpioPinConfig(7, GPIO_7_CANRXB),
			mcu::CAN_BITRATE_125K, mcu::CAN_LOOPBACK_MODE,
			ucanopen::NodeId(0x01),
			&tpdoServiceTest, &rpdoServiceTest, &sdoServiceTest, canIpcSignalsTest);

	ucanopenServerTest.setHeartbeatPeriod(0);
	ucanopenServerTest.setTpdoPeriod(ucanopen::TPDO_NUM1, 200);
	ucanopenServerTest.setTpdoPeriod(ucanopen::TPDO_NUM2, 200);
	ucanopenServerTest.setTpdoPeriod(ucanopen::TPDO_NUM3, 200);
	ucanopenServerTest.setTpdoPeriod(ucanopen::TPDO_NUM4, 200);

	ucanopenServerTest.setRpdoId(ucanopen::RPDO_NUM1, 0x200);
	ucanopenServerTest.enable();
#endif

/*####################################################################################################################*/
#ifdef CRD300
	/*###################################*/
	/*# XM3 CONTROLLER & MOSFET DRIVERS #*/
	/*###################################*/
	crd300::Controller crd300;
	crd300.disableDriverLogic();
	crd300.enableDriverPS();
	mcu::delay_us(200);
	crd300.resetDrivers();
	crd300.enableNeg15V();
	crd300.enablePos15V();
	mcu::delay_us(1e5);
#endif

/*####################################################################################################################*/
	/*#################*/
	/*# POWERUP DELAY #*/
	/*#################*/
	for (size_t i = 0; i < 10; ++i)
	{
#ifdef _LAUNCHXL_F28379D
		mcu::toggleLed(mcu::LED_BLUE);
#endif
		mcu::delay_us(100000);
	}

/*####################################################################################################################*/
	/*############*/
	/*# SETTINGS #*/
	/*############*/
	Settings::init();
	Settings settings;
	Settings::SYSTEM_CONFIG = Settings::DEFAULT_CONFIG;

#ifdef CRD300
	mcu::GpioConfig drvFltPinCfg(15, GPIO_15_GPIO15, mcu::PIN_INPUT, emb::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
#else
	mcu::GpioConfig drvFltPinCfg;
#endif
	mcu::Gpio drvFltPin(drvFltPinCfg);
	Settings::SYSTEM_CONFIG.CONVERTER_CONFIG.fltPin = drvFltPin;

/*####################################################################################################################*/
	/*#############*/
	/*# BOOT CPU2 #*/
	/*#############*/
#ifdef DUALCORE
	mcu::bootCpu2();
	Syslog::addMessage(sys::Message::DEVICE_BOOT_CPU2);
	mcu::waitForRemoteIpcFlag(CPU2_BOOTED);
	Syslog::addMessage(sys::Message::DEVICE_CPU2_BOOT_SUCCESS);
#endif

/*####################################################################################################################*/
	/*#######*/
	/*# ADC #*/
	/*#######*/
	mcu::AdcConfig adcConfig =
	{
		.sampleWindow_ns = 200,
	};

	mcu::Adc adc(adcConfig);

/*####################################################################################################################*/
	/*#######*/
	/*# DAC #*/
	/*#######*/
	mcu::Dac<mcu::DACA> daca;
	mcu::Dac<mcu::DACB> dacb;

/*####################################################################################################################*/
	/*#############*/
	/*# CONVERTER #*/
	/*#############*/
	converter = new(converterobj_loc) fuelcell::Converter(
			Settings::SYSTEM_CONFIG.CONVERTER_CONFIG,
			Settings::SYSTEM_CONFIG.PWM_CONFIG);

	settings.registerObjects(converter);

#ifdef CRD300
	crd300.enableDriverLogic();	// PWM outputs now ready
#endif

/*####################################################################################################################*/
	/*###############*/
	/*# CLOCK TASKS #*/
	/*###############*/
	mcu::SystemClock::setTaskPeriod(0, 1000);
	mcu::SystemClock::registerTask(0, taskToggleLed);

	mcu::SystemClock::setTaskPeriod(1, 50);
	mcu::SystemClock::registerTask(1, taskStartTempSensors);

	mcu::SystemClock::setWatchdogPeriod(1000);
	mcu::SystemClock::registerWatchdogTask(taskWatchdogTimeout);

/*####################################################################################################################*/
	/*#######*/
	/*# CAN #*/
	/*#######*/
	ucanopen::IpcFlags canIpcFlags =
	{
		.RPDO1_RECEIVED = mcu::IpcFlag(4),
		.RPDO2_RECEIVED = mcu::IpcFlag(5),
		.RPDO3_RECEIVED = mcu::IpcFlag(6),
		.RPDO4_RECEIVED = mcu::IpcFlag(7),
		.RSDO_RECEIVED = mcu::IpcFlag(8),
		.TSDO_READY = mcu::IpcFlag(9),
	};

	ucanopen::RpdoService<mcu::CANA, mcu::IPC_MODE_SINGLECORE, emb::MODE_MASTER> rpdoService(converter);
	ucanopen::TpdoService<mcu::CANA, mcu::IPC_MODE_SINGLECORE, emb::MODE_MASTER> tpdoService(converter);
	ucanopen::SdoService<mcu::CANA, mcu::IPC_MODE_SINGLECORE, emb::MODE_MASTER> sdoService(converter);
	ucanopen::Server<mcu::CANA, mcu::IPC_MODE_SINGLECORE, emb::MODE_MASTER> ucanopenServer(
			mcu::GpioConfig(19, GPIO_19_CANTXA),
			mcu::GpioConfig(18, GPIO_18_CANRXA),
			mcu::CAN_BITRATE_125K, mcu::CAN_NORMAL_MODE,
			ucanopen::NodeId(0x01),
			&tpdoService, &rpdoService, &sdoService, canIpcFlags);

	ucanopenServer.setHeartbeatPeriod(1000);
	ucanopenServer.setTpdoPeriod(ucanopen::TPDO_NUM1, 1000);
	ucanopenServer.setTpdoPeriod(ucanopen::TPDO_NUM2, 1000);
	ucanopenServer.setTpdoPeriod(ucanopen::TPDO_NUM3, 1000);
	ucanopenServer.setTpdoPeriod(ucanopen::TPDO_NUM4, 1000);

	ucanopenServer.setRpdoId(ucanopen::RPDO_NUM1, 0x1A0);
	ucanopenServer.setRpdoId(ucanopen::RPDO_NUM2, 0x2A0);
	ucanopenServer.setRpdoId(ucanopen::RPDO_NUM3, 0x194);

/*####################################################################################################################*/
	/*##################*/
	/*# IPC INTERRUPTS #*/
	/*##################*/
	// NONE

#ifdef DUALCORE
	mcu::waitForRemoteIpcFlag(CPU2_PERIPHERY_CONFIGURED);
	Syslog::addMessage(sys::Message::DEVICE_CPU2_READY);
#endif

	mcu::enableMaskableInterrupts();
	mcu::enableDebugEvents();

/*####################################################################################################################*/
	/*###################*/
	/*# ADC PREPARATION #*/
	/*###################*/
	adc.enableInterrupts();

	// wait for pending ADC INTs (after ADC calibrating) be served
	mcu::delay_us(100);

	// now PWM can be launched
	converter->pwm.acknowledgeEventInterrupt();
	converter->pwm.enableEventInterrupts();
	converter->pwm.enableTripInterrupts();

// END of CPU1 PERIPHERY CONFIGURATION and OBJECTS CREATION
/*####################################################################################################################*/
	mcu::SystemClock::reset();

#ifdef DUALCORE
	// CPU1 has finished all preparations, CPU2 can enable all interrupts
	mcu::setLocalIpcFlag(CPU1_PERIPHERY_CONFIGURED);
#endif
	Syslog::addMessage(sys::Message::DEVICE_CPU1_READY);
	ucanopenServer.enable();

	mcu::SystemClock::enableWatchdog();

/*####################################################################################################################*/
	Syslog::addMessage(sys::Message::DEVICE_READY);

	while (true)
	{
		Syslog::processIpcSignals();
		ucanopenServer.run();
		mcu::SystemClock::runTasks();
		converter->processTemperatureMeasurements();
		// TODO check fuel cells status

#ifdef TEST_CAN_BY_GPIO
		ucanopenServerTest.run();
		static uint64_t sigPrev = 0;
		static bool hasStarted = false;
		if (mcu::SystemClock::now() > (sigPrev + 1000))
		{
			if (!hasStarted)
			{
				fuelcell::Controller::start();
				hasStarted = true;
			}
			else
			{
				fuelcell::Controller::stop();
				hasStarted = false;
			}

			sigPrev = mcu::SystemClock::now();
		}
#endif

		daca.convert(mcu::DacInput(dacaInput));
		dacb.convert(mcu::DacInput(dacbInput));
	}
}















