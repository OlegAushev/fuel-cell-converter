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
	ADC_CURRENT_PHASE_U,
	ADC_CURRENT_PHASE_V,
	ADC_CURRENT_PHASE_W,
	ADC_CURRENT_PHASE_X,
	ADC_CURRENT_PHASE_Y,
	ADC_CURRENT_PHASE_Z,

	ADC_VOLTAGE_DC,

	ADC_TEMPERATURE_PHASE_U,
	ADC_TEMPERATURE_PHASE_V,
	ADC_TEMPERATURE_PHASE_W,
	ADC_TEMPERATURE_PHASE_X,
	ADC_TEMPERATURE_PHASE_Y,
	ADC_TEMPERATURE_PHASE_Z,
	ADC_TEMPERATURE_CASE,

	ADC_CHANNEL_COUNT
};

/// ADC interrupt request source (application-specific).
enum AdcIrq
{
	ADC_IRQ_CURRENT_UVW,
	ADC_IRQ_CURRENT_XYZ,
	ADC_IRQ_VOLTAGE_DC,
	ADC_IRQ_TEMPERATURE_UVW,
	ADC_IRQ_TEMPERATURE_XYZ,
	ADC_IRQ_TEMPERATURE_CASE,

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
		ADC_IntNumber intNo;
		ADC_SOCNumber soc;
		uint32_t pieIntNo;
		Irq() {}
		Irq(uint32_t _base, ADC_IntNumber _intNo, ADC_SOCNumber _soc, uint32_t _pieIntNo)
			: base(_base)
			, intNo(_intNo)
			, soc(_soc)
			, pieIntNo(_pieIntNo)
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

	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts() const;

/*============================================================================*/
/*=========================== Current methods ================================*/
/*============================================================================*/
	/**
	 * @brief Launches UVW-phase currents ADCs.
	 * @param (none)
	 * @return (none)
	 */
	void convertCurrentsUVW() const
	{
		ADC_forceSOC(m_channels[ADC_CURRENT_PHASE_U].base, m_channels[ADC_CURRENT_PHASE_U].soc);
		ADC_forceSOC(m_channels[ADC_CURRENT_PHASE_V].base, m_channels[ADC_CURRENT_PHASE_V].soc);
		ADC_forceSOC(m_channels[ADC_CURRENT_PHASE_W].base, m_channels[ADC_CURRENT_PHASE_W].soc);
	}

	/**
	 * @brief Launches XYZ-phase currents ADCs.
	 * @param (none)
	 * @return (none)
	 */
	void convertCurrentsXYZ() const
	{
		ADC_forceSOC(m_channels[ADC_CURRENT_PHASE_X].base, m_channels[ADC_CURRENT_PHASE_X].soc);
		ADC_forceSOC(m_channels[ADC_CURRENT_PHASE_Y].base, m_channels[ADC_CURRENT_PHASE_Y].soc);
		ADC_forceSOC(m_channels[ADC_CURRENT_PHASE_Z].base, m_channels[ADC_CURRENT_PHASE_Z].soc);
	}

	/**
	 * @brief Returns raw ADC-result of current of specified phase.
	 * @param phase - phase
	 * @return Raw ADC-result of current of specified phase.
	 */
//	uint16_t current(crd600::Phase phase) const
//	{
//		size_t id = static_cast<size_t>(ADC_CURRENT_PHASE_U) + static_cast<size_t>(phase);
//		return ADC_readResult(m_channels[id].resultBase, m_channels[id].soc);
//	}

/*============================================================================*/
/*=========================== Voltage methods ================================*/
/*============================================================================*/
	/**
	 * @brief Launch DC-voltage ADC.
	 * @param (none)
	 * @return (none)
	 */
	void convertVoltageDc() const
	{
		ADC_forceSOC(m_channels[ADC_VOLTAGE_DC].base, m_channels[ADC_VOLTAGE_DC].soc);
	}

