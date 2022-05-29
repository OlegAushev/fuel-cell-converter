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
#include "mcu/system/mcusystem.h"
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

/**
 * @brief DAC module implementation.
 */
struct DacModuleImpl
{
	const uint32_t base;
};

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
class DacUnit : public emb::c28x::Singleton<DacUnit<Module> >
{
private:
	DacUnit(const DacUnit& other);			// no copy constructor
	DacUnit& operator=(const DacUnit& other);	// no copy assignment operator
public:
	/// DAC module
	static DacModuleImpl module;

	/**
	 * @brief Initializes MCU DAC unit.
	 * @param (none)
	 */
	DacUnit();

	/**
	 * @brief Starts DAC.
	 * @param value - value to be converted.
	 * @return (none)
	 */
	void convert(DacInput input)
	{
		DAC_setShadowValue(module.base, input.value());
	}
};


/// @}
} /* namespace mcu */



