/**
 * @file
 * @ingroup cli
 */


#include "cli_server.h"


namespace cli {


const char* PROMPT_BEGIN = CLI_PROMPT_BEGIN;
const char* PROMPT_END = CLI_PROMPT_END;


emb::IUart* Server::s_uart = static_cast<emb::IUart*>(NULL);
emb::IGpioOutput* Server::s_pinRTS = static_cast<emb::IGpioOutput*>(NULL);
emb::IGpioInput* Server::s_pinCTS = static_cast<emb::IGpioInput*>(NULL);

char Server::PROMPT[CLI_PROMPT_MAX_LENGTH] = {0};
emb::String<CLI_CMDLINE_MAX_LENGTH> Server::s_cmdline;
emb::String<CLI_ESCSEQ_MAX_LENGTH> Server::s_escseq;

size_t Server::s_cursorPos = 0;

emb::Queue<char, CLI_OUTBUT_BUFFER_LENGTH> Server::s_outputBuf;

#ifdef CLI_USE_HISTORY
emb::CircularBuffer<emb::String<CLI_CMDLINE_MAX_LENGTH>, CLI_HISTORY_LENGTH> Server::s_history;
size_t Server::s_lastCmdHistoryPos = 0;
size_t Server::s_historyPosition = 0;
bool Server::s_newCmdSaved = false;
#endif

int (*Server::exec)(int argc, const char** argv) = Server::execNull;


const Server::EscSeq Server::ESCSEQ_LIST[] = {
{.str = "\x0D",		.len = 1,	.handler = Server::_escReturn},
{.str = "\x0A",		.len = 1,	.handler = Server::_escReturn},
{.str = CLI_ESC"[D",	.len = 3,	.handler = Server::_escMoveCursorLeft},
{.str = CLI_ESC"[C",	.len = 3,	.handler = Server::_escMoveCursorRight},
{.str = CLI_ESC"[H",	.len = 3,	.handler = Server::_escHome},
{.str = CLI_ESC"[F",	.len = 3,	.handler = Server::_escEnd},
{.str = "\x08",		.len = 1,	.handler = Server::_escBack},
{.str = "\x7F",		.len = 1,	.handler = Server::_escBack},
{.str = CLI_ESC"[3~",	.len = 4,	.handler = Server::_escDel},
{.str = CLI_ESC"[A",	.len = 3,	.handler = Server::_escUp},
{.str = CLI_ESC"[B",	.len = 3,	.handler = Server::_escDown},
};

const size_t ESCSEQ_LIST_SIZE = sizeof(Server::ESCSEQ_LIST) / sizeof(Server::ESCSEQ_LIST[0]);


///
///
///
Server::Server(const char* deviceName, emb::IUart* uart, emb::IGpioOutput* pinRTS, emb::IGpioInput* pinCTS)
{
	s_uart = uart;
	s_pinRTS = pinRTS;	// output
	s_pinCTS = pinCTS;	// input

	memset(PROMPT, 0, CLI_PROMPT_MAX_LENGTH);
	strcat(PROMPT, PROMPT_BEGIN);
	strncat(PROMPT, deviceName, CLI_DEVICE_NAME_MAX_LENGTH);
	strcat(PROMPT, PROMPT_END);

	printPrompt();
}


///
///
///
void Server::run()
{
	if (!s_outputBuf.empty())
	{
		if (s_uart->send(s_outputBuf.front()))
		{
			s_outputBuf.pop();
		}
	}
	else
	{
		char ch;
		if (s_uart->recv(ch))
		{
			processChar(ch);
		}
	}
}


///
///
///
void Server::print(char ch)
{
	if (!s_outputBuf.full())
	{
		s_outputBuf.push(ch);
	}
}


///
///
///
void Server::print(const char* str)
{
	while ((*str != '\0') && !s_outputBuf.full())
	{
		s_outputBuf.push(*str++);
	}
}


///
///
///
void Server::printBlocking(const char* str)
{
	s_uart->send(str, strlen(str));
}


///
///
///
void Server::processChar(char ch)
{
	if (s_cmdline.full())
		return;

	if (s_escseq.empty())
	{
		// Check escape signature
		if (ch <= 0x1F || ch == 0x7F)
		{
			s_escseq.push_back(ch);
		}
		// Print symbol if escape sequence signature is not found
		if (s_escseq.empty())
		{
			if (s_cursorPos < s_cmdline.lenght())
			{
				s_cmdline.insert(s_cursorPos, ch);
				saveCursorPos();
				print(s_cmdline.begin() + s_cursorPos);
				loadCursorPos();
			}
			else
			{
				s_cmdline.push_back(ch);
			}
			print(ch);
			++s_cursorPos;
		}
	}
	else
	{
		s_escseq.push_back(ch);
	}

	// Process escape sequence
	if (!s_escseq.empty())
	{
		int possibleEscseqCount = 0;
		size_t escseqIdx = 0;
		for (size_t i = 0; i < ESCSEQ_LIST_SIZE; ++i)
		{
			if ((s_escseq.lenght() <= ESCSEQ_LIST[i].len)
					&& (strncmp(s_escseq.data(), ESCSEQ_LIST[i].str, s_escseq.lenght()) == 0))
			{
				++possibleEscseqCount;
				escseqIdx = i;
			}
		}

		switch (possibleEscseqCount)
		{
		case 0: // no sequence - display all symbols
			/*for (size_t i = 0; (i < m_escseq.lenght()) && (!m_cmdline.full()); ++i)
			{
				if (m_escseq[i] <= 0x1F || m_escseq[i] == 0x7F)
				{
					m_escseq[i] = '?';
				}
				m_cmdline.insert(m_cursorPos + i, m_escseq[i]);
			}
			print(m_cmdline.begin() + m_cursorPos);
			m_cursorPos += m_escseq.lenght();*/
			s_escseq.clear();
			break;

		case 1: // one possible sequence found - check size and call handler
			if (s_escseq.lenght() == ESCSEQ_LIST[escseqIdx].len)
			{
				s_escseq.clear();
				ESCSEQ_LIST[escseqIdx].handler();
			}
			break;

		default: // few possible sequences found
			break;
		}
	}
}


///
///
///
void Server::moveCursor(int offset)
{
	char str[16] = {0};
	if (offset > 0) {
		snprintf(str, 16, CLI_ESC"[%dC", offset);
	} else if (offset < 0) {
		snprintf(str, 16, CLI_ESC"[%dD", -(offset));
	}
	print(str);
}


///
///
///
void Server::printPrompt()
{
	print(CLI_ENDL);
	print(PROMPT);
	s_cmdline.clear();
	s_cursorPos = 0;
}


///
///
///
int Server::tokenize(const char** argv, emb::String<CLI_CMDLINE_MAX_LENGTH>& cmdline)
{
	int argc = 0;
	size_t idx = 0;

	if (cmdline.empty())
	{
		return 0;
	}

	// replace all ' ' with '\0'
	for (size_t i = 0; i < cmdline.lenght(); ++i)
	{
		if (cmdline[i] == ' ')
		{
			cmdline[i] = '\0';
		}
	}

	while (true)
	{
		// go to the first not-whitespace (now - '\0')
		while (cmdline[idx] == '\0')
		{
			if (++idx >= cmdline.lenght())
				return argc;
		}

		if (argc >= CLI_TOKEN_MAX_COUNT)
			return -1;

		argv[argc++] = cmdline.begin() + idx;

		// go to the first whitespace (now - '\0')
		while ((cmdline[idx] != '\0') && (idx < cmdline.lenght()))
		{
			if (++idx >= cmdline.lenght())
				return argc;
		}
	}
}


#ifdef CLI_USE_HISTORY
///
///
///
void Server::searchHistory(HistorySearchDirection dir)
{
	static size_t pos;

	switch (dir)
	{
	case CLI_HISTORY_SEARCH_UP:
		if (s_newCmdSaved)
		{
			pos = s_historyPosition;
		}
		else
		{
			s_historyPosition = (s_historyPosition + (s_history.size() - 1)) % s_history.size();
			pos = s_historyPosition;
		}
		break;
	case CLI_HISTORY_SEARCH_DOWN:
		s_historyPosition = (s_historyPosition + 1) % s_history.size();
		pos = s_historyPosition;
		break;
	}

	s_newCmdSaved = false;

	// move cursor to line beginning
	if (s_cursorPos > 0)
	{
		moveCursor(-s_cursorPos);
		s_cursorPos = 0;
	}

	int remainder = s_cmdline.size() - s_history.data()[pos].size();
	s_cmdline = s_history.data()[pos];
	print(s_cmdline.data());
	s_cursorPos = s_cmdline.size();

	// clear remaining symbols
	saveCursorPos();
	for (int i = 0; i < remainder; ++i)
	{
		print(" ");
	}
	loadCursorPos();
}
#endif


} // namespace cli


