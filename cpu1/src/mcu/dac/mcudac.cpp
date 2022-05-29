/**
 * @file
 * @ingroup mcu mcu_dac
 */


#include "mcudac.h"


namespace mcu {

DacModuleImpl DacUnit<DACA>::module = {.base = DACA_BASE};
DacModuleImpl DacUnit<DACB>::module = {.base = DACB_BASE};
DacModuleImpl DacUnit<DACC>::module = {.base = DACC_BASE};

///
///
///
template <DacModule Module>
DacUnit<Module>::DacUnit()
	: emb::c28x::Singleton<DacUnit<Module> >(this)
{
	DAC_setReferenceVoltage(module.base, DAC_REF_ADC_VREFHI);
	DAC_enableOutput(module.base);
	DAC_setShadowValue(module.base, 0);
	mcu::delay_us(10);	// Delay for buffered DAC to power up
}
// Explicit instantiation
template DacUnit<DACA>::DacUnit();
template DacUnit<DACB>::DacUnit();
template DacUnit<DACC>::DacUnit();


} /* namespace mcu */
