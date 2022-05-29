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

/// QEP A pins
enum QepAPin
{
	QEP1_A_GPIO_20,
	QEP2_A_GPIO_54,
};

/// QEP B pins
enum QepBPin
{
	QEPX_B_NC,
	QEP1_B_GPIO_21,
	QEP2_B_GPIO_55,
};

/// QEP I pins
enum QepIPin
{
	QEPX_I_NC,
	QEP1_I_GPIO_23,
	QEP2_I_GPIO_57,
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
	uint32_t interruptFreq;
	uint32_t latchMode;
	uint16_t initMode;
	uint32_t initPosition;
};

/**
 * @brief QEP module implementation.
 */
struct QepModuleImpl
{
	const uint32_t base;
	uint32_t qepaPin;
	uint32_t qepaPinMux;
	uint32_t qepbPin;
	uint32_t qepbPinMux;
	uint32_t qepiPin;
	uint32_t qepiPinMux;
};

/**
 * @brief QEP unit class.
 */
template <QepModule Module>
class QepUnit : public emb::c28x::Singleton<QepUnit<Module> >
{
private:
	QepUnit(const QepUnit& other);			// no copy constructor
	QepUnit& operator=(const QepUnit& other);	// no copy assignment operator

public:
	/// QEP module
	static QepModuleImpl module;

	/**
	 * @brief Initializes MCU QEP unit.
	 * @param (none)
	 */
	QepUnit(QepAPin qepaPin, QepBPin qepbPin, QepIPin qepiPin, const QepConfig& cfg);
};








/// @}
} /* namespace mcu */


