///
#pragma once


#include <stdint.h>
#include <stddef.h>


namespace emb {


/**
 * @brief
 */
template <typename T, size_t Capacity>
class CircularBuffer
{
private:
	T m_data[Capacity];
	size_t m_front;
	size_t m_back;
	bool m_full;
public:
	CircularBuffer()
		: m_front(0)
		, m_back(0)
		, m_full(false)
	{}

	void clear()
	{
		m_front = 0;
		m_back = 0;
		m_full = false;
	}

	bool empty() const { return (!m_full && (m_front == m_back)); }
	bool full() const { return m_full; }
	size_t capacity() const { return Capacity; }

	size_t size() const
	{
		size_t size = Capacity;
		if (!m_full)
		{
			if (m_back >= m_front)
			{
				size = m_back - m_front;
			}
			else
			{
				size = Capacity + m_back - m_front;
			}
		}

		return size;
	}

	void push(const T& value)
	{
		m_data[m_back] = value;
		if (m_full)
		{
			m_front = (m_front + 1) % Capacity;
		}
		m_back = (m_back + 1) % Capacity;
		m_full = (m_front == m_back);
	}

	const T& front() const
	{
		assert(!empty());
		return m_data[m_front];
	}

	const T& back() const
	{
		assert(!empty());
		return m_data[(m_back + Capacity - 1) % Capacity];
	}

	void pop()
	{
		assert(!empty());
		m_full = false;
		m_front = (m_front + 1) % Capacity;
	}

	const T* data() const { return m_data; }
	const T* begin() const { return m_data; }
	const T* end() const { return m_data + Capacity; }

	void fill(const T& value)
	{
		for (size_t i = 0; i < Capacity; ++i)
		{
			m_data[i] = value;
		}
	}
};


} // namespace emb


