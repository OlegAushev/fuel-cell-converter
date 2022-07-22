/**
 * @defgroup mcu_qep QEP
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_qep
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "../gpio/mcu_gpio.h"
#include "emb/emb_common.h"


namespace mcu {
/// @addtogroup mcu_pwm
/// @{


/// QEP modules
enum QepModule
{
	QEP1,
	QEP2,
	QEP3
};


/// QEP position counter input mode
enum QepInputMode
{
	QEP_QUADRATURE = EQEP_CONFIG_QUADRATURE,
	QEP_CLOCK_DIR = EQEP_CONFIG_CLOCK_DIR,
	QEP_UP_COUNT = EQEP_CONFIG_UP_COUNT,
	QEP_DOWN_COUNT = EQEP_CONFIG_DOWN_COUNT
};


/// QEP resolution
enum QepResolution
{
	QEP_2X_RESOLUTION = EQEP_CONFIG_2X_RESOLUTION,	//!< Count rising and falling edge
	QEP_1X_RESOLUTION = EQEP_CONFIG_1X_RESOLUTION	//!< Count rising edge only
};


/// QEP swap QEPA and QEPB pins mode
enum QepSwapAB
{
	QEP_AB_NO_SWAP = EQEP_CONFIG_NO_SWAP,
	QEP_AB_SWAP = EQEP_CONFIG_SWAP
};


/// QEP position counter operating (position reset) mode
enum QepPositionResetMode
{
	QEP_POSITION_RESET_ON_IDX = EQEP_POSITION_RESET_IDX,
	QEP_POSITION_RESET_ON_MAX = EQEP_POSITION_RESET_MAX_POS,
	QEP_POSITION_RESET_ON_1ST_IDX = EQEP_POSITION_RESET_1ST_IDX,
	QEP_POSITION_RESET_ON_TIMEOUT = EQEP_POSITION_RESET_UNIT_TIME_OUT
};


/**
 * @brief QEP unit config.
 */
struct QepConfig
{
	QepInputMode inputMode;
	QepResolution resolution;
	QepSwapAB swapAB;
	QepPositionResetMode resetMode;
	uint32_t maxPosition;
	uint32_t timeoutFreq;
	uint32_t latchMode;
	uint16_t initMode;
	uint32_t initPosition;
	EQEP_CAPCLKPrescale capPrescaler;
	EQEP_UPEVNTPrescale eventPrescaler;
	uint16_t intFlags;
};


namespace detail {


/**
 * @brief QEP module implementation.
 */
struct QepModuleImpl
{
	uint32_t base;
	uint16_t intFlags;
	uint32_t pieIntNum;
	QepModuleImpl(uint32_t _base, uint16_t _intFlags, uint32_t _pieIntNum)
		: base(_base), intFlags(_intFlags), pieIntNum(_pieIntNum) {}
};


extern const uint32_t qepBases[3];
extern const uint32_t qepPieIntNums[3];


} // namespace detail


/**
 * @brief QEP unit class.
 */
template <QepModule Module>
class Qep : public emb::c28x::Singleton<Qep<Module> >
{
private:
	detail::QepModuleImpl m_module;

private:
	Qep(const Qep& other);			// no copy constructor
	Qep& operator=(const Qep& other);	// no copy assignment operator
public:
	/**
	 * @brief Initializes MCU QEP unit.
	 * @param (none)
	 */
	Qep(const GpioConfig& qepaPin, const GpioConfig& qepbPin,
			const GpioConfig& qepiPin, const QepConfig& cfg)
		: emb::c28x::Singleton<Qep<Module> >(this)
		, m_module(detail::qepBases[Module], cfg.intFlags, detail::qepPieIntNums[Module])
	{
#ifdef CPU1
		_initPins(qepaPin, qepbPin, qepiPin);
#endif

		// Configure the decoder
		EQEP_setDecoderConfig(m_module.base,
				static_cast<uint16_t>(cfg.inputMode)
				| static_cast<uint16_t>(cfg.resolution)
				| static_cast<uint16_t>(cfg.swapAB));
		EQEP_setEmulationMode(m_module.base, EQEP_EMULATIONMODE_RUNFREE);

		// Configure the position counter to reset on an index event
		EQEP_setPositionCounterConfig(m_module.base, static_cast<EQEP_PositionResetMode>(cfg.resetMode), cfg.maxPosition);

		// Configure initial position
		EQEP_setPositionInitMode(m_module.base, cfg.initMode);
		EQEP_setInitialPosition(m_module.base, cfg.initPosition);

		// Enable the unit timer, setting the frequency
		EQEP_enableUnitTimer(m_module.base, (mcu::sysclkFreq() / cfg.timeoutFreq));

		// Configure the position counter to be latched on a unit time out
		EQEP_setLatchMode(m_module.base, cfg.latchMode);

		// Enable the eQEP1 module
		EQEP_enableModule(m_module.base);

		// Configure and enable the edge-capture unit.
		EQEP_setCaptureConfig(m_module.base, cfg.capPrescaler, cfg.eventPrescaler);
		EQEP_enableCapture(m_module.base);
	}

	/**
	 * @brief Returns base of QEP-unit.
	 * @param (none)
	 * @return Base of QEP-unit.
	 */
	uint32_t base() const { return m_module.base; }

	/**
	 * @brief Registers interrupt handler.
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	void registerInterruptHandler(void (*handler)(void)) const
	{
		Interrupt_register(m_module.pieIntNum, handler);
		EQEP_enableInterrupt(m_module.base, m_module.intFlags);
	}

	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts() const
	{
		Interrupt_enable(m_module.pieIntNum);
	}

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts() const
	{
		Interrupt_disable(m_module.pieIntNum);
	}

protected:
#ifdef CPU1
	static void _initPins(const GpioConfig& qepaPin, const GpioConfig& qepbPin,
			const GpioConfig& qepiPin)
	{
		GPIO_setPadConfig(qepaPin.no, GPIO_PIN_TYPE_STD);
		GPIO_setPinConfig(qepaPin.mux);

		if (qepbPin.valid)
		{
			GPIO_setPadConfig(qepbPin.no, GPIO_PIN_TYPE_STD);
			GPIO_setPinConfig(qepbPin.mux);
		}

		if (qepiPin.valid)
		{
			GPIO_setPadConfig(qepiPin.no, GPIO_PIN_TYPE_STD);
			GPIO_setPinConfig(qepiPin.mux);
		}
	}
#endif
};


/// @}
} // namespace mcu


