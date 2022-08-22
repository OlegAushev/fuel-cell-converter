///
#pragma once


#include "mcu/system/mcu_system.h"
#include "mcu/support/mcu_support.h"
#include "mcu/cputimers/mcu_cputimers.h"
#include "mcu/spi/mcu_spi.h"
#include "emb/emb_pair.h"
#include "emb/emb_math.h"


/**
 * @brief Toggling led task.
 * @return Task execution status.
 */
mcu::ClockTaskStatus taskToggleLed();


