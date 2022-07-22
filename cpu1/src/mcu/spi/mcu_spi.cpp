/**
 * @file
 * @ingroup mcu mcu_spi
 */


#include "mcu_spi.h"


namespace mcu {


namespace detail {


const uint32_t spiBases[3] = {SPIA_BASE, SPIB_BASE, SPIC_BASE};
const uint32_t spiRxPieIntNums[3] = {INT_SPIA_RX, INT_SPIB_RX, INT_SPIC_RX};


}


} // namespace mcu


