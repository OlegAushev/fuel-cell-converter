/**
 * @defgroup mcu_dac DAC
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_dac
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "mcu/system/mcu_system.h"
#include "emb/emb_common.h"


namespace mcu {
/// @addtogroup mcu_dac
/// @{


/// DAC modules
enum DacModule
{
	DACA,/**< DACA */
	DACB,/**< DACB */
	DACC /**< DACC */
};


namespace detail {


/**
 * @brief DAC module implementation.
 */
struct DacModuleImpl
{
	uint32_t base;
	DacModuleImpl(uint32_t _base) : base(_base) {}
};


extern const uint32_t dacBases[3];


} // namespace detail


/**
 * @brief DAC input class.
 */
class DacInput
{
private:
	uint16_t m_tag : 4;
	uint16_t m_value : 12;
public:
	DacInput()
		: m_tag(0)
		, m_value(0)
	{}

	explicit DacInput(uint16_t value)
		: m_tag(0)
		, m_value(value & 0x0FFF)
	{}

	DacInput(uint16_t value, DacModule module)
		: m_tag(static_cast<uint16_t>(module))
		, m_value(value & 0x0FFF)
	{}

	uint16_t value() const { return m_value; }
	uint16_t tag() const { return m_tag; }
};


/**
 * @brief DAC unit class.
 */
template <DacModule Module>
class Dac : public emb::c28x::Singleton<Dac<Module> >
{
private:
	detail::DacModuleImpl m_module;

private:
	Dac(const Dac& other);			// no copy constructor
	Dac& operator=(const Dac& other);	// no copy assignment operator
public:
	/**
	 * @brief Initializes MCU DAC unit.
	 * @param (none)
	 */
	Dac()
		: emb::c28x::Singleton<Dac<Module> >(this)
		, m_module(detail::dacBases[Module])
	{
		DAC_setReferenceVoltage(m_module.base, DAC_REF_ADC_VREFHI);
		DAC_enableOutput(m_module.base);
		DAC_setShadowValue(m_module.base, 0);
		mcu::delay_us(10);	// Delay for buffered DAC to power up
	}

	/**
	 * @brief Starts DAC.
	 * @param value - value to be converted.
	 * @return (none)
	 */
	void convert(DacInput input)
	{
		DAC_setShadowValue(m_module.base, input.value());
	}
};


/// @}
} // namespace mcu


