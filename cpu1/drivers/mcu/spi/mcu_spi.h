/**
 * @defgroup mcu_spi SPI
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_spi
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "../gpio/mcu_gpio.h"
#include "emb/emb_common.h"


namespace mcu {
/// @addtogroup mcu_spi
/// @{


/// SPI modules
enum SpiModule
{
	SPIA,
	SPIB,
	SPIC
};


/// SPI protocol
enum SpiProtocol
{
	SPI_PROTOCOL_POL0PHA0 = SPI_PROT_POL0PHA0,	//!< Mode 0. Polarity 0, phase 0. Rising edge without delay
	SPI_PROTOCOL_POL0PHA1 =  SPI_PROT_POL0PHA1,	//!< Mode 1. Polarity 0, phase 1. Rising edge with delay.
	SPI_PROTOCOL_POL1PHA0 = SPI_PROT_POL1PHA0,	//!< Mode 2. Polarity 1, phase 0. Falling edge without delay.
	SPI_PROTOCOL_POL1PHA1 = SPI_PROT_POL1PHA1	//!< Mode 3. Polarity 1, phase 1. Falling edge with delay.
};


/// SPI mode
enum SpiMode
{
	SPI_MODE_SLAVE = ::SPI_MODE_SLAVE,		//!< SPI slave
	SPI_MODE_MASTER = ::SPI_MODE_MASTER,		//!< SPI master
	SPI_MODE_SLAVE_OD = ::SPI_MODE_SLAVE_OD,	//!< SPI slave w/ output (TALK) disabled
	SPI_MODE_MASTER_OD = ::SPI_MODE_MASTER_OD	//!< SPI master w/ output (TALK) disabled
};


/// SPI bitrates
enum SpiBitrate
{
	SPI_BITRATE_1M = 1000000,
	SPI_BITRATE_12M5 = 12500000,
};


/// SPI word length
enum SpiWordLen
{
	SPI_WORD_8BIT = 8,
	SPI_WORD_16BIT = 16
};


/**
 * @brief SPI unit config.
 */
struct SpiConfig
{
	SpiProtocol protocol;
	SpiMode mode;
	SpiBitrate bitrate;
	SpiWordLen wordLen;
	uint16_t dataSize;
};


namespace detail {


/**
 * @brief SPI module implementation.
 */
struct SpiModuleImpl
{
	uint32_t base;
	uint32_t pieRxIntNum;
	SpiModuleImpl(uint32_t _base, uint32_t _pieRxIntNum)
		: base(_base), pieRxIntNum(_pieRxIntNum) {}
};


extern const uint32_t spiBases[3];
extern const uint32_t spiRxPieIntNums[3];


} // namespace detail


/**
 * @brief SPI unit class.
 */
template <SpiModule Module>
class Spi : public emb::c28x::Singleton<Spi<Module> >
{
private:
	detail::SpiModuleImpl m_module;
	SpiWordLen m_wordLen;

private:
	Spi(const Spi& other);			// no copy constructor
	Spi& operator=(const Spi& other);	// no copy assignment operator
public:
	/**
	 * @brief Initializes MCU SPI unit.
	 * @param mosiPin
	 * @param misoPin
	 * @param clkPin
	 * @param csPin
	 * @param cfg
	 */
	Spi(const GpioConfig& mosiPin, const GpioConfig& misoPin,
			const GpioConfig& clkPin, const GpioConfig& csPin,
			const SpiConfig& cfg)
		: emb::c28x::Singleton<Spi<Module> >(this)
		, m_module(detail::spiBases[Module], detail::spiRxPieIntNums[Module])
	{
		assert((cfg.dataSize >= 1) && (cfg.dataSize <= 16));

		m_wordLen = cfg.wordLen;

		SPI_disableModule(m_module.base);
		SPI_setConfig(m_module.base, DEVICE_LSPCLK_FREQ,
				static_cast<SPI_TransferProtocol>(cfg.protocol),
				static_cast<SPI_Mode>(cfg.mode),
				static_cast<uint32_t>(cfg.bitrate), static_cast<uint16_t>(cfg.wordLen));
		SPI_disableLoopback(m_module.base);
		SPI_setEmulationMode(m_module.base, SPI_EMULATION_FREE_RUN);

#ifdef CPU1
		_initPins(mosiPin, misoPin, clkPin, csPin);
#endif

		SPI_enableFIFO(m_module.base);
		SPI_setFIFOInterruptLevel(m_module.base, SPI_FIFO_TXEMPTY, static_cast<SPI_RxFIFOLevel>(cfg.dataSize));
		SPI_enableModule(m_module.base);
	}

#ifdef CPU1
	/**
	 * @brief Transfers control over SPI unit to CPU2.
	 * @param mosiPin
	 * @param misoPin
	 * @param clkPin
	 * @param csPin
	 * @param csMode
	 * @return (none)
	 */
	static void transferControlToCpu2(const GpioConfig& mosiPin, const GpioConfig& misoPin,
			const GpioConfig& clkPin, const GpioConfig& csPin)
	{
		_initPins(mosiPin, misoPin, clkPin, csPin);
		GPIO_setMasterCore(mosiPin.no, GPIO_CORE_CPU2);
		GPIO_setMasterCore(misoPin.no, GPIO_CORE_CPU2);
		GPIO_setMasterCore(clkPin.no, GPIO_CORE_CPU2);
		if (csPin.valid)
		{
			GPIO_setMasterCore(csPin.no, GPIO_CORE_CPU2);
		}

		SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL6_SPI,
				static_cast<uint16_t>(Module)+1, SYSCTL_CPUSEL_CPU2);
	}
