
/*=====================*/

#include "F28x_Project.h"
#include "device.h"

#include "profiler/profiler.h"

#include "mcu/system/mcusystem.h"
#include "mcu/ipc/mcuipc.h"
#include "mcu/cputimers/mcucputimers.h"
#include "mcoserver/mcoserver.h"
#include "mcu/spi/mcuspi.h"
#include "mcu/dac/mcudac.h"

#include "syslog/syslog.h"
#include "clocktasks/cpu2clocktasks.h"


/* ========================================================================== */
/* =========================== DRIVE POINTERS =============================== */
/* ========================================================================== */
#if (defined(ACIM_MOTOR_SIX_PHASE))
#pragma DATA_SECTION("DRIVE")
unsigned char driveobj_loc[sizeof(acim::Drive<acim::SIX_PHASE, acim::DRIVE_INSTANCE_1>)];
acim::Drive<acim::SIX_PHASE, acim::DRIVE_INSTANCE_1>* drive;

#elif (defined(ACIM_MOTOR_THREE_PHASE))
#pragma DATA_SECTION("DRIVE")
unsigned char driveobj_loc[sizeof(acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_1>)];
acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_1>* drive;

#elif (defined(ACIM_TWO_MOTORS))
#pragma DATA_SECTION("DRIVE")
unsigned char driveobj_loc[sizeof(acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_1>)];
acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_1>* drive;

#pragma DATA_SECTION("DRIVE2")
unsigned char drive2obj_loc[sizeof(acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_2>)];
acim::Drive<acim::THREE_PHASE, acim::DRIVE_INSTANCE_2>* drive2;
#endif


/**
 * @brief Systick timer ISR.
 * @return (none)
 */
__interrupt void onSystickInterrupt()
{
	static uint64_t counter = 0;
	float valSin = 200 + 200 * sinf(emb::PI * counter / 1000);
	float valCos = 400 + 400 * cosf(emb::PI * counter / 1000);

	mcu::DacInput ch1(valSin, mcu::DACA);
	mcu::DacInput ch2(valCos, mcu::DACB);

	mcu::SpiUnit<mcu::SPIA>::instance()->send<emb::Pair<mcu::DacInput, mcu::DacInput> >
			(emb::Pair<mcu::DacInput, mcu::DacInput>(ch1, ch2));
}


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
	Syslog::init();

/*####################################################################################################################*/
	/*#########*/
	/*# CLOCK #*/
	/*#########*/
	mcu::Clock::init();
	mcu::Clock::setTaskPeriod(0, 1000);	// Led toggle period
	mcu::Clock::registerTask(0, taskToggleLed);
	mcu::SystickTimer::init(100);
	mcu::SystickTimer::registerInterruptHandler(onSystickInterrupt);
	mcu::SystickTimer::start();

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
	/*# PWM #*/
	/*#####################*/


/*####################################################################################################################*/
	/*#######*/
	/*# CAN #*/
	/*#######*/
	microcanopen::TpdoService tpdoService;
	microcanopen::McoServer<mcu::CANA, emb::MODE_MASTER> uCanOpenServer(mcu::CANA_TX_GPIO_31, mcu::CANA_RX_GPIO_30,
			mcu::CAN_BITRATE_500K, microcanopen::NodeId(1),
			&tpdoService, NULL, NULL);

	uCanOpenServer.setHeartbeatPeriod(1000);
	uCanOpenServer.setTpdoPeriod(microcanopen::TPDO_NUM1, 1000);
	uCanOpenServer.setTpdoPeriod(microcanopen::TPDO_NUM2, 1000);
	uCanOpenServer.setTpdoPeriod(microcanopen::TPDO_NUM3, 1000);
	uCanOpenServer.setTpdoPeriod(microcanopen::TPDO_NUM4, 1000);

	uCanOpenServer.setRpdoId(microcanopen::RPDO_NUM1, 0x194);
	uCanOpenServer.setRpdoId(microcanopen::RPDO_NUM2, 0x294);

/*####################################################################################################################*/
	/*#######*/
	/*# SPI #*/
	/*#######*/
	mcu::SpiConfig spiCfg =
	{
		.protocol = mcu::SPI_PROTOCOL_POL1PHA0,
		.mode = mcu::SPI_MODE_MASTER,
		.bitrate = mcu::SPI_BITRATE_1M,
		.wordLen = mcu::SPI_WORD_16BIT,
		.dataSize = 2,
	};
	mcu::SpiUnit<mcu::SPIA> spiUnit(mcu::SPIA_MOSI_GPIO_16, mcu::SPIA_MISO_GPIO_17,
			mcu::SPIA_CLK_GPIO_18, mcu::SPIA_CS_GPIO_19, spiCfg);

/*####################################################################################################################*/
	/*##################*/
	/*# IPC INTERRUPTS #*/
	/*##################*/
	mcu::registerIpcInterruptHandler(mcu::IPC_INTERRUPT_3, Syslog::onFaultsAndWarningsReset);

	mcu::sendIpcSignal(CPU2_PERIPHERY_CONFIGURED);
	mcu::waitForIpcSignal(CPU1_PERIPHERY_CONFIGURED);

	// END of CPU2 PERIPHERY CONFIGURATION and OBJECTS CREATION
/*####################################################################################################################*/

#ifdef CONTROLCARD
	///mcu::turnLedOff(mcu::LED_RED);
#endif

	mcu::Clock::reset();
	uCanOpenServer.enable();
	mcu::enableMaskableInterrupts();
	mcu::enableDebugEvents();

	while (true)
	{
		uCanOpenServer.run();
		mcu::Clock::runPeriodicTasks();
	}
}

















