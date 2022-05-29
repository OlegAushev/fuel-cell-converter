/**
 * @file
 * @ingroup mcu mcu_adc
 */


#include "mcuadc.h"


namespace mcu {

const AdcModuleImpl AdcUnit::module[4] = { {.base = ADCA_BASE}, {.base = ADCB_BASE}, {.base = ADCC_BASE}, {.base = ADCD_BASE} };
AdcUnit::Channel AdcUnit::m_channels[ADC_CHANNEL_COUNT];
AdcUnit::Irq AdcUnit::m_irqs[ADC_IRQ_COUNT];

///
///
///
void AdcUnit::initChannels()
{
//	m_channels[ADC_CURRENT_PHASE_U] = Channel(ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN0, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY);
//	m_channels[ADC_CURRENT_PHASE_V] = Channel(ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN1, ADC_SOC_NUMBER1, ADC_TRIGGER_SW_ONLY);
//	m_channels[ADC_CURRENT_PHASE_W] = Channel(ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN2, ADC_SOC_NUMBER2, ADC_TRIGGER_SW_ONLY);
//	m_channels[ADC_CURRENT_PHASE_X] = Channel(ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN5, ADC_SOC_NUMBER3, ADC_TRIGGER_SW_ONLY);
//	m_channels[ADC_CURRENT_PHASE_Y] = Channel(ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN4, ADC_SOC_NUMBER4, ADC_TRIGGER_SW_ONLY);
//	m_channels[ADC_CURRENT_PHASE_Z] = Channel(ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN3, ADC_SOC_NUMBER5, ADC_TRIGGER_SW_ONLY);
//
//	m_channels[ADC_VOLTAGE_DC] = Channel(ADCD_BASE, ADCDRESULT_BASE, ADC_CH_ADCIN0, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY);
//
//	m_channels[ADC_TEMPERATURE_PHASE_U] = Channel(ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN14, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY);
//	m_channels[ADC_TEMPERATURE_PHASE_V] = Channel(ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN15, ADC_SOC_NUMBER1, ADC_TRIGGER_SW_ONLY);
//	m_channels[ADC_TEMPERATURE_PHASE_W] = Channel(ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN2, ADC_SOC_NUMBER2, ADC_TRIGGER_SW_ONLY);
//	m_channels[ADC_TEMPERATURE_PHASE_X] = Channel(ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN3, ADC_SOC_NUMBER3, ADC_TRIGGER_SW_ONLY);
//	m_channels[ADC_TEMPERATURE_PHASE_Y] = Channel(ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN4, ADC_SOC_NUMBER4, ADC_TRIGGER_SW_ONLY);
//	m_channels[ADC_TEMPERATURE_PHASE_Z] = Channel(ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN5, ADC_SOC_NUMBER5, ADC_TRIGGER_SW_ONLY);
//
//	m_channels[ADC_TEMPERATURE_CASE] = Channel(ADCD_BASE, ADCDRESULT_BASE, ADC_CH_ADCIN1, ADC_SOC_NUMBER1, ADC_TRIGGER_SW_ONLY);
}

///
///
///
void AdcUnit::initIRQs()
{
//	m_irqs[ADC_IRQ_CURRENT_UVW] = Irq(ADCB_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER2, INT_ADCB2);
//	m_irqs[ADC_IRQ_CURRENT_XYZ] = Irq(ADCB_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER5, INT_ADCB3);
//
//	m_irqs[ADC_IRQ_VOLTAGE_DC] = Irq(ADCD_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER0, INT_ADCD2);
//
//	m_irqs[ADC_IRQ_TEMPERATURE_UVW] = Irq(ADCC_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER2, INT_ADCC2);
//	m_irqs[ADC_IRQ_TEMPERATURE_XYZ] = Irq(ADCC_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER5, INT_ADCC3);
//
//	m_irqs[ADC_IRQ_TEMPERATURE_CASE] = Irq(ADCD_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER1, INT_ADCD3);
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

///
///
///
void AdcUnit::enableInterrupts() const
{
	for (size_t i = 0; i < ADC_IRQ_COUNT; ++i)
	{
		Interrupt_enable(m_irqs[i].pieIntNo);
	}
}



} /* namespace uc */
