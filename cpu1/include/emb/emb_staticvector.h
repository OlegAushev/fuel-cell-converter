///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include "emb_algorithm.h"


namespace emb {


/**
 * @brief
 */
template <typename T, size_t Capacity>
class StaticVector
{
private:
	T m_data[Capacity];
	size_t m_size;
public:
	StaticVector()
		: m_size(0)
	{}

	explicit StaticVector(size_t size)
		: m_size(size)
	{
		assert(size <= Capacity);
		emb::fill(begin(), end(), T());
	}

	StaticVector(size_t size, const T& value)
		: m_size(size)
	{
		assert(size <= Capacity);
		emb::fill(begin(), end(), value);
	}

	template <class V>
	StaticVector(V* first, V* last)
		: m_size(last - first)
	{
		assert(first <= last);
		assert((last - first) <= Capacity);
		emb::copy(first, last, begin());
	}

public:
	size_t capacity() const { return Capacity; }
	size_t size() const { return m_size; }
	bool empty() const { return m_size == 0; }
	bool full() const { return m_size == Capacity; }

	T& operator[] (size_t pos)
	{
#ifdef NDEBUG
		return m_data[pos];
#else
		return at(pos);
#endif
	}

	const T& operator[](size_t pos) const
	{
#ifdef NDEBUG
		return m_data[pos];
#else
		return at(pos);
#endif
	}

	T& at(size_t pos)
	{
		assert(pos < m_size);
		return m_data[pos];
	}

	const T& at(size_t pos) const
	{
		assert(pos < m_size);
		return m_data[pos];
	}

public:
	T* begin() { return m_data; }
	T* end() { return m_data + m_size; }
	const T* begin() const { return m_data; }
	const T* end() const { return m_data + m_size; }

	T* data() { return m_data; }
	const T* data() const { return m_data; }

	T& front()
	{
		assert(!empty());
		return m_data[0];
	}

	const T& front() const
	{
		assert(!empty());
		return m_data[0];
	}

	T& back()
	{
		assert(!empty());
		return m_data[m_size - 1];
	}

	const T& back() const
	{
		assert(!empty());
		return m_data[m_size - 1];
	}

public:
	void resize(size_t size)
	{
		assert(size <= Capacity);
		if (size > m_size)
		{
			emb::fill(m_data + m_size, m_data + size, T());
		}
		m_size = size;
	}

	void resize(size_t size, const T& value)
	{
		assert(size <= Capacity);
		if (size > m_size)
		{
			emb::fill(m_data + m_size, m_data + size, value);
		}
		m_size = size;
	}

	void clear()
	{
		m_size = 0;
	}

public:
	void push_back(const T& value)
	{
		assert(!full());
		m_data[m_size++] = value;
	}

	void pop_back()
	{
		assert(!empty());
		--m_size;
	}

	void insert(T* pos, const T& value)
	{
		assert(!full());
		assert(pos <= end());

		StaticVector<T, Capacity> buf(pos, end());
		*pos++ = value;
		emb::copy(buf.begin(), buf.end(), pos);
		++m_size;
	}

	void insert(T* pos, size_t count, const T& value)
	{
		assert((m_size + count) <= Capacity);
		assert(pos <= end());

		StaticVector<T, Capacity> buf(pos, end());
		for (size_t i = 0; i < count; ++i)
		{
			*pos++ = value;
		}
		emb::copy(buf.begin(), buf.end(), pos);
		m_size += count;
	}

	template <class V>
	void insert(T* pos, V* first, V* last)
	{
		assert(first <= last);
		assert((m_size + last - first) <= Capacity);
		assert(pos <= end());

		StaticVector<T, Capacity> buf(pos, end());
		pos = emb::copy(first, last, pos);
		emb::copy(buf.begin(), buf.end(), pos);
		m_size = m_size + last - first;
	}

public:
	T* erase(T* pos)
	{
		assert(!empty());
		assert(pos < end());

		StaticVector<T, Capacity> buf(pos + 1, end());
		--m_size;
		return emb::copy(buf.begin(), buf.end(), pos);
	}

	T* erase(T* first, T* last)
	{
		assert(m_size >= (last - first));
		assert(begin() <= first);
		assert(first <= last);
		assert(last <= end());

		StaticVector<T, Capacity> buf(last, end());
		m_size = m_size - (last - first);
		return emb::copy(buf.begin(), buf.end(), first);
	}
};


} // namespace emb


