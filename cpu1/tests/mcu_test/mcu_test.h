///
#ifndef TESTS_MCU_TEST_MCU_TEST_H_
#define TESTS_MCU_TEST_MCU_TEST_H_

#include "emb/emb_testrunner/emb_testrunner.h"
#include "mcu_c28x/system/mcu_system.h"
#include "mcu_c28x/gpio/mcu_gpio.h"
#include "mcu_c28x/cputimers/mcu_cputimers.h"
#include "mcu_c28x/support/mcu_support.h"


class McuTest
{
public:
	static void GpioTest();
	static void ClockTest();
};


#endif /* TESTS_MCU_TEST_MCU_TEST_H_ */
