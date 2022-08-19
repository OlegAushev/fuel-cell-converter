/**
 * @file
 * @ingroup cli
 */


#pragma once


#include "cli/shell/cli_shell.h"

#include "mcu_c28x/cputimers/mcu_cputimers.h"


int cli_uptime(int argc, const char** argv)
{
	char str[32];

	if (argc == 0)
	{
		snprintf(str, 32, "uptime: %llums", mcu::SystemClock::now());
	}
	else
	{
		if (strcmp(argv[0], "-v") == 0)
		{
			uint64_t sec = mcu::SystemClock::now() / 1000;
			uint64_t msec = mcu::SystemClock::now() - 1000 * sec;
			uint64_t min = sec / 60;
			sec -= 60 * min;

			float uptime = mcu::SystemClock::now() / 1000.f;
			snprintf(str, 32, "uptime: %llum %llus %llums", min, sec, msec);
		}
		else
		{
			snprintf(str, 32, "uptime: invalid option - \"%s\"", argv[0]);
		}
	}

	cli::print(str);
	return 0;
}


