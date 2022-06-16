
/*=====================*/

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
#ifdef CONTROLCARD
	///mcu::turnLedOn(mcu::LED_RED);	// BEGIN of CPU2 PERIPHERY CONFIGURATION and OBJECTS CREATION
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

	microcanopen::TpdoService<mcu::CANA> tpdoService(converter);
	microcanopen::RpdoService<mcu::CANA> rpdoService;
	microcanopen::SdoService<mcu::CANA> sdoService;
	microcanopen::McoServer<mcu::CANA, emb::MODE_MASTER> mcoServer(
			mcu::GpioPinConfig(19, GPIO_19_CANTXA),
			mcu::GpioPinConfig(18, GPIO_18_CANRXA),
			mcu::CAN_BITRATE_125K, mcu::CAN_NORMAL_MODE,
			microcanopen::NodeId(1),
			&tpdoService, &rpdoService, &sdoService, canIpcSignals);

	mcoServer.setHeartbeatPeriod(1000);
	mcoServer.setTpdoPeriod(microcanopen::TPDO_NUM1, 1000);
	mcoServer.setTpdoPeriod(microcanopen::TPDO_NUM2, 1000);
	mcoServer.setTpdoPeriod(microcanopen::TPDO_NUM3, 1000);
	mcoServer.setTpdoPeriod(microcanopen::TPDO_NUM4, 1000);

	mcoServer.setRpdoId(microcanopen::RPDO_NUM1, 0x194);
	mcoServer.setRpdoId(microcanopen::RPDO_NUM2, 0x294);

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
	mcoServer.enable();
	mcu::enableMaskableInterrupts();
	mcu::enableDebugEvents();

	while (true)
	{
		mcoServer.run();
		mcu::SystemClock::runTasks();
	}
}

















