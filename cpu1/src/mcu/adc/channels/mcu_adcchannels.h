/**
 * @file
 * @ingroup mcu mcu_adc
 */


#pragma once


namespace mcu {
/// @addtogroup mcu_adc
/// @{


/// ADC channels (application-specific)
enum AdcChannelName
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


/// @}
} // namespace mcu


