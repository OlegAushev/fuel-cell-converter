/**
 * @defgroup mcu_sci SCI
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_sci
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "../gpio/mcu_gpio.h"
#include "emb/emb_common.h"
#include "emb/emb_interfaces/emb_uart.h"


namespace mcu {
/// @addtogroup mcu_sci
/// @{


/// SCI modules
enum SciModule
{
	SCIA,
	SCIB,
	SCIC,
	SCID
};


/// SCI baudrates
enum SciBaudrate
{
	SCI_BAUDRATE_9600 = 9600,
	SCI_BAUDRATE_115200 = 115200,
};


/// SCI word length
enum SciWordLen
{
	SCI_WORD_8BIT = SCI_CONFIG_WLEN_8,
	SCI_WORD_7BIT = SCI_CONFIG_WLEN_7,
	SCI_WORD_6BIT = SCI_CONFIG_WLEN_6,
	SCI_WORD_5BIT = SCI_CONFIG_WLEN_5,
	SCI_WORD_4BIT = SCI_CONFIG_WLEN_4,
	SCI_WORD_3BIT = SCI_CONFIG_WLEN_3,
	SCI_WORD_2BIT = SCI_CONFIG_WLEN_2,
	SCI_WORD_1BIT = SCI_CONFIG_WLEN_1
};


/// SCI stop bits count
enum SciStopBits
{
	SCI_STOP_BIT_ONE = SCI_CONFIG_STOP_ONE,
	SCI_STOP_BIT_TWO = SCI_CONFIG_STOP_TWO
};


/// SCI parity mode
enum SciParityMode
{
	SCI_PARITY_NONE = SCI_CONFIG_PAR_NONE,
	SCI_PARITY_EVEN = SCI_CONFIG_PAR_EVEN,
	SCI_PARITY_ODD = SCI_CONFIG_PAR_ODD
};


/// SCI Auto-baud mode
enum SciAutoBaudMode
{
	SCI_AUTO_BAUD_DISABLED,
	SCI_AUTO_BAUD_ENABLED
};



/**
 * @brief SCI unit config.
 */
struct SciConfig
{
	SciBaudrate baudrate;
	SciWordLen wordLen;
	SciStopBits stopBits;
	SciParityMode parityMode;
	SciAutoBaudMode autoBaudMode;
};


namespace detail {


/**
 * @brief SCI module implementation.
 */
struct SciModuleImpl
{
	uint32_t base;
	uint32_t pieRxIntNum;
	uint16_t pieIntGroup;
	SciModuleImpl(uint32_t _base, uint32_t _pieRxIntNum, uint16_t _pieIntGroup)
		: base(_base), pieRxIntNum(_pieRxIntNum), pieIntGroup(_pieIntGroup) {}
};


extern const uint32_t sciBases[4];
extern const uint32_t sciRxPieIntNums[4];
extern const uint16_t sciPieIntGroups[4];


} // namespace detail


/**
 * @brief SCI unit class.
 */
template <SciModule Module>
class Sci : public emb::c28x::Singleton<Sci<Module> >, public emb::IUart
{
private:
	detail::SciModuleImpl m_module;

private:
	Sci(const Sci& other);			// no copy constructor
	Sci& operator=(const Sci& other);	// no copy assignment operator
public:
	/**
	 * @brief Initializes MCU SCI unit.
	 * @param rxPin
	 * @param txPin
	 * @param cfg
	 */
	Sci(const GpioConfig& rxPin, const GpioConfig& txPin,
			const SciConfig& cfg)
		: emb::c28x::Singleton<Sci<Module> >(this)
		, m_module(detail::sciBases[Module],
				detail::sciRxPieIntNums[Module],
				detail::sciPieIntGroups[Module])
	{
#ifdef CPU1
		_initPins(rxPin, txPin);
#endif
		SCI_disableModule(m_module.base);

		uint32_t configFlags = static_cast<uint32_t>(cfg.wordLen)
				| static_cast<uint32_t>(cfg.stopBits)
				| static_cast<uint32_t>(cfg.parityMode);

		SCI_setConfig(m_module.base, DEVICE_LSPCLK_FREQ,
				static_cast<uint32_t>(cfg.baudrate),
				configFlags);

		SCI_resetChannels(m_module.base);
		SCI_resetRxFIFO(m_module.base);
		SCI_resetTxFIFO(m_module.base);

		SCI_clearInterruptStatus(m_module.base, SCI_INT_TXFF | SCI_INT_RXFF);
		SCI_setFIFOInterruptLevel(m_module.base, SCI_FIFO_TX8, SCI_FIFO_RX8);
		SCI_enableFIFO(m_module.base);
		SCI_enableModule(m_module.base);
		SCI_performSoftwareReset(m_module.base);

		if (cfg.autoBaudMode == SCI_AUTO_BAUD_ENABLED)
		{
			// Perform an autobaud lock.
			// SCI expects an 'a' or 'A' to lock the baud rate.
			SCI_lockAutobaud(m_module.base);
		}
		// TODO

	}

#ifdef CPU1
	/**
	 * @brief Transfers control over SCI unit to CPU2.
	 * @param txPin
	 * @param rxPin
	 * @return (none)
	 */
	static void transferControlToCpu2(const GpioConfig& rxPin, const GpioConfig& txPin)
	{
		_initPins(rxPin, txPin);
		GPIO_setMasterCore(rxPin.no, GPIO_CORE_CPU2);
		GPIO_setMasterCore(txPin.no, GPIO_CORE_CPU2);
		SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL5_SCI,
				static_cast<uint16_t>(Module)+1, SYSCTL_CPUSEL_CPU2);
	}