#endif

	/**
	 * @brief Returns base of SPI-unit.
	 * @param (none)
	 * @return Base of SPI-unit.
	 */
	uint32_t base() const { return m_module.base; }

	/**
	 * @brief Enables loopback mode.
	 * @param (none)
	 * @return (none)
	 */
	void enableLoopback() const
	{
		SPI_disableModule(m_module.base);
		SPI_enableLoopback(m_module.base);
		SPI_enableModule(m_module.base);
	}

	/**
	 * @brief Retrieves received data.
	 * @param data - reference to destination data
	 * @return (none)
	 */
	template <typename T>
	void recv(T& data) const
	{
		switch (m_wordLen)
		{
		case SPI_WORD_8BIT:
			uint16_t byte8[sizeof(T)*2];
			for (size_t i = 0; i < sizeof(T)*2; ++i)
			{
				byte8[i] = SPI_readDataBlockingFIFO(m_module.base) & 0x00FF;
			}
			emb::c28x::from_bytes8<T>(data, byte8);
			break;

		case SPI_WORD_16BIT:
			uint16_t byte16[sizeof(T)];
			for (size_t i = 0; i < sizeof(T); ++i)
			{
				byte16[i] = SPI_readDataBlockingFIFO(m_module.base);
			}
			memcpy(&data, byte16, sizeof(T));
			break;
		}
	}

	/**
	 * @brief Sends data.
	 * @param data - reference to source data
	 * @return (none)
	 */
	template <typename T>
	void send(const T& data) const
	{
		switch (m_wordLen)
		{
		case SPI_WORD_8BIT:
			uint16_t byte8[sizeof(T)*2];
			emb::c28x::to_bytes8<T>(byte8, data);
			for (size_t i = 0; i < sizeof(T)*2; ++i)
			{
				SPI_writeDataBlockingFIFO(m_module.base, byte8[i] << 8);
			}
			break;

		case SPI_WORD_16BIT:
			uint16_t byte16[sizeof(T)];
			memcpy(byte16, &data, sizeof(T));
			for (size_t i = 0; i < sizeof(T); ++i)
			{
				SPI_writeDataBlockingFIFO(m_module.base, byte16[i]);
			}
			break;
		}
	}

	/**
	 * @brief Registers Rx-interrupt handler.
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	void registerRxInterruptHandler(void (*handler)(void)) const
	{
		SPI_disableModule(m_module.base);
		Interrupt_register(m_module.pieRxIntNum, handler);
		SPI_enableInterrupt(m_module.base, SPI_INT_RXFF);
		SPI_enableModule(m_module.base);
	}

	/**
	 * @brief Enables Rx-interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableRxInterrupts() const
	{
		Interrupt_enable(m_module.pieRxIntNum);
	}

	/**
	 * @brief Disables Rx-interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableRxInterrupts() const
	{
		Interrupt_disable(m_module.pieRxIntNum);
	}

	/**
	 * @brief Acknowledges interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeRxInterrupt() const
	{
		SPI_clearInterruptStatus(m_module.base, SPI_INT_RXFF);
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
	}

	/**
	 * @brief Resets RX FIFO.
	 * @param (none)
	 * @return (none)
	 */
	void resetRxFifo() const
	{
		SPI_resetRxFIFO(m_module.base);
	}

	/**
	 * @brief Resets TX FIFO.
	 * @param (none)
	 * @return (none)
	 */
	void resetTxFifo() const
	{
		SPI_resetTxFIFO(m_module.base);
	}

protected:
	static void _initPins(const GpioConfig& mosiPin, const GpioConfig& misoPin,
			const GpioConfig& clkPin, const GpioConfig& csPin)
	{
		GPIO_setPinConfig(mosiPin.mux);
		//GPIO_setPadConfig(mosiPin.no, GPIO_PIN_TYPE_PULLUP);
		GPIO_setQualificationMode(mosiPin.no, GPIO_QUAL_ASYNC);

		GPIO_setPinConfig(misoPin.mux);
		//GPIO_setPadConfig(misoPin.no, GPIO_PIN_TYPE_PULLUP);
		GPIO_setQualificationMode(misoPin.no, GPIO_QUAL_ASYNC);

		GPIO_setPinConfig(clkPin.mux);
		//GPIO_setPadConfig(clkPin.no, GPIO_PIN_TYPE_PULLUP);
		GPIO_setQualificationMode(clkPin.no, GPIO_QUAL_ASYNC);

		if (csPin.valid)
		{
			GPIO_setPinConfig(csPin.mux);
			//GPIO_setPadConfig(csPin.no, GPIO_PIN_TYPE_PULLUP);
			GPIO_setQualificationMode(csPin.no, GPIO_QUAL_ASYNC);
		}
	}
};


/// @}
} // namespace mcu


