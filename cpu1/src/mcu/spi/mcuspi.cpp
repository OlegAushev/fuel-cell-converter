/**
 * @file
 * @ingroup mcu mcu_spi
 */


#include "mcuspi.h"


namespace mcu {

namespace detail {
const uint32_t spiBases[3] = {SPIA_BASE, SPIB_BASE, SPIC_BASE};
const uint32_t spiRxPieIntNos[3] = {INT_SPIA_RX, INT_SPIB_RX, INT_SPIC_RX};
}


} /* namespace mcu */


