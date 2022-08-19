/**
 * @defgroup cli CLI
 *
 * @file
 * @ingroup cli
 */


#pragma once


#define CLI_PROMPT_MAX_LENGTH 32

#define CLI_DEVICE_NAME_MAX_LENGTH 16

#define CLI_CMDLINE_MAX_LENGTH 64

#define CLI_ESCSEQ_MAX_LENGTH 16

#define CLI_OUTBUT_BUFFER_LENGTH 256

#define CLI_TOKEN_MAX_COUNT 8
#define CLI_TOKEN_MAX_LENGHT 16


#define CLI_ESC		"\x1B"
#define CLI_ENDL	"\r\n"

#define CLI_COLOR_OFF		"\033[0m"	// Text Reset
#define CLI_COLOR_GREEN 	"\033[1;32m"	// Green
#define CLI_COLOR_YELLOW	"\033[1;33m"	// Yellow
#define CLI_COLOR_PURPLE	"\033[1;35m"	// Purple

#define CLI_PROMPT_BEGIN CLI_COLOR_PURPLE"[root@"
#define CLI_PROMPT_END "]> "CLI_COLOR_OFF


