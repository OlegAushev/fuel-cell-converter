/**
 * @file
 * @ingroup mcu mcu_can
 */


#include "mcucan.h"


namespace mcu {

namespace detail {
const uint32_t canBases[2] = {CANA_BASE, CANB_BASE};
const uint32_t canPieIntNos[2] = {INT_CANA0, INT_CANB0};
}


} /* namespace mcu */


