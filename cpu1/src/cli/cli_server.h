/**
 * @file
 * @ingroup cli
 */


#pragma once


#include <cstring>
#include <cstdio>
#include "emb/emb_interfaces/emb_uart.h"
#include "emb/emb_interfaces/emb_gpio.h"
#include "emb/emb_queue.h"
#include "emb/emb_string.h"
#include "emb/emb_circularbuffer.h"

#include "cli_config.h"


namespace cli {
/// @addtogroup cli
/// @{


/**
 * @brief CLI server.
 */
class Server
{
	friend void print(const char* str);
	friend void print_blocking(const char* str);
private:
	static emb::IUart* s_uart;
	static emb::IGpioOutput* s_pinRTS;
	static emb::IGpioInput* s_pinCTS;

	static char PROMPT[CLI_PROMPT_MAX_LENGTH];
	static emb::String<CLI_CMDLINE_MAX_LENGTH> s_cmdline;
	static emb::String<CLI_ESCSEQ_MAX_LENGTH> s_escseq;

	static size_t s_cursorPos;

	static emb::Queue<char, CLI_OUTBUT_BUFFER_LENGTH> s_outputBuf;

#ifdef CLI_USE_HISTORY
	static emb::CircularBuffer<emb::String<CLI_CMDLINE_MAX_LENGTH>, CLI_HISTORY_LENGTH> s_history;
	static size_t s_lastCmdHistoryPos;
	static size_t s_historyPosition;
	static bool s_newCmdSaved;
#endif

private:
	Server(const Server& other);		// no copy constructor
	Server& operator=(const Server& other);	// no copy assignment operator
public:
	Server(const char* deviceName, emb::IUart* uart, emb::IGpioOutput* pinRTS, emb::IGpioInput* pinCTS);

	void run();
	void registerExecCallback(int (*_exec)(int argc, const char** argv))
	{
		exec = _exec;
	}

private:
	static void print(char ch);
	static void print(const char* str);
	static void printBlocking(const char* str);

	static void processChar(char ch);
	static void saveCursorPos() { print(CLI_ESC"[s"); }
	static void loadCursorPos() { print(CLI_ESC"[u"); }
	static void moveCursor(int offset);
	static void printWelcome();
	static void printPrompt();
	static int tokenize(const char** argv, emb::String<CLI_CMDLINE_MAX_LENGTH>& cmdline);

	static int (*exec)(int argc, const char** argv);
	static int execNull(int argc, const char** argv)
	{
		print(CLI_ENDL"error: exec-callback not registered");
		print(CLI_ENDL"tokens:");
		for (size_t i= 0; i < argc; ++i)
		{
			print(CLI_ENDL);
			print(argv[i]);
		}
		return -1;
	}

public:
	/**
	 * @brief
	 */
	struct EscSeq
	{
		const char* str;
		size_t len;
		void (*handler)();
	};
	static const EscSeq ESCSEQ_LIST[];
private:
	static void _escReturn();
	static void _escMoveCursorLeft();
	static void _escMoveCursorRight();
	static void _escHome();
	static void _escEnd();
	static void _escBack();
	static void _escDel();
	static void _escUp();
	static void _escDown();

private:
#ifdef CLI_USE_HISTORY
	enum HistorySearchDirection
	{
		CLI_HISTORY_SEARCH_UP,
		CLI_HISTORY_SEARCH_DOWN,
	};
	static void searchHistory(HistorySearchDirection dir);
#endif
};


inline void print(const char* str)
{
	Server::print(str);
}


inline void nextline()
{
	print(CLI_ENDL);
}


inline void print_blocking(const char* str)
{
	Server::printBlocking(str);
}


inline void nextline_blocking()
{
	print_blocking(CLI_ENDL);
}


/// @}
} // namespace cli


