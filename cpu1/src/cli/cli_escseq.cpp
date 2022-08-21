/**
 * @file
 * @ingroup cli
 */


#include "cli_server.h"


namespace cli {


///
///
///
void Server::_escReturn()
{
#ifdef CLI_USE_HISTORY
	if (!s_cmdline.empty())
	{
		if (s_history.empty())
		{
			s_history.push(s_cmdline);
			s_lastCmdHistoryPos = 0;
		}
		else if (s_cmdline != s_history.back())
		{
			s_history.push(s_cmdline);
			s_lastCmdHistoryPos = (s_lastCmdHistoryPos + 1) % s_history.capacity();
		}
	}
	s_newCmdSaved = true;
	s_historyPosition = s_lastCmdHistoryPos;
#endif

	const char* argv[CLI_TOKEN_MAX_COUNT];
	int argc = tokenize(argv, s_cmdline);

	switch (argc)
	{
	case -1:
		print(CLI_ENDL);
		print("error: too many tokens");
		break;
	case 0:
		break;
	default:
		exec(argc, argv);
		break;
	}

	printPrompt();
}

///
///
///
void Server::_escMoveCursorLeft()
{
	if (s_cursorPos > 0)
	{
		--s_cursorPos;
		print(CLI_ESC"[D");
	}

}


///
///
///
void Server::_escMoveCursorRight()
{
	if (s_cursorPos < s_cmdline.lenght())
	{
		++s_cursorPos;
		print(CLI_ESC"[C");
	}
}


///
///
///
void Server::_escHome()
{
	if (s_cursorPos > 0)
	{
		moveCursor(-s_cursorPos);
		s_cursorPos = 0;
	}
}


///
///
///
void Server::_escEnd()
{
	if (s_cursorPos < s_cmdline.lenght())
	{
		moveCursor(s_cmdline.lenght() - s_cursorPos);
		s_cursorPos = s_cmdline.lenght();
	}
}


///
///
///
void Server::_escBack()
{
	if (s_cursorPos > 0)
	{
		memmove(s_cmdline.begin() + s_cursorPos - 1,
				s_cmdline.begin() + s_cursorPos,
				s_cmdline.lenght() - s_cursorPos);
		s_cmdline.pop_back();
		--s_cursorPos;

		print(CLI_ESC"[D"" "CLI_ESC"[D");	// delete symbol
		saveCursorPos();
		print(s_cmdline.begin() + s_cursorPos);
		print(" ");				// hide last symbol
		loadCursorPos();
	}
}


///
///
///
void Server::_escDel()
{
	if (s_cursorPos < s_cmdline.lenght())
	{
		memmove(s_cmdline.begin() + s_cursorPos,
				s_cmdline.begin() + s_cursorPos + 1,
				s_cmdline.lenght() - s_cursorPos);
		s_cmdline.pop_back();

		saveCursorPos();
		print(s_cmdline.begin() + s_cursorPos);
		print(" ");
		loadCursorPos();
	}
}


///
///
///
void Server::_escUp()
{
#ifdef CLI_USE_HISTORY
	if (!s_history.empty())
	{
		searchHistory(CLI_HISTORY_SEARCH_UP);
	}
#endif
}


///
///
///
void Server::_escDown()
{
#ifdef CLI_USE_HISTORY
	if (!s_history.empty())
	{
		searchHistory(CLI_HISTORY_SEARCH_DOWN);
	}
#endif
}


} // namespace cli


