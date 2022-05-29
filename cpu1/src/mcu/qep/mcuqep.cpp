/**
 * @file
 * @ingroup mcu mcu_qep
 */


#include "mcuqep.h"


namespace mcu {

namespace detail {
const uint32_t qepaPins[2] = 	{20, 54};
const uint32_t qepaPinMuxs[2] =	{GPIO_20_EQEP1A, GPIO_54_EQEP2A};
const uint32_t qepbPins[3] =	{0, 21, 55};
const uint32_t qepbPinMuxs[3] =	{0, GPIO_21_EQEP1B, GPIO_55_EQEP2B};
const uint32_t qepiPins[3] =	{0, 23, 57};
const uint32_t qepiPinMuxs[3] =	{0, GPIO_23_EQEP1I, GPIO_57_EQEP2I};
}

QepModuleImpl QepUnit<QEP1>::module =
{
	.base = EQEP1_BASE,
};

QepModuleImpl QepUnit<QEP2>::module =
{
	.base = EQEP2_BASE,
};

QepModuleImpl QepUnit<QEP3>::module =
{
	.base = EQEP3_BASE,
};

///
///
///
template <QepModule Module>
QepUnit<Module>::QepUnit(QepAPin qepaPin, QepBPin qepbPin, QepIPin qepiPin, const QepConfig& cfg)
	: emb::c28x::Singleton<QepUnit<Module> >(this)
{
	module.qepaPin = detail::qepaPins[qepaPin];
	module.qepaPinMux = detail::qepaPinMuxs[qepaPin];
	module.qepbPin = detail::qepbPins[qepbPin];
	module.qepbPinMux = detail::qepbPinMuxs[qepbPin];
	module.qepiPin = detail::qepiPins[qepiPin];
	module.qepiPinMux = detail::qepiPinMuxs[qepiPin];


	GPIO_setPadConfig(module.qepaPin, GPIO_PIN_TYPE_STD);
	GPIO_setPinConfig(module.qepaPinMux);

	if ((cfg.inputMode != QEP_UP_COUNT) && (cfg.inputMode != QEP_DOWN_COUNT))
	{
		if (qepbPin != QEPX_B_NC)
		{
			GPIO_setPadConfig(module.qepbPin, GPIO_PIN_TYPE_STD);
			GPIO_setPinConfig(module.qepbPinMux);
		}

		if (qepiPin != QEPX_I_NC)
		{
			GPIO_setPadConfig(module.qepiPin, GPIO_PIN_TYPE_STD);
			GPIO_setPinConfig(module.qepiPinMux);
		}
	}

	// Configure the decoder
	EQEP_setDecoderConfig(module.base,
			static_cast<uint16_t>(cfg.inputMode)
			|| static_cast<uint16_t>(cfg.resolution)
			|| static_cast<uint16_t>(cfg.swapAB));
	EQEP_setEmulationMode(module.base, EQEP_EMULATIONMODE_RUNFREE);

	// Configure the position counter to reset on an index event
	EQEP_setPositionCounterConfig(module.base, static_cast<EQEP_PositionResetMode>(cfg.resetMode), cfg.maxPosition);

	// Configure initial position
	EQEP_setPositionInitMode(module.base, cfg.initMode);
	EQEP_setInitialPosition(module.base, cfg.initPosition);


	// Enable the unit timer, setting the frequency
	EQEP_enableUnitTimer(module.base, (DEVICE_SYSCLK_FREQ / cfg.interruptFreq));

	// Configure the position counter to be latched on a unit time out
	EQEP_setLatchMode(module.base, cfg.latchMode);

	// Enable the eQEP1 module
	EQEP_enableModule(module.base);

	// Configure and enable the edge-capture unit. The capture clock divider is
	// SYSCLKOUT/128. The unit-position event divider is QCLK/8.
	EQEP_setCaptureConfig(module.base, EQEP_CAPTURE_CLK_DIV_128, EQEP_UNIT_POS_EVNT_DIV_8);
	EQEP_enableCapture(module.base);
}
// Explicit instantiation
template QepUnit<QEP1>::QepUnit(QepAPin qepaPin, QepBPin qepbPin, QepIPin qepiPin, const QepConfig& cfg);
template QepUnit<QEP2>::QepUnit(QepAPin qepaPin, QepBPin qepbPin, QepIPin qepiPin, const QepConfig& cfg);
template QepUnit<QEP3>::QepUnit(QepAPin qepaPin, QepBPin qepbPin, QepIPin qepiPin, const QepConfig& cfg);









} // namespace mcu