#endif

	/**
	 * @brief Returns base of SCI-unit.
	 * @param (none)
	 * @return Base of SPI-unit.
	 */
	uint32_t base() const { return m_module.base; }

	/**
	 * @brief Resets SCI-unit.
	 * @param (none)
	 * @return (none)
	 */
	virtual void reset()
	{
		SCI_performSoftwareReset(m_module.base);
	}

	/**
	 * @brief Checks if there is any Rx-errors.
	 * @param (none)
	 * @return \c true if there is any Rx-error, \c false otherwise.
	 */
	virtual bool hasRxError() const
	{
		return SCI_getRxStatus(m_module.base) & SCI_RXSTATUS_ERROR;
	}

	/**
	 * @brief Receives one char in non-blocking mode.
	 * @param ch - reference to char var
	 * @return Status of operation: "1" - char received, "0" - otherwise.
	 */
	virtual int recv(char& ch)
	{
		if (SCI_getRxFIFOStatus(m_module.base) != SCI_FIFO_RX0)
		{
			ch = SCI_readCharNonBlocking(m_module.base);
			return 1;
		}
		return 0;
	}

	/**
	 * @brief Receives chars in non-blocking mode.
	 * @param buf - pointer to buffer
	 * @param bufLen - buffer size
	 * @return Number of received characters.
	 */
	virtual int recv(char* buf, size_t bufLen)
	{
		size_t i = 0;
		char ch = 0;

		while ((i < bufLen) && (recv(ch) == 1))
		{
			buf[i++] = ch;
		}

		if (hasRxError())
		{
			return -1;
		}
		return i;
	}

	/**
	 * @brief Sends one char in blocking mode.
	 * @param ch - char to be send
	 * @return Status of operation: "1" - char sent, "0" - otherwise.
	 */
	virtual int send(char ch)
	{
		if (SCI_getTxFIFOStatus(m_module.base) != SCI_FIFO_TX15)
		{
			SCI_writeCharBlockingFIFO(m_module.base, ch);
			return 1;
		}
		return 0;
	}

	/**
	 * @brief Sends chars from buffer in blocking mode.
	 * @param buf - pointer to buffer to be send
	 * @param len - length of buffer
	 * @return Number of sent characters.
	 */
	virtual int send(const char* buf, uint16_t len)
	{
		SCI_writeCharArray(m_module.base, reinterpret_cast<const uint16_t*>(buf), len);
		return len;
	}

	/**
	 * @brief Registers Rx-interrupt handler.
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	virtual void registerRxInterruptHandler(void (*handler)(void))
	{
		SCI_disableModule(m_module.base);
		Interrupt_register(m_module.pieRxIntNum, handler);
		SCI_enableInterrupt(m_module.base, SCI_INT_RXFF);
		SCI_enableModule(m_module.base);
	}

	/**
	 * @brief Enables Rx-interrupts.
	 * @param (none)
	 * @return (none)
	 */
	virtual void enableRxInterrupts()
	{
		Interrupt_enable(m_module.pieRxIntNum);
	}

	/**
	 * @brief Disables Rx-interrupts.
	 * @param (none)
	 * @return (none)
	 */
	virtual void disableRxInterrupts()
	{
		Interrupt_disable(m_module.pieRxIntNum);
	}

	/**
	 * @brief Acknowledges interrupt.
	 * @param (none)
	 * @return (none)
	 */
	virtual void acknowledgeRxInterrupt()
	{
		SCI_clearInterruptStatus(m_module.base, SPI_INT_RXFF);
		Interrupt_clearACKGroup(m_module.pieIntGroup);
	}

protected:
	static void _initPins(const GpioConfig& rxPin, const GpioConfig& txPin)
	{
		GPIO_setPinConfig(rxPin.mux);
		GPIO_setDirectionMode(rxPin.no, GPIO_DIR_MODE_IN);
		GPIO_setPadConfig(rxPin.no, GPIO_PIN_TYPE_STD);
		GPIO_setQualificationMode(rxPin.no, GPIO_QUAL_ASYNC);

		GPIO_setPinConfig(txPin.mux);
		GPIO_setDirectionMode(txPin.no, GPIO_DIR_MODE_OUT);
		GPIO_setPadConfig(txPin.no, GPIO_PIN_TYPE_STD);
		GPIO_setQualificationMode(txPin.no, GPIO_QUAL_ASYNC);
	}
};


/// @}
} // namespace mcu


