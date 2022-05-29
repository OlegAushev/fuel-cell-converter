/**
 * @file
 * @ingroup mcu mcu_spi
 */


#include "mcuspi.h"


namespace mcu {

namespace detail {
const uint32_t spiMosiPins[3] = {16, 58, 63};
const uint32_t spiMosiPinMuxs[3] = {GPIO_16_SPISIMOA, GPIO_58_SPISIMOA, GPIO_63_SPISIMOB};
const uint32_t spiMisoPins[3] = {17, 59, 64};
const uint32_t spiMisoPinMuxs[3] = {GPIO_17_SPISOMIA, GPIO_59_SPISOMIA, GPIO_64_SPISOMIB};
const uint32_t spiClkPins[3] = {18, 60, 65};
const uint32_t spiClkPinMuxs[3] = {GPIO_18_SPICLKA, GPIO_60_SPICLKA, GPIO_65_SPICLKB};
const uint32_t spiCsPins[4] = {0, 19, 61, 66};
const uint32_t spiCsPinMuxs[4] = {0, GPIO_19_SPISTEA, GPIO_61_SPISTEA, GPIO_66_SPISTEB};
}

SpiModuleImpl SpiUnit<SPIA>::module =
{
	.base = SPIA_BASE,
	.rxPieIntNo = INT_SPIA_RX,
};

SpiModuleImpl SpiUnit<SPIB>::module =
{
	.base = SPIB_BASE,
	.rxPieIntNo = INT_SPIB_RX,
};

SpiModuleImpl SpiUnit<SPIC>::module =
{
	.base = SPIC_BASE,
	.rxPieIntNo = INT_SPIC_RX,
};

SpiWordLen SpiUnit<SPIA>::m_wordLen;
SpiWordLen SpiUnit<SPIB>::m_wordLen;
SpiWordLen SpiUnit<SPIC>::m_wordLen;

///
///
///
template <SpiModule Module>
SpiUnit<Module>::SpiUnit(SpiMosiPin mosiPin, SpiMisoPin misoPin, SpiClkPin clkPin, SpiCsPin csPin,
		const SpiConfig& cfg)
	: emb::c28x::Singleton<SpiUnit<Module> >(this)
{
	ASSERT((cfg.dataSize >= 1) && (cfg.dataSize <= 16));

	m_wordLen = cfg.wordLen;

	SPI_disableModule(module.base);
	SPI_setConfig(module.base, DEVICE_LSPCLK_FREQ,
			static_cast<SPI_TransferProtocol>(cfg.protocol),
			static_cast<SPI_Mode>(cfg.mode),
			static_cast<uint32_t>(cfg.bitrate), static_cast<uint16_t>(cfg.wordLen));
	SPI_disableLoopback(module.base);
	SPI_setEmulationMode(module.base, SPI_EMULATION_FREE_RUN);

	initPins(mosiPin, misoPin, clkPin, csPin);

	SPI_enableFIFO(module.base);
	SPI_setFIFOInterruptLevel(module.base, SPI_FIFO_TXEMPTY, static_cast<SPI_RxFIFOLevel>(cfg.dataSize));
	SPI_enableModule(module.base);
}
// Explicit instantiation
template SpiUnit<SPIA>::SpiUnit(SpiMosiPin mosiPin, SpiMisoPin misoPin, SpiClkPin clkPin, SpiCsPin csPin,
		const SpiConfig& cfg);
template SpiUnit<SPIB>::SpiUnit(SpiMosiPin mosiPin, SpiMisoPin misoPin, SpiClkPin clkPin, SpiCsPin csPin,
		const SpiConfig& cfg);
template SpiUnit<SPIC>::SpiUnit(SpiMosiPin mosiPin, SpiMisoPin misoPin, SpiClkPin clkPin, SpiCsPin csPin,
		const SpiConfig& cfg);

#ifdef CPU1
///
///
///
template <SpiModule Module>
void SpiUnit<Module>::transferControlToCpu2(SpiMosiPin mosiPin, SpiMisoPin misoPin, SpiClkPin clkPin, SpiCsPin csPin)
{
	initPins(mosiPin, misoPin, clkPin, csPin);
	GPIO_setMasterCore(detail::spiMosiPins[mosiPin], GPIO_CORE_CPU2);
	GPIO_setMasterCore(detail::spiMisoPins[misoPin], GPIO_CORE_CPU2);
	GPIO_setMasterCore(detail::spiClkPins[clkPin], GPIO_CORE_CPU2);
	if (csPin != SPI_CS_SOFTWARE)
	{
		GPIO_setMasterCore(detail::spiCsPins[csPin], GPIO_CORE_CPU2);
	}

	SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL6_SPI,
			static_cast<uint16_t>(Module)+1, SYSCTL_CPUSEL_CPU2);
}
// Explicit instantiation
template void SpiUnit<SPIA>::transferControlToCpu2(SpiMosiPin mosiPin, SpiMisoPin misoPin,
		SpiClkPin clkPin, SpiCsPin csPin);
template void SpiUnit<SPIB>::transferControlToCpu2(SpiMosiPin mosiPin, SpiMisoPin misoPin,
		SpiClkPin clkPin, SpiCsPin csPin);
template void SpiUnit<SPIC>::transferControlToCpu2(SpiMosiPin mosiPin, SpiMisoPin misoPin,
		SpiClkPin clkPin, SpiCsPin csPin);
#endif

///
///
///
template <SpiModule Module>
void SpiUnit<Module>::initPins(SpiMosiPin mosiPin, SpiMisoPin misoPin, SpiClkPin clkPin, SpiCsPin csPin)
{
	module.mosiPin = detail::spiMosiPins[mosiPin];
	module.mosiPinMux = detail::spiMosiPinMuxs[mosiPin];

	module.misoPin = detail::spiMisoPins[misoPin];
	module.misoPinMux = detail::spiMisoPinMuxs[misoPin];

	module.clkPin = detail::spiClkPins[clkPin];
	module.clkPinMux = detail::spiClkPinMuxs[clkPin];

	module.csPin = detail::spiCsPins[csPin];
	module.csPinMux = detail::spiCsPinMuxs[csPin];

#ifdef CPU1
	// GPIO initialization
	GPIO_setPinConfig(module.mosiPinMux);
	//GPIO_setPadConfig(module.mosiPin, GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(module.mosiPin, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(module.misoPinMux);
	//GPIO_setPadConfig(module.misoPin, GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(module.misoPin, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(module.clkPinMux);
	//GPIO_setPadConfig(module.clkPin, GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(module.clkPin, GPIO_QUAL_ASYNC);

	if (csPin != SPI_CS_SOFTWARE)
	{
		GPIO_setPinConfig(module.csPinMux);
		//GPIO_setPadConfig(module.csPin, GPIO_PIN_TYPE_PULLUP);
		GPIO_setQualificationMode(module.csPin, GPIO_QUAL_ASYNC);
	}
#endif
}
// Explicit instantiation
template void SpiUnit<SPIA>::initPins(SpiMosiPin mosiPin, SpiMisoPin misoPin, SpiClkPin clkPin, SpiCsPin csPin);
template void SpiUnit<SPIB>::initPins(SpiMosiPin mosiPin, SpiMisoPin misoPin, SpiClkPin clkPin, SpiCsPin csPin);
template void SpiUnit<SPIC>::initPins(SpiMosiPin mosiPin, SpiMisoPin misoPin, SpiClkPin clkPin, SpiCsPin csPin);








} /* namespace mcu */


