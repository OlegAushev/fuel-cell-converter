/**
 * @file
 * @ingroup mcu mcu_i2c
 */


#include "mcui2c.h"

namespace mcu {

namespace detail {
const uint32_t i2cSdaPins[3] = {32, 104, 40};
const uint32_t i2cSdaPinMuxs[3] = {GPIO_32_SDAA, GPIO_104_SDAA, GPIO_40_SDAB};
const uint32_t i2cSclPins[3] = {33, 105, 41};
const uint32_t i2cSclPinMuxs[3] = {GPIO_33_SCLA, GPIO_105_SCLA, GPIO_41_SCLB};
}

I2CModuleImpl I2CUnit<I2CA>::module = {.base = I2CA_BASE};
I2CModuleImpl I2CUnit<I2CB>::module = {.base = I2CB_BASE};


///
///
///
template <I2CModule Module>
I2CUnit<Module>::I2CUnit(I2CSdaPin sdaPin, I2CSclPin sclPin, const I2CConfig& cfg)
	: emb::c28x::Singleton<I2CUnit<Module> >(this)
{
	module.sdaPin = detail::i2cSdaPins[sdaPin];
	module.sdaPinMux = detail::i2cSdaPinMuxs[sdaPin];
	module.sclPin = detail::i2cSclPins[sclPin];
	module.sclPinMux = detail::i2cSclPinMuxs[sclPin];

#ifdef CPU1
	GPIO_setPadConfig(module.sdaPin, GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(module.sdaPin, GPIO_QUAL_ASYNC);
	GPIO_setPinConfig(module.sdaPinMux);

	GPIO_setPadConfig(module.sclPin, GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(module.sclPin, GPIO_QUAL_ASYNC);
	GPIO_setPinConfig(module.sclPinMux);
#endif
	I2C_disableModule(module.base);

	I2C_initMaster(module.base, DEVICE_SYSCLK_FREQ, cfg.bitrate, static_cast<I2C_DutyCycle>(cfg.dutyCycle));
	I2C_setBitCount(module.base, static_cast<I2C_BitCount>(cfg.bitCount));
	I2C_setSlaveAddress(module.base, cfg.slaveAddr);
	I2C_setEmulationMode(module.base, I2C_EMULATION_FREE_RUN);

	I2C_disableFIFO(module.base);
	I2C_enableModule(module.base);
}
// Explicit instantiation
template I2CUnit<I2CA>::I2CUnit(I2CSdaPin sdaPin, I2CSclPin sclPin, const I2CConfig& cfg);
template I2CUnit<I2CB>::I2CUnit(I2CSdaPin sdaPin, I2CSclPin sclPin, const I2CConfig& cfg);


#ifdef CPU1
///
///
///
template <I2CModule Module>
void I2CUnit<Module>::transferControlToCpu2(I2CSdaPin sdaPin, I2CSclPin sclPin)
{
	GPIO_setPadConfig(detail::i2cSdaPins[sdaPin], GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(detail::i2cSdaPins[sdaPin], GPIO_QUAL_ASYNC);
	GPIO_setPinConfig(detail::i2cSdaPinMuxs[sdaPin]);
	GPIO_setMasterCore(detail::i2cSdaPins[sdaPin], GPIO_CORE_CPU2);

	GPIO_setPadConfig(detail::i2cSclPins[sclPin], GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(detail::i2cSclPins[sclPin], GPIO_QUAL_ASYNC);
	GPIO_setPinConfig(detail::i2cSclPinMuxs[sclPin]);
	GPIO_setMasterCore(detail::i2cSclPins[sclPin], GPIO_CORE_CPU2);

	SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL7_I2C, static_cast<uint16_t>(Module)+1, SYSCTL_CPUSEL_CPU2);
}
// Explicit instantiation
template void I2CUnit<I2CA>::transferControlToCpu2(I2CSdaPin sdaPin, I2CSclPin sclPin);
template void I2CUnit<I2CB>::transferControlToCpu2(I2CSdaPin sdaPin, I2CSclPin sclPin);
#endif




} /* namespace mcu */


