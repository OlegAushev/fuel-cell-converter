///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include "emb_common.h"
#include "emb_array.h"
#include <bitset>


namespace emb {


/**
 * @brief
 */
template <size_t BitCount>
class Bitset
{
	EMB_STATIC_ASSERT(BitCount > 0);
private:
	static const size_t BYTES_COUNT = (BitCount + CHAR_BIT - 1) / CHAR_BIT;
	static const size_t EXTRA_BITS = BitCount % CHAR_BIT;

	unsigned int m_data[BYTES_COUNT];

	static size_t _whichByte(size_t pos) { return pos / CHAR_BIT; }
	static size_t _whichBit(size_t pos) { return pos % CHAR_BIT; }
	unsigned int& _hiByte() { return m_data[BYTES_COUNT-1]; }
	unsigned int _hiByte() const { return m_data[BYTES_COUNT-1]; }
	static void _resetUnusedBits(unsigned int& hiByte) { if (EXTRA_BITS) hiByte &= ~(~0U << EXTRA_BITS); }

public:
	class Reference;

	Bitset()
	{
		for (size_t i = 0; i < BYTES_COUNT; ++i)
		{
			m_data[i] = 0U;
		}
	}

	Bitset(const emb::Array<unsigned int, BYTES_COUNT>& words)
	{
		for (size_t i = 0; i < BYTES_COUNT; ++i)
		{
			m_data[i] = words[i];
		}
		_resetUnusedBits(_hiByte());
	}

	Bitset(uint64_t value)
	{
		EMB_STATIC_ASSERT(BYTES_COUNT <= 4);
		uint64_t val = value;	// protection against implicit conversion and wrong memcpy()
		memcpy(m_data, &val, BYTES_COUNT);
		_resetUnusedBits(_hiByte());
	}

	size_t size() const { return BitCount; }

	bool operator[](size_t pos) const
	{
#ifdef NDEBUG
		return m_data[_whichByte(pos)] & (1U << _whichBit(pos));
#else
		return test(pos);
#endif
	}

	Reference operator[](size_t pos)
	{
		return Reference(this, pos);
	}

	bool test(size_t pos) const
	{
		assert(pos < BitCount);
		return m_data[_whichByte(pos)] & (1U << _whichBit(pos));
	}

	bool all() const
	{
		// int index is used to suppress "pointless comparison of unsigned integer with zero" warning
		for (int i = 0; i < BYTES_COUNT-1; ++i)
		{
			if (m_data[i] != 0xFFFF) return false;
		}

		if (EXTRA_BITS)
		{
			if (_hiByte() != ~(~0U << EXTRA_BITS)) return false;
		}
		else
		{
			if (_hiByte() != 0xFFFF) return false;
		}
		return true;
	}

	bool any() const
	{
		for (size_t i = 0; i < BYTES_COUNT; ++i)
		{
			if (m_data[i] != 0) return true;
		}
		return false;
	}

	bool none() const
	{
		return !any();
	}

	size_t count() const
	{
		size_t ret = 0;
		for (size_t i = 0; i < BitCount; ++i)
		{
			if ((*this)[i]) ++ret;
		}
		return ret;
	}

	void set()
	{
		for (size_t i = 0; i < BYTES_COUNT; ++i)
		{
			m_data[i] = 0xFFFF;
		}
		_resetUnusedBits(_hiByte());
	}

	void set(size_t pos, bool value = true)
	{
		assert(pos < BitCount);
		if (value)
			m_data[_whichByte(pos)] |= 1U << _whichBit(pos);
		else
			m_data[_whichByte(pos)] &= ~(1U << _whichBit(pos));
	}

	void reset()
	{
		for (size_t i = 0; i < BYTES_COUNT; ++i)
		{
			m_data[i] = 0U;
		}
	}

	void reset(size_t pos)
	{
		assert(pos < BitCount);
		m_data[_whichByte(pos)] &= ~(1U << _whichBit(pos));
	}

	void flip()
	{
		for (size_t i = 0; i < BYTES_COUNT; ++i)
		{
			m_data[i] = ~m_data[i];
		}
		_resetUnusedBits(_hiByte());
	}

	void flip(size_t pos)
	{
		assert(pos < BitCount);
		m_data[_whichByte(pos)] ^= 1U << _whichBit(pos);
	}

	bool operator==(const Bitset& rhs) const
	{
		for (size_t i = 0; i < BYTES_COUNT; ++i)
		{
			if (m_data[i] != rhs.m_data[i]) return false;
		}
		return true;
	}

	bool operator!=(const Bitset& rhs) const
	{
		return !((*this) == rhs);
	}

	struct Reference
	{
		Bitset* self;
		size_t pos;
		Reference(Bitset* _self, size_t _pos) : self(_self), pos(_pos) {}
		operator bool() const { return self->test(pos); }
		Reference& operator=(bool value)
		{
			self->set(pos, value);
			return *this;
		}
	};
};


} // namespace emb


