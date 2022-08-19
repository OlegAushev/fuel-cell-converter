/**
 * @file
 * @ingroup cli
 */


#include "cli_shell.h"

#include "mcu_c28x/system/mcu_system.h"
#include "mcu_c28x/cputimers/mcu_cputimers.h"


int cli_sysinfo(int argc, const char** argv);
int cli_reboot(int argc, const char** argv);
int cli_uptime(int argc, const char** argv);


namespace cli {


Cmd Shell::COMMANDS[] =
{
{"list",		Shell::list, 		"Prints all available commands."},
{"sysinfo",		cli_sysinfo,		"Prints basic information about system."},
{"reboot",		cli_reboot,		"Reboots device."},
{"uptime",		cli_uptime,		"Shows system uptime."},
};

const size_t Shell::COMMANDS_COUNT = sizeof(Shell::COMMANDS) / sizeof(Shell::COMMANDS[0]);
Cmd* Shell::COMMANDS_END = Shell::COMMANDS + Shell::COMMANDS_COUNT;


///
///
///
void Shell::init()
{
	std::sort(COMMANDS, COMMANDS_END);
}


///
///
///
int Shell::exec(int argc, const char** argv)
{
	if (argc == 0) return 0;

	const Cmd* cmd = emb::binary_find(COMMANDS, COMMANDS_END, argv[0]);
	if (cmd == COMMANDS_END)
	{
		cli::print(argv[0]);
		cli::print(": command not found");
		return -1;
	}

	if (argc == 1)
	{
		return cmd->exec(--argc, ++argv);
	}
	else if (strcmp(argv[1], "--help") == 0)
	{
		cli::print(cmd->help);
		return 0;
	}
	else
	{
		return cmd->exec(--argc, ++argv);
	}
}


///
///
///
int Shell::list(int argc, const char** argv)
{
	cli::print("Available commands are:");
	for (size_t i = 0; i < COMMANDS_COUNT; ++i)
	{
		cli::nextline();
		cli::print(COMMANDS[i].name);
	}
	return 0;
}















} // namespace cli


