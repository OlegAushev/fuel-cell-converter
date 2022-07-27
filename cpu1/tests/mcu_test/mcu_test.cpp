///
#include "mcu_test.h"

///
///
///
void McuTest::GpioTest()
{
#ifdef _LAUNCHXL_F28379D
	mcu::GpioConfig ledBlueCfg(31, GPIO_31_GPIO31, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);
	mcu::GpioConfig ledRedCfg(34, GPIO_34_GPIO34, mcu::PIN_OUTPUT, emb::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);

	mcu::Gpio ledBlue(ledBlueCfg);
	mcu::Gpio ledRed(ledRedCfg);

	mcu::turnLedOff(mcu::LED_BLUE);
	mcu::turnLedOff(mcu::LED_RED);

	EMB_ASSERT_EQUAL(ledBlue.read(), emb::PIN_ACTIVE);
	EMB_ASSERT_EQUAL(ledRed.read(), emb::PIN_INACTIVE);

	ledBlue.set(emb::PIN_INACTIVE);	// led - on
	ledRed.set(emb::PIN_INACTIVE);	// led - off
	DEVICE_DELAY_US(100000);

	EMB_ASSERT_EQUAL(GPIO_readPin(31), 0);
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 1);
	EMB_ASSERT_EQUAL(ledBlue.read(), emb::PIN_INACTIVE);
	EMB_ASSERT_EQUAL(ledRed.read(), emb::PIN_INACTIVE);

	ledBlue.set(emb::PIN_ACTIVE);	// led - off
	ledRed.set(emb::PIN_ACTIVE);	// led - on
	DEVICE_DELAY_US(100000);

	EMB_ASSERT_EQUAL(GPIO_readPin(31), 1);
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 0);
	EMB_ASSERT_EQUAL(ledBlue.read(), emb::PIN_ACTIVE);
	EMB_ASSERT_EQUAL(ledRed.read(), emb::PIN_ACTIVE);

	mcu::turnLedOff(mcu::LED_BLUE);
	mcu::turnLedOff(mcu::LED_RED);
#elif defined(TEST_BUILD)
#warning "LAUNCHXL is required for full testing."
#endif

#ifdef _LAUNCHXL_F28379D
	mcu::GpioConfig outCfg(27, GPIO_27_GPIO27, mcu::PIN_OUTPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);
	mcu::GpioConfig in1Cfg(25, GPIO_25_GPIO25, mcu::PIN_INPUT, emb::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);
	mcu::GpioConfig in2Cfg(25, GPIO_25_GPIO25, mcu::PIN_INPUT, emb::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);

	mcu::Gpio out(outCfg);
	mcu::Gpio in1(in1Cfg);
	mcu::GpioDebouncer db1(in1, 10, 20, 30);

	EMB_ASSERT_EQUAL(in1.read(), emb::PIN_INACTIVE);
	EMB_ASSERT_EQUAL(db1.state(), emb::PIN_INACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());

	out.set(emb::PIN_ACTIVE);
	EMB_ASSERT_EQUAL(in1.read(), emb::PIN_ACTIVE);
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PIN_INACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PIN_ACTIVE);
	EMB_ASSERT_TRUE(db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());


	out.set(emb::PIN_INACTIVE);
	EMB_ASSERT_EQUAL(in1.read(), emb::PIN_INACTIVE);
	EMB_ASSERT_EQUAL(db1.state(), emb::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PIN_INACTIVE);
	EMB_ASSERT_TRUE(db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PIN_INACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());


	out.set(emb::PIN_INACTIVE);
	mcu::Gpio in2(in2Cfg);
	mcu::GpioDebouncer db2(in2, 10, 40, 20);
	EMB_ASSERT_EQUAL(in2.read(), emb::PIN_ACTIVE);
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_INACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_INACTIVE);
	db2.debounce();
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_INACTIVE);
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_ACTIVE);
	EMB_ASSERT_TRUE(db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());

	out.set(emb::PIN_ACTIVE);
	EMB_ASSERT_EQUAL(in2.read(), emb::PIN_INACTIVE);
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_INACTIVE);
	EMB_ASSERT_TRUE(db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_INACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());

	out.set(emb::PIN_INACTIVE);
	EMB_ASSERT_EQUAL(in2.read(), emb::PIN_ACTIVE);
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_INACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_INACTIVE);
	db2.debounce();
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_INACTIVE);
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_ACTIVE);
	EMB_ASSERT_TRUE(db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());
#elif defined(TEST_BUILD)
#warning "LAUNCHXL is required for full testing."
#endif
}

///
///
///
void TestingDelayedTask()
{
	mcu::turnLedOn(mcu::LED_RED);
}

///
///
///
void McuTest::ClockTest()
{
	mcu::turnLedOff(mcu::LED_RED);

	mcu::SystemClock::registerDelayedTask(TestingDelayedTask, 200);
	DEVICE_DELAY_US(150000);
	mcu::SystemClock::runTasks();
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 1);
	DEVICE_DELAY_US(100000);
	mcu::SystemClock::runTasks();
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 0);

	mcu::turnLedOff(mcu::LED_RED);
}









