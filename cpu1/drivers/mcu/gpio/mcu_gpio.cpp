/**
 * @file
 * @ingroup mcu mcu_gpio
 */


#include "mcu_gpio.h"


namespace mcu {


namespace detail {


const uint32_t PIE_XINT_NUMBERS[5] = {INT_XINT1, INT_XINT2, INT_XINT3, INT_XINT4, INT_XINT5};
const uint16_t PIE_XINT_GROUPS[5] = {INTERRUPT_ACK_GROUP1, INTERRUPT_ACK_GROUP1,
		INTERRUPT_ACK_GROUP12, INTERRUPT_ACK_GROUP12, INTERRUPT_ACK_GROUP12};


}


} // namespace mcu


