/**
 * @file
 * @ingroup crd300
 */


#ifdef CRD600


#include "controller.h"


namespace crd600 {


const emb::Array<mcu::GpioPinConfig, 6> Controller::PSDIS_PINS_CONFIG =
{
	mcu::GpioPinConfig(79, GPIO_79_GPIO79, mcu::PIN_OUTPUT, mcu::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(87, GPIO_87_GPIO87, mcu::PIN_OUTPUT, mcu::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(133, GPIO_133_GPIO133, mcu::PIN_OUTPUT, mcu::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(78, GPIO_78_GPIO78, mcu::PIN_OUTPUT, mcu::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(86, GPIO_86_GPIO86, mcu::PIN_OUTPUT, mcu::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(94, GPIO_94_GPIO94, mcu::PIN_OUTPUT, mcu::ACTIVE_LOW, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1)
};


const emb::Array<mcu::GpioPinConfig, 6> Controller::LEN_PINS_CONFIG =
{
	mcu::GpioPinConfig(77, GPIO_77_GPIO77, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(85, GPIO_85_GPIO85, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(93, GPIO_93_GPIO93, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(76, GPIO_76_GPIO76, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(84, GPIO_84_GPIO84, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(92, GPIO_92_GPIO92, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
};


const emb::Array<mcu::GpioPinConfig, 6> Controller::OCEN_PINS_CONFIG =
{
	mcu::GpioPinConfig(75, GPIO_75_GPIO75, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(83, GPIO_83_GPIO83, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(91, GPIO_91_GPIO91, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(74, GPIO_74_GPIO74, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(82, GPIO_82_GPIO82, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
	mcu::GpioPinConfig(90, GPIO_90_GPIO90, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1),
};


const emb::Array<mcu::GpioPinConfig, 6> Controller::FAULT_PINS_CONFIG =
{
	mcu::GpioPinConfig(36, GPIO_36_GPIO36, mcu::PIN_INPUT, mcu::ACTIVE_LOW, mcu::PIN_PULLUP, mcu::PIN_QUAL_ASYNC, 1),
	mcu::GpioPinConfig(38, GPIO_38_GPIO38, mcu::PIN_INPUT, mcu::ACTIVE_LOW, mcu::PIN_PULLUP, mcu::PIN_QUAL_ASYNC, 1),
	mcu::GpioPinConfig(61, GPIO_61_GPIO61, mcu::PIN_INPUT, mcu::ACTIVE_LOW, mcu::PIN_PULLUP, mcu::PIN_QUAL_ASYNC, 1),
	mcu::GpioPinConfig(35, GPIO_35_GPIO35, mcu::PIN_INPUT, mcu::ACTIVE_LOW, mcu::PIN_PULLUP, mcu::PIN_QUAL_ASYNC, 1),
	mcu::GpioPinConfig(37, GPIO_37_GPIO37, mcu::PIN_INPUT, mcu::ACTIVE_LOW, mcu::PIN_PULLUP, mcu::PIN_QUAL_ASYNC, 1),
	mcu::GpioPinConfig(60, GPIO_60_GPIO60, mcu::PIN_INPUT, mcu::ACTIVE_LOW, mcu::PIN_PULLUP, mcu::PIN_QUAL_ASYNC, 1),
};


const mcu::GpioPinConfig Controller::FAULT_PIN_UVW_CONFIG =
		mcu::GpioPinConfig(65, GPIO_65_GPIO65, mcu::PIN_INPUT, mcu::ACTIVE_LOW, mcu::PIN_PULLUP, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioPinConfig Controller::FAULT_PIN_XYZ_CONFIG =
		mcu::GpioPinConfig(67, GPIO_67_GPIO67, mcu::PIN_INPUT, mcu::ACTIVE_LOW, mcu::PIN_PULLUP, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioPinConfig Controller::FAULT_PIN_ALL_CONFIG=
		mcu::GpioPinConfig(69, GPIO_69_GPIO69, mcu::PIN_INPUT, mcu::ACTIVE_LOW, mcu::PIN_PULLUP, mcu::PIN_QUAL_ASYNC, 1);


const mcu::GpioPinConfig Controller::SHUTDOWN_POS15V_PIN_CONFIG =
	mcu::GpioPinConfig(88, GPIO_88_GPIO88, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_SYNC, 1);


const emb::Array<mcu::GpioPinConfig, 3> Controller::LED_PINS_CONFIG =
{
	mcu::GpioPinConfig(64, GPIO_64_GPIO64, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1),
	mcu::GpioPinConfig(66, GPIO_66_GPIO66, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1),
	mcu::GpioPinConfig(68, GPIO_68_GPIO68, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1),
};


#ifdef USE_SD_PINS_FOR_PROFILING
const mcu::GpioPinConfig Controller::PROFILER_PIN_C1_CONFIG =
	mcu::GpioPinConfig(49, GPIO_49_GPIO49, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioPinConfig Controller::PROFILER_PIN_C2_CONFIG =
	mcu::GpioPinConfig(51, GPIO_51_GPIO51, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioPinConfig Controller::PROFILER_PIN_C3_CONFIG =
	mcu::GpioPinConfig(53, GPIO_53_GPIO53, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioPinConfig Controller::PROFILER_PIN_D1_CONFIG =
	mcu::GpioPinConfig(48, GPIO_48_GPIO48, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
//const mcu::GpioPinConfig Controller::PROFILER_PIN_D2_CONFIG =
//	mcu::GpioPinConfig(50, GPIO_50_GPIO50, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
const mcu::GpioPinConfig Controller::PROFILER_PIN_D3_CONFIG =
	mcu::GpioPinConfig(52, GPIO_52_GPIO52, mcu::PIN_OUTPUT, mcu::ACTIVE_HIGH, mcu::PIN_STD, mcu::PIN_QUAL_ASYNC, 1);
#endif


///
///
///
Controller::Controller()
	: emb::c28x::Singleton<Controller>(this)
{
	for (size_t i = 0; i < PHASE_COUNT; ++i)
	{
		psdisPins[i].init(PSDIS_PINS_CONFIG[i]);
		lenPins[i].init(LEN_PINS_CONFIG[i]);
		ocenPins[i].init(OCEN_PINS_CONFIG[i]);
		faultPins[i].init(FAULT_PINS_CONFIG[i]);
	}

	faultPinUVW.init(FAULT_PIN_UVW_CONFIG);
	faultPinXYZ.init(FAULT_PIN_XYZ_CONFIG);
	faultPinALL.init(FAULT_PIN_ALL_CONFIG);
	shutdownPos15VPin.init(SHUTDOWN_POS15V_PIN_CONFIG);

	for (size_t i = 0; i < 3; ++i)
	{
		ledPins[i].init(LED_PINS_CONFIG[i]);
	}

#ifdef USE_SD_PINS_FOR_PROFILING
	profilerPinC1.init(PROFILER_PIN_C1_CONFIG);
	profilerPinC2.init(PROFILER_PIN_C2_CONFIG);
	profilerPinC3.init(PROFILER_PIN_C3_CONFIG);
	profilerPinD1.init(PROFILER_PIN_D1_CONFIG);
#warning "D2 Pin used for QEP"
//	profilerPinD2.init(PROFILER_PIN_D2_CONFIG);
	profilerPinD3.init(PROFILER_PIN_D3_CONFIG);
#endif
}


///
///
///
void Controller::resetAllDrivers()
{
	for (size_t i = 0; i < PHASE_COUNT; ++i)
	{
		ocenPins[i].set(mcu::PIN_ACTIVE);
	}
	mcu::delay_us(100);
	for (size_t i = 0; i < PHASE_COUNT; ++i)
	{
		ocenPins[i].set(mcu::PIN_INACTIVE);
	}
	mcu::delay_us(10000);
}


} // namespace crd600


#endif // #ifdef CRD600


