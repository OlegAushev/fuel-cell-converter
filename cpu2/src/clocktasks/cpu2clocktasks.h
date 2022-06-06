///
#pragma once


#include "mcu/system/mcusystem.h"
#include "mcu/support/mcusupport.h"
#include "mcu/cputimers/mcucputimers.h"
#include "mcu/spi/mcuspi.h"
#include "emb/emb_pair.h"
#include "emb/emb_math.h"


/**
 * @brief Toggling led task.
 * @return Task execution status.
 */
mcu::ClockTaskStatus taskToggleLed();


