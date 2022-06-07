///
#pragma once


#include <stdint.h>
#include <stddef.h>


namespace emb {


/**
 * @brief
 */
template <typename T1, typename T2>
class Pair
{
public:
	T1 first;
	T2 second;
	Pair() {}
	Pair(const T1& _first, const T2& _second)
		: first(_first)
		, second(_second) {}

	Pair& operator=(const Pair& other)
	{
		if (this != &other)	// not a self-assignment
		{
			this->first = other.first;
			this->second = other.second;
		}
		return *this;
	}
};


} // namespace emb


