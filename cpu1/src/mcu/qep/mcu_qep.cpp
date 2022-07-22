/**
 * @file
 * @ingroup mcu mcu_qep
 */


#include "mcu_qep.h"


namespace mcu {


namespace detail {


const uint32_t qepBases[3] = {EQEP1_BASE, EQEP2_BASE, EQEP3_BASE};
const uint32_t qepPieIntNums[3] = {INT_EQEP1, INT_EQEP2, INT_EQEP3};


}


} // namespace mcu


