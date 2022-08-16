/**
 * @defgroup mcu_cap CAP
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_cap
 */


#pragma once


#include "driverlib.h"
#include "device.h"

#include "mcu_c28x/gpio/mcu_gpio.h"
#include "emb/emb_common.h"
#include "emb/emb_array.h"


namespace mcu {
/// @addtogroup mcu_cap
/// @{


/// CAP modules
enum CapModule
{
	CAP1,
	CAP2,
	CAP3,
	CAP4,
	CAP5,
	CAP6
};


/**
 * @brief CAP config.
 */
template <unsigned int ChannelCount>
struct CapConfig
{
	CapModule module[ChannelCount];
	mcu::Gpio inputPin[ChannelCount];
};


namespace detail {


/**
 * @brief CAP module implementation.
 */
template <unsigned int ChannelCount>
struct CapModuleImpl
{
	CapModule instance[ChannelCount];
	uint32_t base[ChannelCount];
	XBAR_InputNum xbarInput[ChannelCount];
	uint32_t pieIntNum[ChannelCount];
};


extern const uint32_t capBases[6];
extern const XBAR_InputNum capXbarInputs[6];
extern const uint32_t capPieIntNums[6];


} // namespace detail


/**
 * @brief CAP unit class.
 */
template <unsigned int ChannelCount>
class Cap
{
private:
	detail::CapModuleImpl<ChannelCount> m_module;

private:
	Cap(const Cap& other);			// no copy constructor
	Cap& operator=(const Cap& other);	// no copy assignment operator
public:
	/**
	 * @brief Initializes MCU CAP unit.
	 * @param cfg - CAP unit config
	 */
	Cap(const CapConfig<ChannelCount>& cfg)
	{
		EMB_STATIC_ASSERT(ChannelCount > 0);
		EMB_STATIC_ASSERT(ChannelCount <= 6);

		// XBAR setup
		for (size_t i = 0; i < ChannelCount; ++i)
		{
			m_module.instance[i] = cfg.module[i];
			m_module.base[i] = detail::capBases[cfg.module[i]];
			m_module.xbarInput[i] = detail::capXbarInputs[cfg.module[i]];
			m_module.pieIntNum[i] = detail::capPieIntNums[cfg.module[i]];
			XBAR_setInputPin(m_module.xbarInput[i], cfg.inputPin[i].config().no);
		}

		/* ECAP setup */
		for (size_t i = 0; i < ChannelCount; ++i)
		{
			ECAP_disableInterrupt(m_module.base[i],
								  (ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
								   ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
								   ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
								   ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
								   ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
								   ECAP_ISR_SOURCE_COUNTER_PERIOD   |
								   ECAP_ISR_SOURCE_COUNTER_COMPARE));
			ECAP_clearInterrupt(m_module.base[i],
								(ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
								 ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
								 ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
								 ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
								 ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
								 ECAP_ISR_SOURCE_COUNTER_PERIOD   |
								 ECAP_ISR_SOURCE_COUNTER_COMPARE));

			ECAP_disableTimeStampCapture(m_module.base[i]);

			ECAP_stopCounter(m_module.base[i]);
			ECAP_enableCaptureMode(m_module.base[i]);
			ECAP_setCaptureMode(m_module.base[i], ECAP_CONTINUOUS_CAPTURE_MODE, ECAP_EVENT_2);


			ECAP_setEventPolarity(m_module.base[i], ECAP_EVENT_1, ECAP_EVNT_RISING_EDGE);
			ECAP_setEventPolarity(m_module.base[i], ECAP_EVENT_2, ECAP_EVNT_FALLING_EDGE);

			ECAP_enableCounterResetOnEvent(m_module.base[i], ECAP_EVENT_1);
			ECAP_enableCounterResetOnEvent(m_module.base[i], ECAP_EVENT_2);

			ECAP_setSyncOutMode(m_module.base[i], ECAP_SYNC_OUT_DISABLED);
			ECAP_startCounter(m_module.base[i]);
			ECAP_enableTimeStampCapture(m_module.base[i]);
			ECAP_reArm(m_module.base[i]);
		}
	}

	/**
	 * @brief Rearms unit.
	 * @param (none)
	 * @return (none)
	 */
	void rearm() const
	{
		for (int i = 0; i < ChannelCount; ++i)
		{
			ECAP_reArm(m_module.base[i]);
		}
	}

	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts() const
	{
		for (int i = 0; i < ChannelCount; ++i)
		{
			ECAP_enableInterrupt(m_module.base[i], ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
			ECAP_enableInterrupt(m_module.base[i], ECAP_ISR_SOURCE_CAPTURE_EVENT_2);
			ECAP_enableInterrupt(m_module.base[i], ECAP_ISR_SOURCE_COUNTER_OVERFLOW);
		}
	}

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts() const
	{
		for (int i = 0; i < ChannelCount; ++i)
		{
			ECAP_disableInterrupt(m_module.base[i], ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
			ECAP_disableInterrupt(m_module.base[i], ECAP_ISR_SOURCE_CAPTURE_EVENT_2);
			ECAP_disableInterrupt(m_module.base[i], ECAP_ISR_SOURCE_COUNTER_OVERFLOW);
		}
	}

	/**
	 * @brief Registers interrupt handler.
	 * @param iCap - capture module
	 * @param handler - interrupt handler
	 * @return (none)
	 */
	void registerInterruptHandler(CapModule module, void (*handler)(void)) const
	{
		for (int i = 0; i < ChannelCount; ++i)
		{
			if (m_module.instance[i] == module)
			{
				Interrupt_register(m_module.pieIntNum[i], handler);
				Interrupt_enable(m_module.pieIntNum[i]);
				return;
			}
		}
	}
};


/// @}
} // namespace mcu


