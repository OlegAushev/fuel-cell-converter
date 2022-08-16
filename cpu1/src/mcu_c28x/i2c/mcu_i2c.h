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
#include "../system/mcu_system.h"
#include "../gpio/mcu_gpio.h"
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


namespace detail {


/**
 * @brief I2C module implementation
 */
struct I2CModuleImpl
{
	uint32_t base;
	I2CModuleImpl(uint32_t _base) : base(_base) {}
};


extern const uint32_t i2cBases[2];


} // namespace detail


/**
 * @brief I2C unit class.
 */
template <I2CModule Module>
class I2C : public emb::c28x::Singleton<I2C<Module> >
{
private:
	detail::I2CModuleImpl m_module;

private:
	I2C(const I2C& other);			// no copy constructor
	I2C& operator=(const I2C& other);	// no copy assignment operator
public:
	/**
	 * @brief Initializes MCU I2C unit.
	 * @param sdaPin - MCU I2C-SDA pin config
	 * @param sclPin - MCU I2C-SCL pin config
	 * @param cfg - I2C config
	 */
	I2C(const GpioConfig& sdaPin, const GpioConfig& sclPin, const I2CConfig& cfg)
		: emb::c28x::Singleton<I2C<Module> >(this)
		, m_module(detail::i2cBases[Module])
	{
#ifdef CPU1
		_initPins(sdaPin, sclPin);
#endif
		I2C_disableModule(m_module.base);

		I2C_initMaster(m_module.base, mcu::sysclkFreq(), cfg.bitrate, static_cast<I2C_DutyCycle>(cfg.dutyCycle));
		I2C_setBitCount(m_module.base, static_cast<I2C_BitCount>(cfg.bitCount));
		I2C_setSlaveAddress(m_module.base, cfg.slaveAddr);
		I2C_setEmulationMode(m_module.base, I2C_EMULATION_FREE_RUN);

		I2C_disableFIFO(m_module.base);
		I2C_enableModule(m_module.base);
	}

#ifdef CPU1
	/**
	 * @brief Transfers control over I2C unit to CPU2.
	 * @param sdaPin - MCU I2C-SDA pin config
	 * @param sclPin - MCU I2C-SCL pin config
	 * @return (none)
	 */
	static void transferControlToCpu2(const GpioConfig& sdaPin, const GpioConfig& sclPin)
	{
		_initPins(sdaPin, sclPin);
		GPIO_setMasterCore(sdaPin.no, GPIO_CORE_CPU2);
		GPIO_setMasterCore(sclPin.no, GPIO_CORE_CPU2);

		SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL7_I2C, static_cast<uint16_t>(Module)+1, SYSCTL_CPUSEL_CPU2);
	}
#endif

	/**
	 * @brief Returns base of I2C-unit.
	 * @param (none)
	 * @return Base of I2C-unit.
	 */
	uint32_t base() const { return m_module.base; }

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
	void enable() { I2C_enableModule(m_module.base); }

	/**
	 * @brief Disables unit.
	 * @param (none)
	 * @return (none)
	 */
	void disable() { I2C_disableModule(m_module.base); }

protected:
#ifdef CPU1
	static void _initPins(const GpioConfig& sdaPin, const GpioConfig& sclPin)
	{
		GPIO_setPadConfig(sdaPin.no, GPIO_PIN_TYPE_PULLUP);
		GPIO_setQualificationMode(sdaPin.no, GPIO_QUAL_ASYNC);
		GPIO_setPinConfig(sdaPin.mux);

		GPIO_setPadConfig(sclPin.no, GPIO_PIN_TYPE_PULLUP);
		GPIO_setQualificationMode(sclPin.no, GPIO_QUAL_ASYNC);
		GPIO_setPinConfig(sclPin.mux);
	}
#endif
};


/// @}
} // namespace mcu


