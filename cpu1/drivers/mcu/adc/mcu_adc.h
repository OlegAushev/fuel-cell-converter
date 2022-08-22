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
#include "emb/emb_common.h"
#include "emb/emb_array.h"
#include "mcu/system/mcu_system.h"
#include "mcu/adc/channels/mcu_adcchannels.h"


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


/**
 * @brief ADC unit config.
 */
struct AdcConfig
{
	uint32_t sampleWindow_ns;
};


namespace detail {


/**
 * @brief ADC module implementation.
 */
struct AdcModuleImpl
{
	uint32_t base;
};


extern const uint32_t adcBases[4];


/**
 * @brief ADC channel implementation.
 */
struct AdcChannelImpl
{
	uint32_t base;
	uint32_t resultBase;
	ADC_Channel channel;
	ADC_SOCNumber soc;
	ADC_Trigger trigger;
	AdcChannelImpl() {}
	AdcChannelImpl(uint32_t _base, uint32_t _resultBase, ADC_Channel _channel, ADC_SOCNumber _soc, ADC_Trigger _trigger)
		: base(_base)
		, resultBase(_resultBase)
		, channel(_channel)
		, soc(_soc)
		, trigger(_trigger)
	{}
};


/**
 * @brief ADC IRQ implementation.
 */
struct AdcIrqImpl
{
	uint32_t base;
	ADC_IntNumber intNum;
	ADC_SOCNumber soc;
	uint32_t pieIntNum;
	AdcIrqImpl() {}
	AdcIrqImpl(uint32_t _base, ADC_IntNumber _intNum, ADC_SOCNumber _soc, uint32_t _pieIntNum)
		: base(_base)
		, intNum(_intNum)
		, soc(_soc)
		, pieIntNum(_pieIntNum)
	{}
};


} // namespace detail


/**
 * @brief Initializes ADC channels.
 * @param channels - ADC channel array
 * @return (none)
 */
void initAdcChannels(emb::Array<detail::AdcChannelImpl, ADC_CHANNEL_COUNT>& channels);


/**
 * @brief Initializes ADC IRQs.
 * @param irqs - ADC IRQ array
 * @return (none)
 */
void initAdcIrqs(emb::Array<detail::AdcIrqImpl, ADC_IRQ_COUNT>& irqs);


/**
 * @brief ADC unit class.
 */
class Adc : public emb::c28x::Singleton<Adc>
{
private:
	detail::AdcModuleImpl m_module[4];

	static emb::Array<detail::AdcChannelImpl, ADC_CHANNEL_COUNT> s_channels;
	static emb::Array<detail::AdcIrqImpl, ADC_IRQ_COUNT> s_irqs;

	const uint32_t SAMPLE_WINDOW_CYCLES;

private:
	Adc(const Adc& other);			// no copy constructor
	Adc& operator=(const Adc& other);	// no copy assignment operator
public:
	/**
	 * @brief Initializes MCU ADC unit.
	 * @param cfg - ADC config
	 */
	Adc(const AdcConfig& cfg);

	/**
	 * @brief Starts conversion on specified channel.
	 * @param channel - ADC channel
	 * @return (none)
	 */
	void start(AdcChannelName channel) const
	{
		ADC_forceSOC(s_channels[channel].base, s_channels[channel].soc);
	}

	/**
	 * @brief Returns result of ADC on specified channel.
	 * @param channel - ADC channel
	 * @return ADC-result raw data.
	 */
	uint16_t read(AdcChannelName channel) const
	{
		return ADC_readResult(s_channels[channel].resultBase, s_channels[channel].soc);
	}

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
			Interrupt_enable(s_irqs[i].pieIntNum);
		}
	}

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts() const
	{
		for (size_t i = 0; i < ADC_IRQ_COUNT; ++i)
		{
			Interrupt_disable(s_irqs[i].pieIntNum);
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
		Interrupt_register(s_irqs[irq].pieIntNum, handler);
	}

	/**
	 * @brief Acknowledges interrupt.
	 * @param irq - interrupt request
	 * @return (none)
	 */
	void acknowledgeInterrupt(AdcIrq irq) const
	{
		ADC_clearInterruptStatus(s_irqs[irq].base, s_irqs[irq].intNum);

		switch (s_irqs[irq].intNum)
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
	bool interruptPending(AdcIrq irq)
	{
		return ADC_getInterruptStatus(s_irqs[irq].base, s_irqs[irq].intNum);
	}

	/**
	 * @brief Clears interrupt status of specified IRQ.
	 * @param (none)
	 * @return (none)
	 */
	void clearInterruptStatus(AdcIrq irq)
	{
		ADC_clearInterruptStatus(s_irqs[irq].base, s_irqs[irq].intNum);
	}
};


/**
 * @brief ADC channel class
 */
class AdcChannel
{
public:
	Adc* adc;
private:
	AdcChannelName m_channelName;
public:
	/**
	 * @brief Initializes ADC channel.
	 * @param (none)
	 */
	AdcChannel()
		: adc(Adc::instance())
		, m_channelName(ADC_CHANNEL_COUNT)	// dummy write
	{}


	/**
	 * @brief Initializes ADC channel.
	 * @param channelName - channel name
	 */
	AdcChannel(AdcChannelName channelName)
		: adc(Adc::instance())
		, m_channelName(channelName)
	{}


	/**
	 * @brief Initializes ADC channel.
	 * @param channelName - channel name
	 * @return (none)
	 */
	void init(AdcChannelName channelName)
	{
		m_channelName = channelName;
	}


	/**
	 * @brief Starts conversion.
	 * @param (none)
	 * @return (none)
	 */
	void start() const
	{
		adc->start(m_channelName);
	}


	/**
	 * @brief Returns result of ADC.
	 * @param (none)
	 * @return ADC-result raw data.
	 */
	uint16_t read() const
	{
		return adc->read(m_channelName);
	}
};


/// @}
} // namespace mcu


