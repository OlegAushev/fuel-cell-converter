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

#include "../mco_def.h"
#include "emb/emb_math.h"
#include "mcu/system/mcu_system.h"
#include "mcu/cputimers/mcu_cputimers.h"

// APP-SPECIFIC headers
#include "syslog/syslog.h"
#include "fuelcell/converter/fuelcell_converter.h"
#include "fuelcell/controller/fuelcell_controller.h"


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


// APP-SPECIFIC objects
extern fuelcell::Converter* converter;


/// @}
} // namespace od
} // namespace microcanopen





