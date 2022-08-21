///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <cstring>

#include "emb_algorithm.h"


namespace emb {


/**
 * @brief
 */
template <size_t Capacity>
class String
{
private:
	static const size_t DATA_LEN = Capacity + 1;
	char m_data[DATA_LEN];
	size_t m_len;
public:
	String()
	{
		memset(m_data, 0, DATA_LEN);
		m_len = 0;
	}

	String(const char str[])
	{
		memset(m_data, 0, DATA_LEN);
		strncpy(m_data, str, Capacity);
		m_len = strlen(str);
	}

public:
	size_t capacity() const { return Capacity; }
	size_t lenght() const { return m_len; }
	size_t size() const { return m_len; }
	bool empty() const { return m_len == 0; }
	bool full() const { return m_len == Capacity; }

	char& operator[] (size_t pos)
	{
#ifdef NDEBUG
		return m_data[pos];
#else
		return at(pos);
#endif
	}

	const char& operator[](size_t pos) const
	{
#ifdef NDEBUG
		return m_data[pos];
#else
		return at(pos);
#endif
	}

	char& at(size_t pos)
	{
		assert(pos < m_len);
		return m_data[pos];
	}

	const char& at(size_t pos) const
	{
		assert(pos < m_len);
		return m_data[pos];
	}

public:
	char* begin() { return m_data; }
	char* end() { return m_data + m_len; }
	const char* begin() const { return m_data; }
	const char* end() const { return m_data + m_len; }

	char* data() { return m_data; }
	const char* data() const { return m_data; }

	char& front()
	{
		assert(!empty());
		return m_data[0];
	}

	const char& front() const
	{
		assert(!empty());
		return m_data[0];
	}

	char& back()
	{
		assert(!empty());
		return m_data[m_len - 1];
	}

	const char& back() const
	{
		assert(!empty());
		return m_data[m_len - 1];
	}

public:
	void resize(size_t len)
	{
		assert(len <= Capacity);
		if (len > m_len)
		{
			emb::fill(m_data + m_len, m_data + len, 0);
		}
		else
		{
			emb::fill(m_data + len, m_data + m_len, 0);
		}
		m_len = len;
	}

	void resize(size_t len, char ch)
	{
		assert(len <= Capacity);
		if (len > m_size)
		{
			emb::fill(m_data + m_len, m_data + len, ch);
		}
		else
		{
			emb::fill(m_data + len, m_data + m_len, ch);
		}
		m_len = len;
	}

	void clear()
	{
		memset(m_data, 0, DATA_LEN);
		m_len = 0;
	}

public:
	void push_back(char ch)
	{
		assert(!full());
		m_data[m_len++] = ch;
	}

	void pop_back()
	{
		assert(!empty());
		m_data[--m_len] = 0;
	}

public:
	void insert(size_t index, char ch)
	{
		assert(!full());
		assert(index <= lenght());

		if (index == m_len)
		{
			push_back(ch);
			return;
		}

		memmove(m_data + index + 1 , m_data + index, m_len - index);
		m_data[index] = ch;
		++m_len;
	}
};


template <size_t Capacity>
inline bool operator==(const String<Capacity>& lhs, const String<Capacity>& rhs)
{
	return strcmp(lhs.data(), rhs.data()) == 0;
}


template <size_t Capacity>
inline bool operator!=(const String<Capacity>& lhs, const String<Capacity>& rhs)
{
	return !(lhs == rhs);
}


} // namespace emb


