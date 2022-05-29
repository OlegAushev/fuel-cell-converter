/**
 * @file
 * @ingroup mcu mcu_can
 */


#include "mcucan.h"


namespace mcu {

namespace detail {
const uint32_t canTxPins[3] = {19, 31, 12};
const uint32_t canTxPinMuxs[3] = {GPIO_19_CANTXA, GPIO_31_CANTXA, GPIO_12_CANTXB};
const uint32_t canRxPins[3] = {18, 30, 17};
const uint32_t canRxPinMuxs[3] = {GPIO_18_CANRXA, GPIO_30_CANRXA, GPIO_17_CANRXB};
}

CanModuleImpl CanUnit<CANA>::module =
{
	.base = CANA_BASE,
	.pieIntNo = INT_CANA0,
};

CanModuleImpl CanUnit<CANB>::module =
{
	.base = CANB_BASE,
	.pieIntNo = INT_CANB0,
};

///
///
///
template <CanModule Module>
CanUnit<Module>::CanUnit(CanTxPin txPin, CanRxPin rxPin, CanBitrate bitrate)
	: emb::c28x::Singleton<CanUnit<Module> >(this)
{
	module.txPin = detail::canTxPins[txPin];
	module.txPinMux = detail::canTxPinMuxs[txPin];
	module.rxPin = detail::canRxPins[rxPin];
	module.rxPinMux = detail::canRxPinMuxs[rxPin];

#ifdef CPU1
	GPIO_setPinConfig(module.rxPinMux);
	GPIO_setPinConfig(module.txPinMux);
#endif

	CAN_initModule(module.base);
	CAN_selectClockSource(module.base, CAN_CLOCK_SOURCE_SYS);

	switch (bitrate)
	{
	case CAN_BITRATE_125K:
	case CAN_BITRATE_500K:
		CAN_setBitRate(module.base, DEVICE_SYSCLK_FREQ, static_cast<uint32_t>(bitrate), 16);
		break;
	case CAN_BITRATE_1M:
		CAN_setBitRate(module.base, DEVICE_SYSCLK_FREQ, static_cast<uint32_t>(bitrate), 10);
		break;
	}

	CAN_setAutoBusOnTime(module.base, 0);
	CAN_enableAutoBusOn(module.base);

	CAN_startModule(module.base);
}
// Explicit instantiation
template CanUnit<CANA>::CanUnit(CanTxPin txPin, CanRxPin rxPin, CanBitrate bitrate);
template CanUnit<CANB>::CanUnit(CanTxPin txPin, CanRxPin rxPin, CanBitrate bitrate);


#ifdef CPU1
///
///
///
template <CanModule Module>
void CanUnit<Module>::transferControlToCpu2(CanTxPin txPin, CanRxPin rxPin)
{

	GPIO_setPinConfig(detail::canTxPinMuxs[txPin]);
	GPIO_setMasterCore(detail::canTxPins[txPin], GPIO_CORE_CPU2);
	GPIO_setPinConfig(detail::canRxPinMuxs[rxPin]);
	GPIO_setMasterCore(detail::canRxPins[rxPin], GPIO_CORE_CPU2);

	SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL8_CAN,
			static_cast<uint16_t>(Module)+1, SYSCTL_CPUSEL_CPU2);
}
// Explicit instantiation
template void CanUnit<CANA>::transferControlToCpu2(CanTxPin txPin, CanRxPin rxPin);
template void CanUnit<CANB>::transferControlToCpu2(CanTxPin txPin, CanRxPin rxPin);
#endif

} /* namespace mcu */


