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
	const char* argv[CLI_TOKEN_MAX_COUNT];
	int argc = tokenize(argv, m_cmdline);

	switch (argc)
	{
	case -1:
		print(CLI_ENDL);
		print("error: too many tokens");
		break;
	case 0:
		break;
	default:
		print(CLI_ENDL);
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
	if (m_cursorPos > 0)
	{
		--m_cursorPos;
		print(CLI_ESC"[D");
	}

}


///
///
///
void Server::_escMoveCursorRight()
{
	if (m_cursorPos < m_cmdline.lenght())
	{
		++m_cursorPos;
		print(CLI_ESC"[C");
	}
}


///
///
///
void Server::_escHome()
{
	if (m_cursorPos > 0)
	{
		moveCursor(-m_cursorPos);
		m_cursorPos = 0;
	}
}


///
///
///
void Server::_escEnd()
{
	if (m_cursorPos < m_cmdline.lenght())
	{
		moveCursor(m_cmdline.lenght() - m_cursorPos);
		m_cursorPos = m_cmdline.lenght();
	}
}


///
///
///
void Server::_escBack()
{
	if (m_cursorPos > 0)
	{
		memmove(m_cmdline.begin() + m_cursorPos - 1,
				m_cmdline.begin() + m_cursorPos,
				m_cmdline.lenght() - m_cursorPos);
		m_cmdline.pop_back();
		--m_cursorPos;

		print(CLI_ESC"[D"" "CLI_ESC"[D");	// delete symbol
		saveCursorPos();
		print(m_cmdline.begin() + m_cursorPos);
		print(" ");				// hide last symbol
		loadCursorPos();
	}
}


///
///
///
void Server::_escDel()
{
	if (m_cursorPos < m_cmdline.lenght())
	{
		memmove(m_cmdline.begin() + m_cursorPos,
				m_cmdline.begin() + m_cursorPos + 1,
				m_cmdline.lenght() - m_cursorPos);
		m_cmdline.pop_back();

		saveCursorPos();
		print(m_cmdline.begin() + m_cursorPos);
		print(" ");
		loadCursorPos();
	}
}


} // namespace cli


