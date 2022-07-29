/**
 * @defgroup settings Settings
 *
 * @file
 * @ingroup settings
 */


#pragma once


#include "fuelcell/converter/fuelcell_converter.h"


/// @addtogroup settings
/// @{


/**
 * @brief Settings  class.
 */
class Settings : public emb::Monostate<Settings>
{
	friend class SettingsTest;
public:
	/// System config
	struct SystemConfig
	{
		fuelcell::ConverterConfig CONVERTER_CONFIG;
		mcu::PwmConfig<mcu::PWM_ONE_PHASE> PWM_CONFIG;
	};

	static const SystemConfig DEFAULT_CONFIG;
	static SystemConfig SYSTEM_CONFIG;

private:
	static fuelcell::Converter* m_converter;

	Settings(const Settings& other);		// no copy constructor
	Settings& operator=(const Settings& other);	// no copy assignment operator

public:
	/**
	 * @brief Initializes Settings.
	 * @param (none)
	 * @return (none)
	 */
	static void init()
	{
		if (initialized()) return;

		m_converter = static_cast<fuelcell::Converter*>(NULL);

		setInitialized();
	}

	/**
	 * @brief Settings object constructor.
	 * @param (none)
	 */
	Settings() {}

	/**
	 * @brief Registers controlled objects.
	 * @param converter - pointer to BoostConverter object
	 * @return (none)
	 */
	void registerObjects(fuelcell::Converter* converter)
	{
		m_converter = converter;
	}
};


/// @}


