/**
 * @defgroup mco_od Object Dictionary
 * @ingroup microcanopen
 *
 * @file
 * @ingroup microcanopen mco_od
 */


#pragma once

#include "driverlib.h"
#include "device.h"

#include "../mcodef.h"
#include "emb/emb_math.h"
#include "mcu/system/mcusystem.h"
#include "mcu/cputimers/mcucputimers.h"
#include "syslog/syslog.h"
#include "boostconverter/boostconverter.h"


namespace microcanopen {

/**
 * @ingroup mco_od
 * @brief MicroCANopen object dictionary
 */
extern ODEntry OBJECT_DICTIONARY[];

/**
 * @ingroup mco_od
 * @brief MicroCANopen object dictionary size
 */
extern const size_t OD_SIZE;

/**
 * @ingroup mco_od
 * @brief Pointer to MicroCANopen object dictionary end.
 */
extern ODEntry* OBJECT_DICTIONARY_END;

namespace od {
/// @addtogroup mco_od
/// @{


extern BoostConverter* converter;


/// @}
} // namespace od
} // namespace microcanopen





