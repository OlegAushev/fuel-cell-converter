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

/// SPI MOSI pins
enum SpiMosiPin
{
	SPIA_MOSI_GPIO_16,
	SPIA_MOSI_GPIO_58,
	SPIB_MOSI_GPIO_63,
};

/// SPI MISO pins
enum SpiMisoPin
{
	SPIA_MISO_GPIO_17,
	SPIA_MISO_GPIO_59,
	SPIB_MISO_GPIO_64,
};

/// SPI CLK pins
enum SpiClkPin
{
	SPIA_CLK_GPIO_18,
	SPIA_CLK_GPIO_60,
	SPIB_CLK_GPIO_65,
};

/// SPI CS pins
enum SpiCsPin
{
	SPI_CS_SOFTWARE,
	SPIA_CS_GPIO_19,
	SPIA_CS_GPIO_61,
	SPIB_CS_GPIO_66,
};

/// SPI protocol
enum SpiProtocol
{
	SPI_PROTOCOL_POL0PHA0 = SPI_PROT_POL0PHA0,	//! Mode 0. Polarity 0, phase 0. Rising edge without delay
	SPI_PROTOCOL_POL0PHA1 =  SPI_PROT_POL0PHA1,	//! Mode 1. Polarity 0, phase 1. Rising edge with delay.
	SPI_PROTOCOL_POL1PHA0 = SPI_PROT_POL1PHA0,	//! Mode 2. Polarity 1, phase 0. Falling edge without delay.
	SPI_PROTOCOL_POL1PHA1 = SPI_PROT_POL1PHA1	//! Mode 3. Polarity 1, phase 1. Falling edge with delay.
};

/// SPI mode
enum SpiMode
{
	SPI_MODE_SLAVE = SPI_MODE_SLAVE,		//!< SPI slave
	SPI_MODE_MASTER = ::SPI_MODE_MASTER,		//!< SPI master
	SPI_MODE_SLAVE_OD = ::SPI_MODE_SLAVE_OD,	//!< SPI slave w/ output (TALK) disabled
	SPI_MODE_MASTER_OD = ::SPI_MODE_MASTER_OD	//!< SPI master w/ output (TALK) disabled
};

/// SPI bitrates
enum SpiBitrate
{
	SPI_BITRATE_1M = 1000000,   ///< SPI_BITRATE_1M
	SPI_BITRATE_12M5 = 12500000,///< SPI_BITRATE_12M5
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

/**
 * @brief SPI module implementation.
 */
struct SpiModuleImpl
{
	const uint32_t base;
	uint32_t mosiPin;
	uint32_t mosiPinMux;
	uint32_t misoPin;
	uint32_t misoPinMux;
	uint32_t clkPin;
	uint32_t clkPinMux;
	uint32_t csPin;
	uint32_t csPinMux;
	const uint32_t rxPieIntNo;
};

/**
 * @brief SPI unit class.
 */
template <SpiModule Module>
class SpiUnit : public emb::c28x::Singleton<SpiUnit<Module> >
{
private:
	static SpiWordLen m_wordLen;

	SpiUnit(const SpiUnit& other);			// no copy constructor
	SpiUnit& operator=(const SpiUnit& other);	// no copy assignment operator

public:
	/// SPI module
	static SpiModuleImpl module;

	/**
	 * @brief Initializes MCU SPI unit.
	 * @param mosiPin
	 * @param misoPin
	 * @param clkPin
	 * @param csPin
	 * @param cfg
	 */
	SpiUnit(SpiMosiPin mosiPin, SpiMisoPin misoPin, SpiClkPin clkPin, SpiCsPin csPin, const SpiConfig& cfg);

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
	static void transferControlToCpu2(SpiMosiPin mosiPin, SpiMisoPin misoPin, SpiClkPin clkPin, SpiCsPin csPin);
#endif

private:
	/**
	 * @brief Initializes SPI unit pins.
	 * @param mosiPin
	 * @param misoPin
	 * @param clkPin
	 * @param csPin
	 * @param csMode
	 * @return (none)
	 */
	static void initPins(SpiMosiPin mosiPin, SpiMisoPin misoPin, SpiClkPin clkPin, SpiCsPin csPin);

public:
	/**
	 * @brief Enables loopback mode.
	 * @param (none)
	 * @return (none)
	 */
	void enableLoopback() const
	{
		SPI_disableModule(module.base);
		SPI_enableLoopback(module.base);
		SPI_enableModule(module.base);
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
				byte8[i] = SPI_readDataBlockingFIFO(module.base) & 0x00FF;
			}
			emb::c28x::from_8bit_bytes<T>(data, byte8);
			break;

		case SPI_WORD_16BIT:
			uint16_t byte16[sizeof(T)];
			for (size_t i = 0; i < sizeof(T); ++i)
			{
				byte16[i] = SPI_readDataBlockingFIFO(module.base);
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
			emb::c28x::to_8bit_bytes<T>(byte8, data);
			for (size_t i = 0; i < sizeof(T)*2; ++i)
			{
				SPI_writeDataBlockingFIFO(module.base, byte8[i] << 8);
			}
			break;

		case SPI_WORD_16BIT:
			uint16_t byte16[sizeof(T)];
			memcpy(byte16, &data, sizeof(T));
			for (size_t i = 0; i < sizeof(T); ++i)
			{
				SPI_writeDataBlockingFIFO(module.base, byte16[i]);
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
		SPI_disableModule(module.base);
		Interrupt_register(module.rxPieIntNo, handler);
		SPI_enableInterrupt(module.base, SPI_INT_RXFF);
		SPI_enableModule(module.base);
	}

	/**
	 * @brief Enables Rx-interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void enableRxInterrupt() const { Interrupt_enable(module.rxPieIntNo); }

	/**
	 * @brief Acknowledges interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeRxInterrupt() const
	{
		SPI_clearInterruptStatus(module.base, SPI_INT_RXFF);
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
	}

	/**
	 * @brief Resets RX FIFO.
	 * @param (none)
	 * @return (none)
	 */
	void resetRxFifo() const
	{
		SPI_resetRxFIFO(module.base);
	}

	/**
	 * @brief Resets TX FIFO.
	 * @param (none)
	 * @return (none)
	 */
	void resetTxFifo() const
	{
		SPI_resetTxFIFO(module.base);
	}
};












/// @}
} /* namespace mcu */


