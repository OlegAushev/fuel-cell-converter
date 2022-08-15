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
class DurationLogger_us
{
private:
	static uint64_t (*m_timeNowFunc)();
	char m_message[32];
	volatile uint64_t m_start;

public:
	explicit DurationLogger_us(const char* message)
	{
		strcpy(m_message, message);
		m_start = m_timeNowFunc();
	}

	~DurationLogger_us()
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

	static void init(uint64_t (*timeNowFunc_ns)(void))
	{
		m_timeNowFunc = timeNowFunc_ns;
	}
};


#define EMB_LOG_DURATION_us(message) \
		volatile emb::DurationLogger_us EMB_UNIQ_ID(__LINE__)(message);


/**
 * @brief All calculations in clock cycles
 */
class DurationLogger_clk
{
private:
	static uint32_t (*m_timeNowFunc)();
	char m_message[32];
	volatile uint32_t m_start;

public:
	explicit DurationLogger_clk(const char* message)
	{
		strcpy(m_message, message);
		m_start = m_timeNowFunc();
	}

	~DurationLogger_clk()
	{
		volatile uint32_t finish = m_timeNowFunc();
		if (finish > m_start)
		{
			printf("%s: timer overflow\n", m_message);
		}
		else
		{
			printf("%s: %lu clock cycles\n", m_message, m_start - finish);
		}
	}

	static void init(uint32_t (*timeNowFunc_clk)(void))
	{
		m_timeNowFunc = timeNowFunc_clk;
	}
};


#define EMB_LOG_DURATION_clk(message) \
		volatile emb::DurationLogger_clk EMB_UNIQ_ID(__LINE__)(message);


/**
 * @brief All calculations in ns
 */
class DurationLoggerAsync_us
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
	DurationLoggerAsync_us(const char* message, size_t channel)
		: m_channel(channel)
	{
		m_durationsUs[m_channel].message = message;
		m_start = m_timeNowFunc();
	}

	~DurationLoggerAsync_us()
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


#define EMB_LOG_DURATION_ASYNC_us(message, channel) \
		volatile emb::DurationLoggerAsync_us EMB_UNIQ_ID(__LINE__)(message, channel);


} // namespace emb


