/**
 * @file
 * @ingroup cli
 */


#include "cli_server.h"


namespace cli {


const char* PROMPT_BEGIN = CLI_PROMPT_BEGIN;
const char* PROMPT_END = CLI_PROMPT_END;


emb::IUart* Server::m_uart = static_cast<emb::IUart*>(NULL);
emb::IGpioOutput* Server::m_pinRTS = static_cast<emb::IGpioOutput*>(NULL);
emb::IGpioInput* Server::m_pinCTS = static_cast<emb::IGpioInput*>(NULL);

char Server::PROMPT[CLI_PROMPT_MAX_LENGTH] = {0};
emb::String<CLI_CMDLINE_MAX_LENGTH> Server::m_cmdline;
emb::String<CLI_ESCSEQ_MAX_LENGTH> Server::m_escseq;

size_t Server::m_cursorPos = 0;

emb::Queue<char, CLI_OUTBUT_BUFFER_LENGTH> Server::m_outputBuf;

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
};

const size_t ESCSEQ_LIST_SIZE = sizeof(Server::ESCSEQ_LIST) / sizeof(Server::ESCSEQ_LIST[0]);


///
///
///
Server::Server(const char* deviceName, emb::IUart* uart, emb::IGpioOutput* pinRTS, emb::IGpioInput* pinCTS)
{
	m_uart = uart;
	m_pinRTS = pinRTS;	// output
	m_pinCTS = pinCTS;	// input

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
	if (!m_outputBuf.empty())
	{
		if (m_uart->send(m_outputBuf.front()))
		{
			m_outputBuf.pop();
		}
	}
	else
	{
		char ch;
		if (m_uart->recv(ch))
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
	if (!m_outputBuf.full())
	{
		m_outputBuf.push(ch);
	}
}


///
///
///
void Server::print(const char* str)
{
	while ((*str != '\0') && !m_outputBuf.full())
	{
		m_outputBuf.push(*str++);
	}
}


///
///
///
void Server::printBlocking(const char* str)
{
	m_uart->send(str, strlen(str));
}


///
///
///
void Server::processChar(char ch)
{
	if (m_cmdline.full())
		return;

	if (m_escseq.empty())
	{
		// Check escape signature
		if (ch <= 0x1F || ch == 0x7F)
		{
			m_escseq.push_back(ch);
		}
		// Print symbol if escape sequence signature is not found
		if (m_escseq.empty())
		{
			if (m_cursorPos < m_cmdline.lenght())
			{
				m_cmdline.insert(m_cursorPos, ch);
				saveCursorPos();
				print(m_cmdline.begin() + m_cursorPos);
				loadCursorPos();
			}
			else
			{
				m_cmdline.push_back(ch);
			}
			print(ch);
			++m_cursorPos;
		}
	}
	else
	{
		m_escseq.push_back(ch);
	}

	// Process escape sequence
	if (!m_escseq.empty())
	{
		int possibleEscseqCount = 0;
		size_t escseqIdx = 0;
		for (size_t i = 0; i < ESCSEQ_LIST_SIZE; ++i)
		{
			if ((m_escseq.lenght() <= ESCSEQ_LIST[i].len)
					&& (strncmp(m_escseq.data(), ESCSEQ_LIST[i].str, m_escseq.lenght()) == 0))
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
			m_escseq.clear();
			break;

		case 1: // one possible sequence found - check size and call handler
			if (m_escseq.lenght() == ESCSEQ_LIST[escseqIdx].len)
			{
				m_escseq.clear();
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
	m_cmdline.clear();
	m_cursorPos = 0;
}


///
///
///
int Server::tokenize(const char** argv, emb::String<CLI_CMDLINE_MAX_LENGTH>& cmdline)
{
	int argc = 0;
	size_t idx = 0;

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







} // namespace cli

