///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "motorcontrol/math.h"
#include "limits.h"
#include "float.h"

#include "emb_algorithm.h"


namespace emb {


const float PI = MATH_PI;
const float PI_OVER_2 = MATH_PI_OVER_TWO;
const float PI_OVER_4 = MATH_PI_OVER_FOUR;
const float PI_OVER_3 = MATH_PI / 3;
const float PI_OVER_6 = MATH_PI / 6;
const float TWO_PI = MATH_TWO_PI;

const float SQRT_2 = sqrtf(2.f);
const float SQRT_3 = sqrtf(3.f);


/**
 * @brief
 */
template <typename T>
inline int sgn(T value) { return (value > T(0)) - (value < T(0)); }


/**
 * @brief
 */
inline float to_rad(float deg) { return PI * deg / 180; }


/**
 * @brief
 */
inline float to_deg(float rad) { return 180 * rad / PI; }


/**
 * @brief
 */
inline float normalize_2pi(float value)
{
	value = fmodf(value, TWO_PI);
	if (value < 0)
	{
		value += TWO_PI;
	}
	return value;
}


/**
 * @brief
 */
inline float normalize_pi(float value)
{
	value = fmodf(value + PI, TWO_PI);
	if (value < 0)
	{
		value += TWO_PI;
	}
	return value - PI;
}


/**
 * @brief
 */
template <typename T>
class Range
{
private:
	T lo_;
	T hi_;
public:
	Range(const T& val1, const T& val2)
	{
		if (val1 < val2)
		{
			lo_ = val1;
			hi_ = val2;
		}
		else
		{
			lo_ = val2;
			hi_ = val1;
		}
	}

	bool contains(const T& val) const { return (lo_ <= val) && (val <= hi_); }

	const T& lo() const { return lo_; }
	void setLo(const T& value)
	{
		if (value <= hi_)
		{
			lo_ = value;
		}
	}

	const T& hi() const { return hi_; }
	void setHi(const T& value)
	{
		if (value >= lo_)
		{
			hi_ = value;
		}
	}
};


/**
 * @brief
 */
template <typename T, typename Time>
class Integrator
{
private:
	T m_sum;
	Time m_dt;
	T m_init;
public:
	Range<T> range;

	Integrator(const Range<T>& _range, const Time& _dt, const T& _init)
		: range(_range)
		, m_dt(_dt)
		, m_init(_init)
	{
		reset();
	}

	void integrate(const T& value)
	{
		m_sum = clamp(m_sum + value * m_dt, range.lo(), range.hi());
	}

	void add(const T& value)
	{
		m_sum = clamp(m_sum + value, range.lo(), range.hi());
	}

	const T& value() const { return m_sum; }
	void reset()
	{
		m_sum = clamp(m_init, range.lo(), range.hi());
	}
};


} // namespace emb


