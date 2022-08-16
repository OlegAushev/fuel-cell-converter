///
#pragma once


#include "F28x_Project.h"
#include "F2837xD_Ipc_drivers.h"
#include "mcu_c28x/support/mcu_support.h"
#include "mcu_c28x/cputimers/mcu_cputimers.h"
#include "sys/syslog/syslog.h"
#include "fuelcell/converter/fuelcell_converter.h"
#include "fuelcell/controller/fuelcell_controller.h"


/**
 * @brief Task performed at watchdog timeout.
 * @param (none)
 * @return Task execution status.
 */
mcu::ClockTaskStatus taskWatchdogTimeout();


/**
 * @brief Toggling led task.
 * @param (none)
 * @return Task execution status.
 */
mcu::ClockTaskStatus taskToggleLed();


/**
 * @brief Start temperature sensors task.
 * @param (none)
 * @return Task execution status.
 */
mcu::ClockTaskStatus taskStartTempSensors();



/**
 * @brief Start fuel cells errors check.
 * @param (none)
 * @return Task execution status.
 */
mcu::ClockTaskStatus taskCheckFuelcellErrors();

