/**
 * @file
 * @ingroup cli
 */


#pragma once


#include "cli/shell/cli_shell.h"

#include "sys/syslog/syslog.h"


int cli_syslog(int argc, const char** argv)
{
	char str[32];
	if (argc == 0)
	{
		strncpy(str, "Options not specified.", 32);
		goto cli_syslog_print;
	}

	if (strcmp(argv[0], "show") == 0)
	{
		if (argc == 1)
		{
			strncpy(str, "syslog-show: options not specified", 32);
			goto cli_syslog_print;
		}

		if (strcmp(argv[1], "errors") == 0)
		{
			snprintf(str, 32, "errors: 0x%08lX", Syslog::errors());
			goto cli_syslog_print;
		}

		if (strcmp(argv[1], "warnings") == 0)
		{
			snprintf(str, 32, "warnings: 0x%08lX", Syslog::warnings());
			goto cli_syslog_print;
		}
	}

cli_syslog_print:
	cli::print(str);
	return 0;
}


