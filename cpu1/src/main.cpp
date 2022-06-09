///
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

/*=====================*/
#include "F28x_Project.h"
#include "device.h"
#include <new>

#include "emb/emb_profiler/emb_profiler.h"
#include "profiler/profiler.h"

#include "mcu/system/mcusystem.h"
#include "mcu/support/mcusupport.h"
#include "mcu/ipc/mcuipc.h"
#include "mcu/cputimers/mcucputimers.h"
#include "mcu/adc/mcuadc.h"
#include "mcu/can/mcucan.h"
#include "mcoserver/mcoserver.h"
#include "mcu/spi/mcuspi.h"

#include "syslog/syslog.h"
#include "clocktasks/cpu1clocktasks.h"
#include "boostconverter/boostconverter.h"
#include "setupmanager/setupmanager.h"

#ifdef CRD300
#include "support/crd300/controller.h"
#endif

#ifdef RUNTESTS
#include "emb/emb_testrunner/emb_testrunner.h"
#endif


#pragma DATA_SECTION("SHARED_CONVERTER")
unsigned char converterobj_loc[sizeof(BoostConverter)];
BoostConverter* converter;
float DUTY_CYCLE = 0.25;

/* ========================================================================== */
/* ============================ SYSTEM INFO ================================= */
/* ========================================================================== */
const char* Syslog::DEVICE_NAME = "FCC";
const uint32_t Syslog::SOFTWARE_VERSION = 2206;

#if defined(RUNTESTS)
const char* Syslog::BUILD_CONFIGURATION = "TEST";
#elif defined(DEBUG)
const char* Syslog::BUILD_CONFIGURATION = "DBG";
#else
const char* Syslog::BUILD_CONFIGURATION = "RLS";
#endif