	/**
	 * @brief Returns raw ADC-result of DC-voltage.
	 * @param (none)
	 * @return Raw ADC-result of DC-voltage.
	 */
	uint16_t voltageDc() const
	{
		return ADC_readResult(m_channels[ADC_VOLTAGE_DC].resultBase, m_channels[ADC_VOLTAGE_DC].soc);
	}

/*============================================================================*/
/*======================= Temperature methods ================================*/
/*============================================================================*/
	/**
	 * @brief Launch UVW-phase modules temperatures ADCs.
	 * @param (none)
	 * @return (none)
	 */
	void convertTemperaturesUVW() const
	{
		ADC_forceSOC(m_channels[ADC_TEMPERATURE_PHASE_U].base, m_channels[ADC_TEMPERATURE_PHASE_U].soc);
		ADC_forceSOC(m_channels[ADC_TEMPERATURE_PHASE_V].base, m_channels[ADC_TEMPERATURE_PHASE_V].soc);
		ADC_forceSOC(m_channels[ADC_TEMPERATURE_PHASE_W].base, m_channels[ADC_TEMPERATURE_PHASE_W].soc);
	}

	/**
	 * @brief Launch XYZ-phase modules temperatures ADCs.
	 * @param (none)
	 * @return (none)
	 */
	void convertTemperaturesXYZ() const
	{
		ADC_forceSOC(m_channels[ADC_TEMPERATURE_PHASE_X].base, m_channels[ADC_TEMPERATURE_PHASE_X].soc);
		ADC_forceSOC(m_channels[ADC_TEMPERATURE_PHASE_Y].base, m_channels[ADC_TEMPERATURE_PHASE_Y].soc);
		ADC_forceSOC(m_channels[ADC_TEMPERATURE_PHASE_Z].base, m_channels[ADC_TEMPERATURE_PHASE_Z].soc);
	}

	/**
	 * @brief Launch case temperature ADC.
	 * @param (none)
	 * @return (none)
	 */
	void convertTemperatureCase() const
	{
		ADC_forceSOC(m_channels[ADC_TEMPERATURE_CASE].base, m_channels[ADC_TEMPERATURE_CASE].soc);
	}

	/**
	 * @brief Returns raw ADC-result of module temperature of specified phase.
	 * @param phase - phase
	 * @return Raw ADC-result of module temperature of specified phase.
	 */
//	uint16_t temperatureModule(crd600::Phase phase) const
//	{
//		size_t id = static_cast<size_t>(ADC_TEMPERATURE_PHASE_U) + static_cast<size_t>(phase);
//		return ADC_readResult(m_channels[id].resultBase, m_channels[id].soc);
//	}

	/**
	 * @brief Returns raw ADC-result of case temperature.
	 * @param (none)
	 * @return Raw ADC-result of case temperature.
	 */
	uint16_t temperatureCase() const
	{
		return ADC_readResult(m_channels[ADC_TEMPERATURE_CASE].resultBase, m_channels[ADC_TEMPERATURE_CASE].soc);
	}

/*============================================================================*/
/*============================ Interrupts ====================================*/
/*============================================================================*/
	/**
	 * @brief Registers ADC ISR
	 * @param irq - interrupt request
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	void registerInterruptHandler(AdcIrq irq, void (*handler)(void)) const
	{
		Interrupt_register(m_irqs[irq].pieIntNo, handler);
	}

	/**
	 * @brief Acknowledges interrupt.
	 * @param irq - interrupt request
	 * @return (none)
	 */
	void acknowledgeInterrupt(AdcIrq irq) const
	{
		ADC_clearInterruptStatus(m_irqs[irq].base, m_irqs[irq].intNo);

		switch (m_irqs[irq].intNo)
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
		return ADC_getInterruptStatus(m_irqs[irq].base, m_irqs[irq].intNo);
	}

	/**
	 * @brief Clears interrupt status of specified IRQ.
	 * @param (none)
	 * @return (none)
	 */
	void clearInterruptStatus(AdcIrq irq)
	{
		ADC_clearInterruptStatus(m_irqs[irq].base, m_irqs[irq].intNo);
	}
};


/// @}
} /* namespace mcu */


