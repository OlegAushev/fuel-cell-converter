///
#include "emb/emb_testrunner/emb_testrunner.h"
#include "emb/tests/emb_test.h"

#include "mcu_test/mcu_test.h"
#include "mcu/adc/mcu_adc.h"
#include "ucanopen_test/tpdoservice_test/tpdoservice_test.h"
#include "ucanopen_test/rpdoservice_test/rpdoservice_test.h"
#include "ucanopen_test/sdoservice_test/sdoservice_test.h"


void RUN_TESTS()
{
#ifdef _LAUNCHXL_F28379D
	mcu::configureLaunchPadLeds(GPIO_CORE_CPU1, GPIO_CORE_CPU1);
#endif
	Syslog::init(Syslog::IpcFlags());
	mcu::SystemClock::init();

	mcu::AdcConfig adcConfig =
	{
		.sampleWindow_ns = 200,
	};
	mcu::Adc adc(adcConfig);
	adc.enableInterrupts();

	for (size_t i = 0; i < 10; ++i)
	{
		mcu::toggleLed(mcu::LED_RED);
		DEVICE_DELAY_US(100000);
	}
	DEVICE_DELAY_US(100000);
	mcu::enableMaskableInterrupts();
	mcu::enableDebugEvents();

	printf("\nTesting...\n");
	EMB_RUN_TEST(EmbTest::CommonTest);
	EMB_RUN_TEST(EmbTest::MathTest);
	EMB_RUN_TEST(EmbTest::AlgorithmTest);
	EMB_RUN_TEST(EmbTest::ArrayTest);
	EMB_RUN_TEST(EmbTest::QueueTest);
	EMB_RUN_TEST(EmbTest::CircularBufferTest);
	EMB_RUN_TEST(EmbTest::FilterTest);
	EMB_RUN_TEST(EmbTest::StackTest);
	EMB_RUN_TEST(EmbTest::BitsetTest);

	EMB_RUN_TEST(McuTest::GpioTest);
	EMB_RUN_TEST(McuTest::ClockTest);

	EMB_RUN_TEST(ucanopen::TpdoServiceTest::MessageProcessingTest);
	EMB_RUN_TEST(ucanopen::RpdoServiceTest::MessageProcessingTest);
	EMB_RUN_TEST(ucanopen::SdoServiceTest::MessageProcessingTest);


	emb::TestRunner::printResult();

	if (emb::TestRunner::passed())
	{
#ifdef _LAUNCHXL_F28379D
		mcu::turnLedOn(mcu::LED_BLUE);
#endif
		while (true)
		{
			mcu::toggleLed(mcu::LED_RED);
			DEVICE_DELAY_US(100000);
		}
	}
	else
	{
		mcu::turnLedOn(mcu::LED_RED);
		while (true) {}
	}
}




