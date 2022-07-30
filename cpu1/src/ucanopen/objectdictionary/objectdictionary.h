/**
 * @defgroup ucanopen_od Object Dictionary
 * @ingroup ucanopen
 *
 * @file
 * @ingroup ucanopen ucanopen_od
 */


#pragma once

#include "driverlib.h"
#include "device.h"

#include "../ucanopen_def.h"
#include "emb/emb_math.h"
#include "mcu/system/mcu_system.h"
#include "mcu/cputimers/mcu_cputimers.h"

// APP-SPECIFIC headers
#include "syslog/syslog.h"
#include "fuelcell/converter/fuelcell_converter.h"
#include "fuelcell/controller/fuelcell_controller.h"


namespace ucanopen {

/**
 * @ingroup ucanopen_od
 * @brief uCANopen object dictionary
 */
extern ODEntry OBJECT_DICTIONARY[];

/**
 * @ingroup ucanopen_od
 * @brief uCANopen object dictionary size
 */
extern const size_t OD_SIZE;

/**
 * @ingroup ucanopen_od
 * @brief Pointer to uCANopen object dictionary end.
 */
extern ODEntry* OBJECT_DICTIONARY_END;

namespace od {
/// @addtogroup ucanopen_od
/// @{


// APP-SPECIFIC objects
extern fuelcell::Converter* converter;


/// @}
} // namespace od
} // namespace ucanopen





