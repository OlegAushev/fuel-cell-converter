///
#include "boardhardware_test.h"

/*
void BoardHardwareTest::startPwm(srm::Drive* drive)
{
	srm::PwmCmpValues pwmCmpValues;
	pwmCmpValues.converterPh3[0] = drive->converter.pwm3PhUnit.period() / 6;
	pwmCmpValues.converterPh3[1] = drive->converter.pwm3PhUnit.period() / 5;
	pwmCmpValues.converterPh3[2] = drive->converter.pwm3PhUnit.period() / 4;
	pwmCmpValues.converterPh1[0] = drive->converter.pwm1PhUnit.period() / 3;
	drive->converter.updateCompareValues(pwmCmpValues);
	drive->converter.start();
}


mcu::ClockTaskStatus toggleOutputs()
{
	static bool isOn = false;

	if (!isOn)
	{
#ifdef _LAUNCHXL_F28379D
		mcu::turnLedOn(mcu::LED_BLUE);
#endif
		DigitalIO::setPinState(DigitalIO::CHARGING_CONTACTOR, mcu::PIN_ACTIVE);
		DigitalIO::setPinState(DigitalIO::POWER_CONTACTOR, mcu::PIN_ACTIVE);
		DigitalIO::setPinState(DigitalIO::FAN, mcu::PIN_ACTIVE);
		isOn = true;
	}
	else
	{
#ifdef _LAUNCHXL_F28379D
		mcu::turnLedOff(mcu::LED_BLUE);
#endif
		DigitalIO::setPinState(DigitalIO::CHARGING_CONTACTOR, mcu::PIN_INACTIVE);
		DigitalIO::setPinState(DigitalIO::POWER_CONTACTOR, mcu::PIN_INACTIVE);
		DigitalIO::setPinState(DigitalIO::FAN, mcu::PIN_INACTIVE);
		isOn = false;
	}

	return mcu::CLOCK_TASK_SUCCESS;
}


void BoardHardwareTest::startOutputsToggling()
{
	mcu::Clock mcuClock;
	mcuClock.registerTask(0, toggleOutputs);
}
*/
