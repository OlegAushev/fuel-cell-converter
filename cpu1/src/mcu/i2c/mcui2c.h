/**
 * @defgroup mcu_i2c I2C
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_i2c
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "emb/emb_common.h"


namespace mcu {
/// @addtogroup mcu_i2c
/// @{


/// I2C modules
enum I2CModule
{
	I2CA,
	I2CB
};


namespace tag {
/// tag - use I2CA
struct use_i2ca {};
/// tag - use I2CB
struct use_i2cb {};
}

/// I2C SDA pins
enum I2CSdaPin
{
	I2CA_SDA_GPIO_32,
	I2CA_SDA_GPIO_104,
	I2CB_SDA_GPIO_40
};

/// I2C SCL pins
enum I2CSclPin
{
	I2CA_SCL_GPIO_33,
	I2CA_SCL_GPIO_105,
	I2CB_SCL_GPIO_41
};

/// Count of bits per data word.
enum I2CBitCount
{
	I2C_BITCOUNT_1 = ::I2C_BITCOUNT_1,
	I2C_BITCOUNT_2 = ::I2C_BITCOUNT_2,
	I2C_BITCOUNT_3 = ::I2C_BITCOUNT_3,
	I2C_BITCOUNT_4 = ::I2C_BITCOUNT_4,
	I2C_BITCOUNT_5 = ::I2C_BITCOUNT_5,
	I2C_BITCOUNT_6 = ::I2C_BITCOUNT_6,
	I2C_BITCOUNT_7 = ::I2C_BITCOUNT_7,
	I2C_BITCOUNT_8 = ::I2C_BITCOUNT_8
};

/// I2C clock duty cycle.
enum I2CDutyCycle
{
    I2C_DUTYCYCLE_33 = ::I2C_DUTYCYCLE_33,
    I2C_DUTYCYCLE_50 = ::I2C_DUTYCYCLE_50
};

/**
 * @brief I2C unit config.
 */
struct I2CConfig
{
	uint32_t bitrate;
	I2CBitCount bitCount;
	I2CDutyCycle dutyCycle;
	uint16_t slaveAddr;
};

/**
 * @brief I2C module implementation
 */
struct I2CModuleImpl
{
	const uint32_t base;
	uint32_t sdaPin;
	uint32_t sdaPinMux;
	uint32_t sclPin;
	uint32_t sclPinMux;
};

namespace detail {
extern const uint32_t i2cSdaPins[3];
extern const uint32_t i2cSdaPinMuxs[3];
extern const uint32_t i2cSclPins[3];
extern const uint32_t i2cSclPinMuxs[3];
}

/**
 * @brief I2C unit class.
 */
template <I2CModule Module>
class I2CUnit : public emb::c28x::Singleton<I2CUnit<Module> >
{
private:
	I2CUnit(const I2CUnit& other);			// no copy constructor
	I2CUnit& operator=(const I2CUnit& other);	// no copy assignment operator
public:
	/// I2C module
	static I2CModuleImpl module;

	/**
	 * @brief Initializes MCU I2C unit.
	 * @param sdaPin - MCU I2C-SDA pin
	 * @param sclPin - MCU I2C-SCL pin
	 * @param slaveAddr - slave address
	 */
	I2CUnit(I2CSdaPin sdaPin, I2CSclPin sclPin, const I2CConfig& cfg);

#ifdef CPU1
	/**
	 * @brief Transfers control over I2C unit to CPU2.
	 * @param sdaPin - MCU I2C-SDA pin
	 * @param sclPin - MCU I2C-SCL pin
	 * @return (none)
	 */
	static void transferControlToCpu2(I2CSdaPin sdaPin, I2CSclPin sclPin);
#endif

	/**
	 * @brief Sets slave address.
	 * @param slaveAddr - slave address
	 * @return (none)
	 */
	void setSlaveAddress(uint16_t slaveAddr) { I2C_setSlaveAddress(module.base, slaveAddr); }

	/**
	 * @brief Enables unit.
	 * @param (none)
	 * @return (none)
	 */
	void enable() { I2C_enableModule(module.base); }

	/**
	 * @brief Disables unit.
	 * @param (none)
	 * @return (none)
	 */
	void disable() { I2C_disableModule(module.base); }
};


/// @}
} /* namespace mcu */



