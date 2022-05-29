/**
 * @file
 * @ingroup mcu mcu_cap
 */


#include "mcucap.h"


namespace mcu {

CapModuleImpl CapUnit::module[3] =
{
	{
		.base = ECAP1_BASE,
		.xbarInput = XBAR_INPUT7,
		.pieIntNo = INT_ECAP1,
	},
	{
		.base = ECAP2_BASE,
		.xbarInput = XBAR_INPUT8,
		.pieIntNo = INT_ECAP2,
	},
	{
		.base = ECAP3_BASE,
		.xbarInput = XBAR_INPUT9,
		.pieIntNo = INT_ECAP3,
	}
};

///
///
///
CapUnit::CapUnit(const emb::Array<mcu::GpioPin, 3>& pins)
	: emb::c28x::Singleton<CapUnit>(this)
{
	// XBAR setup
	for (size_t i = 0; i < 3; ++i)
	{
		module[i].inputPin = pins[i].no;
		module[i].inputPinMux = pins[i].mux;
		XBAR_setInputPin(module[i].xbarInput, module[i].inputPin);
	}

	// GPIO setup
	for (size_t i = 0; i < 3; ++i)
	{
		initGpioPin(pins[i]);
	}

	/* ECAP setup */
	for (size_t i = 0; i < 3; ++i)
	{
		ECAP_disableInterrupt(module[i].base,
							  (ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
							   ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
							   ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
							   ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
							   ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
							   ECAP_ISR_SOURCE_COUNTER_PERIOD   |
							   ECAP_ISR_SOURCE_COUNTER_COMPARE));
		ECAP_clearInterrupt(module[i].base,
							(ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
							 ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
							 ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
							 ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
							 ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
							 ECAP_ISR_SOURCE_COUNTER_PERIOD   |
							 ECAP_ISR_SOURCE_COUNTER_COMPARE));
		ECAP_disableTimeStampCapture(module[i].base);

		ECAP_stopCounter(module[i].base);
		ECAP_enableCaptureMode(module[i].base);
		ECAP_setCaptureMode(module[i].base, ECAP_CONTINUOUS_CAPTURE_MODE, ECAP_EVENT_2);


		ECAP_setEventPolarity(module[i].base, ECAP_EVENT_1, ECAP_EVNT_RISING_EDGE);
		ECAP_setEventPolarity(module[i].base, ECAP_EVENT_2, ECAP_EVNT_FALLING_EDGE);

		ECAP_enableCounterResetOnEvent(module[i].base, ECAP_EVENT_1);
		ECAP_enableCounterResetOnEvent(module[i].base, ECAP_EVENT_2);

		ECAP_setSyncOutMode(module[i].base, ECAP_SYNC_OUT_DISABLED);
		ECAP_startCounter(module[i].base);
		ECAP_enableTimeStampCapture(module[i].base);
		ECAP_reArm(module[i].base);
	}
}


} // namespace mcu