/* ========================================================================== */
/* ============================ PROFILER PINS =============================== */
/* ========================================================================== */
const mcu::GpioPinConfig P61_CFG =
	mcu::GpioPinConfig(61, GPIO_61_GPIO61, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioPinConfig P123_CFG =
	mcu::GpioPinConfig(123, GPIO_123_GPIO123, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioPinConfig P122_CFG =
	mcu::GpioPinConfig(122, GPIO_122_GPIO122, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioPinConfig P22_CFG =
	mcu::GpioPinConfig(22, GPIO_22_GPIO22, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);


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
	MemCfg_setGSRAMMasterSel(MEMCFG_SECT_GS7, MEMCFG_GSRAMMASTER_CPU1);	// configs are placed here
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

#ifdef RUNTESTS
	RUN_TESTS();
#endif

#ifdef CRD300
	mcu::GpioPin p61(P61_CFG);
	mcu::GpioPin p123(P123_CFG);
	mcu::GpioPin p122(P122_CFG);
	mcu::GpioPin p22(P22_CFG);
#endif

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
	Syslog::addMessage(Syslog::DEVICE_CPU1_BOOT_SUCCESS);

// BEGIN of CPU1 PERIPHERY CONFIGURATION and OBJECTS CREATION
/*####################################################################################################################*/
	/*#########*/
	/*# CLOCK #*/
	/*#########*/
	mcu::SystemClock::init();
//	mcu::HighResolutionClock::init(1000000);
//	mcu::HighResolutionClock::start();
//	emb::DurationLogger::init(mcu::HighResolutionClock::now);
//	emb::DurationLoggerAsyncPrint::init(mcu::HighResolutionClock::now);

	// ALL PERFORMANCE TESTS MUST BE PERFORMED AFTER THIS POINT!!!

	/*-------------------------*/
	/* PERFORMANCE TESTS BEGIN */
	/*-------------------------*/

	/*-----------------------*/
	/* PERFORMANCE TESTS END */
	/*-----------------------*/

/*####################################################################################################################*/
#ifdef CRD300
	/*###################################*/
	/*# XM3 CONTROLLER & MOSFET DRIVERS #*/
	/*###################################*/
	crd300::Controller crd300;
	crd300.disableDriverLogic();	// Gate driver output will be held low if power supplies are enabled
	crd300.enableDriverPS();
	mcu::delay_us(200);
	crd300.resetDrivers();
	crd300.enableNeg15V();
	crd300.enablePos15V();		// current sensor power supply must be enabled before current sensors calibrating
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
	/*#################*/
	/*# SETUP MANAGER #*/
	/*#################*/
	SetupManager setupManager;
	SetupManager::SYSTEM_CONFIG = SetupManager::DEFAULT_CONFIG;

#ifdef CRD300
	mcu::GpioPinConfig drvFltPinCfg(15, GPIO_15_GPIO15, mcu::PIN_INPUT, mcu::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
	mcu::GpioPin drvFltPin(drvFltPinCfg);
#else
	mcu::GpioPin drvFltPin;
#endif
	SetupManager::SYSTEM_CONFIG.CONVERTER_CONFIG.fltPin = drvFltPin;

/*####################################################################################################################*/
	/*#############*/
	/*# BOOT CPU2 #*/
	/*#############*/
#ifdef DUALCORE
	mcu::CanUnit<mcu::CANA>::transferControlToCpu2(
			mcu::GpioPinConfig(19, GPIO_19_CANTXA),
			mcu::GpioPinConfig(18, GPIO_18_CANRXA));
	mcu::bootCpu2();
	Syslog::addMessage(Syslog::DEVICE_BOOT_CPU2);
	mcu::waitForIpcSignal(CPU2_BOOTED);
	Syslog::addMessage(Syslog::DEVICE_CPU2_BOOT_SUCCESS);
#endif

/*####################################################################################################################*/
	/*#######*/
	/*# ADC #*/
	/*#######*/
	mcu::AdcUnit mcuAdcUnit(mcu::ADC_CHANNEL_COUNT);

/*####################################################################################################################*/
	/*#####################*/
	/*# CONVERTER #*/
	/*#####################*/
	converter = new(converterobj_loc) BoostConverter(
			SetupManager::SYSTEM_CONFIG.CONVERTER_CONFIG,
			SetupManager::SYSTEM_CONFIG.PWM_CONFIG);

/*####################################################################################################################*/
	/*###############*/
	/*# CLOCK TASKS #*/
	/*###############*/
	mcu::SystemClock::setTaskPeriod(0, 1000);	// Led toggle period
	mcu::SystemClock::registerTask(0, taskToggleLed);

	mcu::SystemClock::setWatchdogBound(1000);
	mcu::SystemClock::registerWatchdogTask(taskWatchdogTimeout);

/*####################################################################################################################*/
	/*#######*/
	/*# CAN #*/
	/*#######*/
	microcanopen::IpcSignals canIpcSignals =
	{
		.rpdo1 = mcu::IpcSignalPair(4),
		.rpdo2 = mcu::IpcSignalPair(5),
		.rpdo3 = mcu::IpcSignalPair(6),
		.rpdo4 = mcu::IpcSignalPair(7),
		.rsdo = mcu::IpcSignalPair(8),
		.tsdo = mcu::IpcSignalPair(9),
	};

#ifdef DUALCORE
	microcanopen::SdoService<mcu::CANA> sdoService;
	microcanopen::RpdoService<mcu::CANA> rpdoService;
	microcanopen::McoServer<mcu::CANA, emb::MODE_SLAVE> uCanOpenServer(NULL, &rpdoService, &sdoService, canIpcSignals);
#else
	microcanopen::SdoService<mcu::CANA> sdoService;
	microcanopen::TpdoService<mcu::CANA> tpdoService(converter);
	microcanopen::RpdoService<mcu::CANA> rpdoService(converter);
	microcanopen::McoServer<mcu::CANA, emb::MODE_MASTER> uCanOpenServer(
			mcu::GpioPinConfig(19, GPIO_19_CANTXA),
			mcu::GpioPinConfig(18, GPIO_18_CANRXA),
			mcu::CAN_BITRATE_500K, microcanopen::NodeId(0x01),
			&tpdoService, &rpdoService, &sdoService, canIpcSignals);

	uCanOpenServer.setHeartbeatPeriod(1000);
	uCanOpenServer.setTpdoPeriod(microcanopen::TPDO_NUM1, 1000);
	uCanOpenServer.setTpdoPeriod(microcanopen::TPDO_NUM2, 1000);
	uCanOpenServer.setTpdoPeriod(microcanopen::TPDO_NUM3, 1000);
	uCanOpenServer.setTpdoPeriod(microcanopen::TPDO_NUM4, 1000);

	uCanOpenServer.setRpdoId(microcanopen::RPDO_NUM1, 0x194);
	uCanOpenServer.setRpdoId(microcanopen::RPDO_NUM2, 0x294);
#endif

/*####################################################################################################################*/
	/*##################*/
	/*# IPC INTERRUPTS #*/
	/*##################*/
	// NONE

#ifdef DUALCORE
	mcu::waitForIpcSignal(CPU2_PERIPHERY_CONFIGURED);
	Syslog::addMessage(Syslog::DEVICE_CPU2_READY);
#endif

	mcu::enableMaskableInterrupts();
	mcu::enableDebugEvents();

/*####################################################################################################################*/
	/*###################*/
	/*# ADC PREPARATION #*/
	/*###################*/
	mcuAdcUnit.enableInterrupts();
	mcu::delay_us(100);				// wait for pending ADC INTs (after ADC calibrating) be served
	// TODO drive->phaseCurrentSensor.resetCompleted();	// clear measurements possibly stored in Drive
	// TODO drive->dcVoltageSensor.resetCompleted();	// clear measurements possibly stored in Drive
	converter->pwmUnit.acknowledgeInterrupt();
	converter->pwmUnit.enableInterrupts();		// now PWM can be launched

// END of CPU1 PERIPHERY CONFIGURATION and OBJECTS CREATION
/*####################################################################################################################*/
	mcu::SystemClock::reset();

#ifdef DUALCORE
	// CPU1 has finished all preparations, CPU2 can now enable all interrupts
	mcu::sendIpcSignal(CPU1_PERIPHERY_CONFIGURED);
#endif
	Syslog::addMessage(Syslog::DEVICE_CPU1_READY);

#ifndef DUALCORE
	uCanOpenServer.enable();
#endif
	mcu::SystemClock::enableWatchdog();

/*####################################################################################################################*/
	Syslog::addMessage(Syslog::DEVICE_READY);

	while (true)
	{
		Syslog::processIpcSignals();
		uCanOpenServer.run();
		mcu::SystemClock::runPeriodicTasks();
		converter->pwmUnit.setDutyCycle(DUTY_CYCLE);
	}
}















