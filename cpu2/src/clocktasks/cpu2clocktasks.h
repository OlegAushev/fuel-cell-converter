///
#pragma once


#include "mcu_c28x/system/mcu_system.h"
#include "mcu_c28x/support/mcu_support.h"
#include "mcu_c28x/cputimers/mcu_cputimers.h"
#include "mcu_c28x/spi/mcu_spi.h"
#include "emb/emb_pair.h"
#include "emb/emb_math.h"


/**
 * @brief Toggling led task.
 * @return Task execution status.
 */
mcu::ClockTaskStatus taskToggleLed();


