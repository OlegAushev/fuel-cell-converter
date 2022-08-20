/**
 * @file
 * @ingroup cli
 */


#pragma once


#include "cli/shell/cli_shell.h"

#include "sys/syslog/syslog.h"


int cli_syslog(int argc, const char** argv)
{
	if (argc == 0)
	{
		strncpy(CLI_OUTPUT, "Options not specified.", CLI_OUTPUT_LENGTH);
		goto cli_syslog_print;
	}

	if (strcmp(argv[0], "show") == 0)
	{
		if (argc == 1)
		{
			strncpy(CLI_OUTPUT, "syslog-show: options not specified", CLI_OUTPUT_LENGTH);
			goto cli_syslog_print;
		}

		if (strcmp(argv[1], "errors") == 0)
		{
			snprintf(CLI_OUTPUT, CLI_OUTPUT_LENGTH, "errors: 0x%08lX", Syslog::errors());
			goto cli_syslog_print;
		}

		if (strcmp(argv[1], "warnings") == 0)
		{
			snprintf(CLI_OUTPUT, CLI_OUTPUT_LENGTH, "warnings: 0x%08lX", Syslog::warnings());
			goto cli_syslog_print;
		}
	}

cli_syslog_print:
	cli::print(CLI_OUTPUT);
	return 0;
}


