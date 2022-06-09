/**
 * @defgroup setup_manger Setup Manager
 *
 * @file
 * @ingroup setup_manager
 */


#pragma once


#include "boostconverter/boostconverter.h"


/// @addtogroup setup_manager
/// @{


/**
 * @brief Setup manager class.
 */
class SetupManager
{
	friend class SetupManagerTest;
public:
	/// System config.
	struct SystemConfig
	{
		BoostConverterConfig CONVERTER_CONFIG;
		mcu::PwmConfig<mcu::PWM_ONE_PHASE> PWM_CONFIG;
	};

private:
	BoostConverter* m_converter;

	SetupManager(const SetupManager& other);		// no copy constructor
	SetupManager& operator=(const SetupManager& other);	// no copy assignment operator
public:
	static const SystemConfig DEFAULT_CONFIG;
	static SystemConfig SYSTEM_CONFIG;

	/**
	 * @brief Creates SetupManager object.
	 * @param (none)
	 */
	SetupManager();

	/**
	 * @brief Registers controlled drive object.
	 * @param drive - pointer to acim::Drive object
	 * @return (none)
	 */
	void registerConverter(BoostConverter* converter);
};


/// @}


