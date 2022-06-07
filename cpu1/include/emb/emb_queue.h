///
#pragma once


#include <stdint.h>
#include <stddef.h>


namespace emb {


/**
 * @brief
 */
template <typename T, size_t Capacity>
class Queue
{
private:
	T m_data[Capacity];
	size_t m_front;
	size_t m_back;
	size_t m_size;
public:
	Queue()
		: m_front(0)
		, m_back(0)
		, m_size(0)
	{}

	void clear()
	{
		m_front = 0;
		m_back = 0;
		m_size = 0;
	}

	bool empty() const { return m_size == 0; }
	bool full() const { return m_size == Capacity; }
	size_t capacity() const { return Capacity; }
	size_t size() const { return m_size; }

	void push(const T& value)
	{
		assert(!full());

		if (empty())
		{
			m_back = m_front;
		}
		else
		{
			m_back = (m_back + 1) % Capacity;
		}
		m_data[m_back] = value;
		++m_size;
	}

	const T& front() const
	{
		assert(!empty());
		return m_data[m_front];
	}

	const T& back() const
	{
		assert(!empty());
		return m_data[m_back];
	}

	void pop()
	{
		assert(!empty());
		m_front = (m_front + 1) % Capacity;
		--m_size;
	}
};


} // namespace embd


