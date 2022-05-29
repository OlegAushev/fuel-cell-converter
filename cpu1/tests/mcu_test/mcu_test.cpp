///
#include "mcu_test.h"

///
///
///
void McuTest::GpioTest()
{
#ifdef _LAUNCHXL_F28379D
	mcu::GpioPin ledBlue(31, GPIO_31_GPIO31, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);
	mcu::GpioPin ledRed(34, GPIO_34_GPIO34, mcu::PIN_OUTPUT, mcu::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);

	mcu::turnLedOff(mcu::LED_BLUE);
	mcu::turnLedOff(mcu::LED_RED);

	EMB_ASSERT_EQUAL(mcu::readPinState(ledBlue), mcu::PIN_ACTIVE);
	EMB_ASSERT_EQUAL(mcu::readPinState(ledRed), mcu::PIN_INACTIVE);

	mcu::setPinState(ledBlue, mcu::PIN_INACTIVE);	// led - on
	mcu::setPinState(ledRed, mcu::PIN_INACTIVE);	// led - off
	DEVICE_DELAY_US(100000);

	EMB_ASSERT_EQUAL(GPIO_readPin(31), 0);
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 1);
	EMB_ASSERT_EQUAL(mcu::readPinState(ledBlue), mcu::PIN_INACTIVE);
	EMB_ASSERT_EQUAL(mcu::readPinState(ledRed), mcu::PIN_INACTIVE);

	mcu::setPinState(ledBlue, mcu::PIN_ACTIVE);	// led - off
	mcu::setPinState(ledRed, mcu::PIN_ACTIVE);	// led - on
	DEVICE_DELAY_US(100000);

	EMB_ASSERT_EQUAL(GPIO_readPin(31), 1);
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 0);
	EMB_ASSERT_EQUAL(mcu::readPinState(ledBlue), mcu::PIN_ACTIVE);
	EMB_ASSERT_EQUAL(mcu::readPinState(ledRed), mcu::PIN_ACTIVE);

	mcu::turnLedOff(mcu::LED_BLUE);
	mcu::turnLedOff(mcu::LED_RED);
#elif defined(RUNTESTS)
#warning "LAUNCHXL is required for full testing."
#endif

#ifdef _LAUNCHXL_F28379D
	mcu::GpioPin out = mcu::GpioPin(27, GPIO_27_GPIO27, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);
	mcu::GpioPin in1 = mcu::GpioPin(25, GPIO_25_GPIO25, mcu::PIN_INPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);
	mcu::GpioPin in2 = mcu::GpioPin(25, GPIO_25_GPIO25, mcu::PIN_INPUT, mcu::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);

	mcu::initGpioPin(out);
	mcu::initGpioPin(in1);
	mcu::PinDebouncer db1(in1, 10, 20, 30);

	EMB_ASSERT_EQUAL(mcu::readPinState(in1), mcu::PIN_INACTIVE);
	EMB_ASSERT_EQUAL(db1.state(), mcu::PIN_INACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());

	mcu::setPinState(out, mcu::PIN_ACTIVE);
	EMB_ASSERT_EQUAL(mcu::readPinState(in1), mcu::PIN_ACTIVE);
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), mcu::PIN_INACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), mcu::PIN_ACTIVE);
	EMB_ASSERT_TRUE(db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), mcu::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());


	mcu::setPinState(out, mcu::PIN_INACTIVE);
	EMB_ASSERT_EQUAL(mcu::readPinState(in1), mcu::PIN_INACTIVE);
	EMB_ASSERT_EQUAL(db1.state(), mcu::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), mcu::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), mcu::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), mcu::PIN_INACTIVE);
	EMB_ASSERT_TRUE(db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), mcu::PIN_INACTIVE);
	EMB_ASSERT_TRUE(!db1.stateChanged());


	mcu::setPinState(out, mcu::PIN_INACTIVE);
	mcu::initGpioPin(in2);
	mcu::PinDebouncer db2(in2, 10, 40, 20);
	EMB_ASSERT_EQUAL(mcu::readPinState(in2), mcu::PIN_ACTIVE);
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_INACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_INACTIVE);
	db2.debounce();
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_INACTIVE);
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_ACTIVE);
	EMB_ASSERT_TRUE(db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());

	mcu::setPinState(out, mcu::PIN_ACTIVE);
	EMB_ASSERT_EQUAL(mcu::readPinState(in2), mcu::PIN_INACTIVE);
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_INACTIVE);
	EMB_ASSERT_TRUE(db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_INACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());

	mcu::setPinState(out, mcu::PIN_INACTIVE);
	EMB_ASSERT_EQUAL(mcu::readPinState(in2), mcu::PIN_ACTIVE);
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_INACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_INACTIVE);
	db2.debounce();
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_INACTIVE);
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_ACTIVE);
	EMB_ASSERT_TRUE(db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), mcu::PIN_ACTIVE);
	EMB_ASSERT_TRUE(!db2.stateChanged());
#elif defined(RUNTESTS)
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

	mcu::Clock::registerDelayedTask(TestingDelayedTask, 200);
	DEVICE_DELAY_US(150000);
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 1);
	DEVICE_DELAY_US(100000);
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 0);

	mcu::turnLedOff(mcu::LED_RED);
}









