/**
 * @file
 * @ingroup mcu mcu_adc
 */


#include "mcu_adc.h"


namespace mcu {


namespace detail {


const uint32_t adcBases[4] = {ADCA_BASE, ADCB_BASE, ADCC_BASE, ADCD_BASE};


}


emb::Array<detail::AdcChannelImpl, ADC_CHANNEL_COUNT> Adc::s_channels;
emb::Array<detail::AdcIrqImpl, ADC_IRQ_COUNT> Adc::s_irqs;


///
///
///
Adc::Adc(const AdcConfig& cfg)
	: emb::c28x::Singleton<Adc>(this)
	, SAMPLE_WINDOW_CYCLES(cfg.sampleWindow_ns / (1000000000 / mcu::sysclkFreq()))
{
	for (size_t i = 0; i < 4; ++i)
	{
		m_module[i].base = detail::adcBases[i];
	}

	initAdcChannels(s_channels);
	initAdcIrqs(s_irqs);

	for (uint32_t i = 0; i < 4; ++i)
	{
		ADC_setPrescaler(m_module[i].base, ADC_CLK_DIV_4_0);		// fclk(adc)max = 50 MHz
		ADC_setMode(m_module[i].base, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
		ADC_setInterruptPulseMode(m_module[i].base, ADC_PULSE_END_OF_CONV);
		ADC_enableConverter(m_module[i].base);
		ADC_setSOCPriority(m_module[i].base, ADC_PRI_ALL_HIPRI);	// SOCs at high priority - easier to control order
		mcu::delay_us(1000);						// delay for power-up
	}

	// Configure SOCs
	// For 12-bit resolution, a sampling window of (5 x SAMPLE_WINDOW_CYCLES)ns
	// at a 200MHz SYSCLK rate will be used
	for (size_t i = 0; i < ADC_CHANNEL_COUNT; ++i)
	{
		ADC_setupSOC(s_channels[i].base, s_channels[i].soc, s_channels[i].trigger,
				s_channels[i].channel, SAMPLE_WINDOW_CYCLES);
	}

	// Interrupt config
	for (size_t i = 0; i < ADC_IRQ_COUNT; ++i)
	{
		ADC_setInterruptSource(s_irqs[i].base, s_irqs[i].intNum, s_irqs[i].soc);
		ADC_enableInterrupt(s_irqs[i].base, s_irqs[i].intNum);
		ADC_clearInterruptStatus(s_irqs[i].base, s_irqs[i].intNum);
	}
}


} // namespace mcu


