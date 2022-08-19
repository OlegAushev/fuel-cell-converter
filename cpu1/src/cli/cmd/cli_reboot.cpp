/**
 * @file
 * @ingroup cli
 */


#pragma once


#include "cli/shell/cli_shell.h"

#include "mcu_c28x/cputimers/mcu_cputimers.h"
#include "mcu_c28x/system/mcu_system.h"


int cli_reboot(int argc, const char** argv)
{
	mcu::SystemClock::registerDelayedTask(mcu::resetDevice, 2000);
	cli::print("Device will reboot in 2 seconds...");
	return 0;
}


