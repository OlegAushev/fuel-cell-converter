///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <assert.h>


namespace emb {

/**
 * @brief
 */
template <typename T, size_t Size>
class Array
{
private:
	T m_data[Size];
public:
	Array()	{}

	Array(const T* data) { memcpy(m_data, data, Size*sizeof(T)); }

	size_t size() const { return Size; }
	const T* data() const { return m_data; }
	T* data() { return m_data; }

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
		assert(pos < Size);
		return m_data[pos];
	}

	const T& at(size_t pos) const
	{
		assert(pos < Size);
		return m_data[pos];
	}

	T* begin() { return m_data; }
	T* end() { return m_data + Size; }
	const T* begin() const { return m_data; }
	const T* end() const { return m_data + Size; }

	void fill(const T& value)
	{
		for (size_t i = 0; i < Size; ++i)
		{
			m_data[i] = value;
		}
	}
};

} // namespace emb


