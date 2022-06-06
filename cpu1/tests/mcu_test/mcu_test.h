///
#ifndef TESTS_MCU_TEST_MCU_TEST_H_
#define TESTS_MCU_TEST_MCU_TEST_H_

#include "emb/emb_testrunner/emb_testrunner.h"
#include "mcu/system/mcusystem.h"
#include "mcu/gpio/mcugpio.h"
#include "mcu/cputimers/mcucputimers.h"
#include "mcu/support/mcusupport.h"


class McuTest
{
public:
	static void GpioTest();
	static void ClockTest();
};


#endif /* TESTS_MCU_TEST_MCU_TEST_H_ */
