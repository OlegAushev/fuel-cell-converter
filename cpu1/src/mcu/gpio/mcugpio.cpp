/**
 * @file
 * @ingroup mcu mcu_gpio
 */


#include "mcugpio.h"


namespace mcu {

///
///
///
void initGpioPin(const GpioPin& pin)
{
	switch (pin.direction)
	{
	case PIN_OUTPUT:
		GPIO_setPadConfig(pin.no, pin.type);
		setPinState(pin, PIN_INACTIVE);
		GPIO_setPinConfig(pin.mux);
		GPIO_setDirectionMode(pin.no, static_cast<GPIO_Direction>(pin.direction));
		break;

	case PIN_INPUT:
		GPIO_setQualificationPeriod(pin.no, pin.qualPeriod);
		GPIO_setQualificationMode(pin.no, static_cast<GPIO_QualificationMode>(pin.qualMode));
		GPIO_setPadConfig(pin.no, pin.type);
		GPIO_setPinConfig(pin.mux);
		GPIO_setDirectionMode(pin.no, static_cast<GPIO_Direction>(pin.direction));
		break;
	}

	GPIO_setMasterCore(pin.no, pin.masterCore);
}

///
///
///
void setGpioPinMasterCore(const GpioPin& pin, GPIO_CoreSelect masterCore)
{
	GPIO_setMasterCore(pin.no, masterCore);
}





} // namespace mcu



