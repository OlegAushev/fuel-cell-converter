///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <cstdio>
#include <cstring>

#include "../emb_common.h"


namespace emb {


/**
 * @brief All calculations in ns
 */
class DurationLogger
{
private:
	static uint64_t (*m_timeNowFunc)();
	char m_message[32];
	volatile uint64_t m_start;

public:
	explicit DurationLogger(const char* message)
	{
		strcpy(m_message, message);
		m_start = m_timeNowFunc();
	}

	~DurationLogger()
	{
		volatile uint64_t finish = m_timeNowFunc();
		if (finish < m_start)
		{
			printf("%s: timer overflow\n", m_message);
		}
		else
		{
			printf("%s: %.3f us\n", m_message, float(finish - m_start) / 1000.f);
		}
	}

	static void init(uint64_t (*timeNowFunc)(void))
	{
		m_timeNowFunc = timeNowFunc;
	}
};


#define EMB_LOG_DURATION(message) \
		volatile emb::DurationLogger EMB_UNIQ_ID(__LINE__)(message);


/**
 * @brief All calculations in ns
 */
class DurationLoggerAsyncPrint
{
private:
	static uint64_t (*m_timeNowFunc)();
	static const size_t CAPACITY = 10;

	struct DurationData
	{
		const char* message;
		volatile float value;
		DurationData() : value(0) {}
	};

	static DurationData m_durationsUs[CAPACITY];

	const size_t m_channel;
	volatile uint64_t m_start;
public:
	DurationLoggerAsyncPrint(const char* message, size_t channel)
		: m_channel(channel)
	{
		m_durationsUs[m_channel].message = message;
		m_start = m_timeNowFunc();
	}

	~DurationLoggerAsyncPrint()
	{
		volatile uint64_t finish = m_timeNowFunc();
		if (finish < m_start)
		{
			m_durationsUs[m_channel].value = 0;
		}
		else
		{
			m_durationsUs[m_channel].value = float(finish - m_start) / 1000.f;
		}
	}

	static void init(uint64_t (*timeNowFunc)(void))
	{
		m_timeNowFunc = timeNowFunc;
	}

	static void print()
	{
		for (size_t i = 0; i < CAPACITY; ++i)
		{
			if (m_durationsUs[i].value != 0)
			{
				printf("%s: %.3f us\n", m_durationsUs[i].message, m_durationsUs[i].value);
			}
		}
	}
};


#define EMB_LOG_DURATION_ASYNC_PRINT(message, channel) \
		volatile emb::DurationLoggerAsyncPrint EMB_UNIQ_ID(__LINE__)(message, channel);


} // namespace emb


