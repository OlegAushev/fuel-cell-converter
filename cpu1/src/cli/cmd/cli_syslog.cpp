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
		strncpy(CLI_CMD_OUTPUT, "Options not specified.", CLI_CMD_OUTPUT_LENGTH);
		goto cli_syslog_print;
	}

	if (strcmp(argv[0], "show") == 0)
	{
		if (argc == 1)
		{
			snprintf(CLI_CMD_OUTPUT, CLI_CMD_OUTPUT_LENGTH, "errors: 0x%08lX\r\n""warnings: 0x%08lX",
					Syslog::errors(),
					Syslog::warnings());
			goto cli_syslog_print;
		}

		if (strcmp(argv[1], "errors") == 0)
		{
			snprintf(CLI_CMD_OUTPUT, CLI_CMD_OUTPUT_LENGTH, "errors: 0x%08lX", Syslog::errors());
			goto cli_syslog_print;
		}

		if (strcmp(argv[1], "warnings") == 0)
		{
			snprintf(CLI_CMD_OUTPUT, CLI_CMD_OUTPUT_LENGTH, "warnings: 0x%08lX", Syslog::warnings());
			goto cli_syslog_print;
		}

		strncpy(CLI_CMD_OUTPUT, "syslog-show: invalid options", CLI_CMD_OUTPUT_LENGTH);
		goto cli_syslog_print;
	}

	if (strcmp(argv[0], "set") == 0)
	{
		if (argc != 3)
		{
			strncpy(CLI_CMD_OUTPUT, "syslog-set: invalid options", CLI_CMD_OUTPUT_LENGTH);
			goto cli_syslog_print;
		}

		if (strcmp(argv[1], "error") == 0)
		{
			Syslog::setError(static_cast<sys::Error::Error>(atoll(argv[2])));
			snprintf(CLI_CMD_OUTPUT, CLI_CMD_OUTPUT_LENGTH, "errors: 0x%08lX", Syslog::errors());
			goto cli_syslog_print;
		}

		if (strcmp(argv[1], "warning") == 0)
		{
			Syslog::setWarning(static_cast<sys::Warning::Warning>(atoll(argv[2])));
			snprintf(CLI_CMD_OUTPUT, CLI_CMD_OUTPUT_LENGTH, "warnings: 0x%08lX", Syslog::warnings());
			goto cli_syslog_print;
		}

		strncpy(CLI_CMD_OUTPUT, "syslog-set: invalid options", CLI_CMD_OUTPUT_LENGTH);
		goto cli_syslog_print;
	}

	if (strcmp(argv[0], "reset") == 0)
	{
		if (argc == 1)
		{
			Syslog::resetErrorsWarnings();
			snprintf(CLI_CMD_OUTPUT, CLI_CMD_OUTPUT_LENGTH, "errors: 0x%08lX\r\n""warnings: 0x%08lX",
					Syslog::errors(),
					Syslog::warnings());
			goto cli_syslog_print;
		}

		if (argc > 3)
		{
			strncpy(CLI_CMD_OUTPUT, "syslog-reset: invalid options", CLI_CMD_OUTPUT_LENGTH);
			goto cli_syslog_print;
		}

		if (strcmp(argv[1], "error") == 0)
		{
			Syslog::resetError(static_cast<sys::Error::Error>(atoll(argv[2])));
			snprintf(CLI_CMD_OUTPUT, CLI_CMD_OUTPUT_LENGTH, "errors: 0x%08lX", Syslog::errors());
			goto cli_syslog_print;
		}

		if (strcmp(argv[1], "warning") == 0)
		{
			Syslog::resetWarning(static_cast<sys::Warning::Warning>(atoll(argv[2])));
			snprintf(CLI_CMD_OUTPUT, CLI_CMD_OUTPUT_LENGTH, "warnings: 0x%08lX", Syslog::warnings());
			goto cli_syslog_print;
		}

		strncpy(CLI_CMD_OUTPUT, "syslog-reset: invalid options", CLI_CMD_OUTPUT_LENGTH);
		goto cli_syslog_print;
	}


cli_syslog_print:
	cli::print(CLI_CMD_OUTPUT);
	return 0;
}


