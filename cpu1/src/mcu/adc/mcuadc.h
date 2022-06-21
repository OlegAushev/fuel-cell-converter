/**
 * @defgroup mcu_adc ADC
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_adc
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "mcu/system/mcusystem.h"
#include "emb/emb_common.h"


namespace mcu {
/// @addtogroup mcu_adc
/// @{


/// ADC modules
enum AdcModule
{
	ADCA,
	ADCB,
	ADCC,
	ADCD
};

/// ADC channels (application-specific)
enum AdcChannel
{
	ADC_VOLTAGE_IN,
	ADC_VOLTAGE_OUT,
	ADC_CURRENT_IN_FIRST,
	ADC_CURRENT_IN_SECOND,

	ADC_CHANNEL_COUNT
};

/// ADC interrupt request source (application-specific).
enum AdcIrq
{
	ADC_IRQ_VOLTAGE_IN,
	ADC_IRQ_VOLTAGE_OUT,
	ADC_IRQ_CURRENT_IN_FIRST,
	ADC_IRQ_CURRENT_IN_SECOND,

	ADC_IRQ_COUNT
};


namespace detail {
/**
 * @brief ADC module implementation.
 */
struct AdcModuleImpl
{
	const uint32_t base;
};
} // namespace detail


/**
 * @brief ADC unit class.
 */
class AdcUnit : public emb::c28x::Singleton<AdcUnit>
{
private:
	struct Channel
	{
		uint32_t base;
		uint32_t resultBase;
		ADC_Channel channel;
		ADC_SOCNumber soc;
		ADC_Trigger trigger;
		Channel() {}
		Channel(uint32_t _base, uint32_t _resultBase, ADC_Channel _channel, ADC_SOCNumber _soc, ADC_Trigger _trigger)
			: base(_base)
			, resultBase(_resultBase)
			, channel(_channel)
			, soc(_soc)
			, trigger(_trigger)
		{}
	};
	static Channel m_channels[ADC_CHANNEL_COUNT];
	static void initChannels();

	struct Irq
	{
		uint32_t base;
		ADC_IntNumber intNum;
		ADC_SOCNumber soc;
		uint32_t pieIntNum;
		Irq() {}
		Irq(uint32_t _base, ADC_IntNumber _intNum, ADC_SOCNumber _soc, uint32_t _pieIntNum)
			: base(_base)
			, intNum(_intNum)
			, soc(_soc)
			, pieIntNum(_pieIntNum)
		{}
	};
	static Irq m_irqs[ADC_IRQ_COUNT];
	static void initIRQs();

	static const uint32_t SAMPLE_WINDOW = 100;			// duration in ns
	static const uint32_t SAMPLE_WINDOW_CYCLES = SAMPLE_WINDOW / 5;	// 5ns - one SYSCLK

	AdcUnit(const AdcUnit& other);			// no copy constructor
	AdcUnit& operator=(const AdcUnit& other);	// no copy assignment operator
public:
	/// ADC modules
	static const detail::AdcModuleImpl module[4];

	/**
	 * @brief Initializes MCU ADC unit.
	 * @param channelCount - number of ADC channels to make ctor non-default
	 */
	AdcUnit(int channelCount);

/*============================================================================*/
/*============================ Interrupts ====================================*/
/*============================================================================*/
	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts() const
	{
		for (size_t i = 0; i < ADC_IRQ_COUNT; ++i)
		{
			Interrupt_enable(m_irqs[i].pieIntNum);
		}
	}

	/**
	 * @brief Registers ADC ISR
	 * @param irq - interrupt request
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	void registerInterruptHandler(AdcIrq irq, void (*handler)(void)) const
	{
		Interrupt_register(m_irqs[irq].pieIntNum, handler);
	}

	/**
	 * @brief Acknowledges interrupt.
	 * @param irq - interrupt request
	 * @return (none)
	 */
	void acknowledgeInterrupt(AdcIrq irq) const
	{
		ADC_clearInterruptStatus(m_irqs[irq].base, m_irqs[irq].intNum);

		switch (m_irqs[irq].intNum)
		{
		case ADC_INT_NUMBER1:
			Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
			break;
		case ADC_INT_NUMBER2: case ADC_INT_NUMBER3: case ADC_INT_NUMBER4:
			Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP10);
			break;
		}
	}

	/**
	 * @brief Returns interrupt status of IRQ.
	 * @param irq - interrupt request
	 * @return \c true if the interrupt flag is set and \c false if it is not.
	 */
	bool interruptPending(AdcIrq irq) const
	{
		return ADC_getInterruptStatus(m_irqs[irq].base, m_irqs[irq].intNum);
	}

	/**
	 * @brief Clears interrupt status of specified IRQ.
	 * @param (none)
	 * @return (none)
	 */
	void clearInterruptStatus(AdcIrq irq) const
	{
		ADC_clearInterruptStatus(m_irqs[irq].base, m_irqs[irq].intNum);
	}

/*============================================================================*/
/*============================= ADC methods ==================================*/
/*============================================================================*/
/*============================================================================*/
/*=========================== Current methods ================================*/
/*============================================================================*/

	uint16_t read(AdcChannel channel) const
	{
		return ADC_readResult(m_channels[channel].resultBase, m_channels[channel].soc);
	}

	/**
	 * @brief Launches input current ADC.
	 * @param (none)
	 * @return (none)
	 */
	void startCurrentIn() const
	{
		ADC_forceSOC(m_channels[ADC_CURRENT_IN_FIRST].base, m_channels[ADC_CURRENT_IN_FIRST].soc);
	}

	/**
	 * @brief Returns raw ADC-result of input current.
	 * @param (none0
	 * @return Raw ADC-result of input current.
	 */
//	uint16_t currentIn() const
//	{
//		return 42;//ADC_readResult(m_channels[ADC_CURRENT_IN].resultBase, m_channels[ADC_CURRENT_IN].soc);
//	}

/*============================================================================*/
/*=========================== Voltage methods ================================*/
/*============================================================================*/
	/**
	 * @brief Launch input voltage ADC.
	 * @param (none)
	 * @return (none)
	 */
	void startVoltageIn() const
	{
		ADC_forceSOC(m_channels[ADC_VOLTAGE_IN].base, m_channels[ADC_VOLTAGE_IN].soc);
	}

	/**
	 * @brief Launch output voltage ADC.
	 * @param (none)
	 * @return (none)
	 */
	void startVoltageOut() const
	{
		ADC_forceSOC(m_channels[ADC_VOLTAGE_OUT].base, m_channels[ADC_VOLTAGE_OUT].soc);
	}

	/**
	 * @brief Returns raw ADC-result of input voltage.
	 * @param (none)
	 * @return Raw ADC-result of input voltage.
	 */
	uint16_t voltageIn() const
	{
		return ADC_readResult(m_channels[ADC_VOLTAGE_IN].resultBase, m_channels[ADC_VOLTAGE_IN].soc);
	}

	/**
	 * @brief Returns raw ADC-result of output voltage.
	 * @param (none)
	 * @return Raw ADC-result of output voltage.
	 */
	uint16_t voltageOut() const
	{
		return ADC_readResult(m_channels[ADC_VOLTAGE_OUT].resultBase, m_channels[ADC_VOLTAGE_OUT].soc);
	}
};


/// @}
} /* namespace mcu */


