/**
 * @file
 * @ingroup mcu mcu_adc
 */


#include "mcuadc.h"


namespace mcu {

const detail::AdcModuleImpl AdcUnit::module[4] =
		{ {.base = ADCA_BASE}, {.base = ADCB_BASE}, {.base = ADCC_BASE}, {.base = ADCD_BASE} };
AdcUnit::Channel AdcUnit::m_channels[ADC_CHANNEL_COUNT];
AdcUnit::Irq AdcUnit::m_irqs[ADC_IRQ_COUNT];

///
///
///
void AdcUnit::initChannels()
{
#ifdef CRD300
	m_channels[ADC_VOLTAGE_IN] =		Channel(ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN2, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA);
	m_channels[ADC_VOLTAGE_OUT] =		Channel(ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN3, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA);
	m_channels[ADC_CURRENT_IN_FIRST] =	Channel(ADCA_BASE, ADCARESULT_BASE, ADC_CH_ADCIN0, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA);
	m_channels[ADC_CURRENT_IN_SECOND] = 	Channel(ADCA_BASE, ADCARESULT_BASE, ADC_CH_ADCIN0, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCB);
#else
#warning "TODO"
#endif
}

///
///
///
void AdcUnit::initIRQs()
{
#ifdef CRD300
	m_irqs[ADC_IRQ_VOLTAGE_IN] = Irq(ADCC_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0, INT_ADCC1);
	m_irqs[ADC_IRQ_VOLTAGE_OUT] = Irq(ADCC_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER1, INT_ADCC2);
	m_irqs[ADC_IRQ_CURRENT_IN_FIRST] = Irq(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0, INT_ADCA1);
	m_irqs[ADC_IRQ_CURRENT_IN_SECOND] = Irq(ADCA_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER1, INT_ADCA2);
#else
#warning "TODO"
#endif
}

///
///
///
AdcUnit::AdcUnit(int channelCount)
	: emb::c28x::Singleton<AdcUnit>(this)
{
	initChannels();
	initIRQs();

	for (uint32_t i = 0; i < 4; ++i)
	{
		ADC_setPrescaler(module[i].base, ADC_CLK_DIV_4_0);	// fclk(adc)max = 50 MHz
		ADC_setMode(module[i].base, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
		ADC_setInterruptPulseMode(module[i].base, ADC_PULSE_END_OF_CONV);
		ADC_enableConverter(module[i].base);
		ADC_setSOCPriority(module[i].base, ADC_PRI_ALL_HIPRI);	// SOCs at high priority - easier to control order
		mcu::delay_us(1000);					// delay for power-up
	}

	// Configure SOCs
	// For 12-bit resolution, a sampling window of (5 x SAMPLE_WINDOW_CYCLES)ns
	// at a 200MHz SYSCLK rate will be used
	for (size_t i = 0; i < ADC_CHANNEL_COUNT; ++i)
	{
		ADC_setupSOC(m_channels[i].base, m_channels[i].soc, m_channels[i].trigger,
				m_channels[i].channel, SAMPLE_WINDOW_CYCLES);
	}

	// Interrupt config
	for (size_t i = 0; i < ADC_IRQ_COUNT; ++i)
	{
		ADC_setInterruptSource(m_irqs[i].base, m_irqs[i].intNo, m_irqs[i].soc);
		ADC_enableInterrupt(m_irqs[i].base, m_irqs[i].intNo);
		ADC_clearInterruptStatus(m_irqs[i].base, m_irqs[i].intNo);
	}
}


} /* namespace uc */
