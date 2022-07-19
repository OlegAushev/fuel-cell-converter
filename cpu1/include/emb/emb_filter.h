///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <algorithm>

#include "emb_common.h"
#include "emb_array.h"
#include "emb_circularbuffer.h"
#include "emb_algorithm.h"


namespace emb {


/**
 * @brief Filter interface
 */
template <typename T>
class IFilter
{
private:
	IFilter(const IFilter& other);			// no copy constructor
	IFilter& operator=(const IFilter& other);	// no copy assignment operator
public:
	IFilter() {}
	virtual ~IFilter() {}

	virtual void push(T value) {}
	virtual T output() const = 0;
	virtual void setOutput(T value) = 0;
	virtual void reset() = 0;
};


/**
 * @brief Moving average filter
 */
template <typename T, size_t WindowSize>
class MovingAvgFilter : public IFilter<T>
{
private:
	size_t m_size;
	T* m_window;
	size_t m_index;
	T m_sum;
	bool m_heapUsed;

	MovingAvgFilter(const MovingAvgFilter& other);			// no copy constructor
	MovingAvgFilter& operator=(const MovingAvgFilter& other);	// no copy assignment operator
public:
	MovingAvgFilter()
		: m_size(WindowSize)
		, m_window(new T[WindowSize])
		, m_index(0)
		, m_sum(0)
		, m_heapUsed(true)
	{
		reset();
	}

	MovingAvgFilter(emb::Array<T, WindowSize>& dataArray)
		: m_size(WindowSize)
		, m_window(dataArray.data)
		, m_index(0)
		, m_sum(T(0))
		, m_heapUsed(false)
	{
		reset();
	}

	~MovingAvgFilter()
	{
		if (m_heapUsed == true)
		{
			delete[] m_window;
		}
	}

	virtual void push(T value)
	{
		m_sum = m_sum + value - m_window[m_index];
		m_window[m_index] = value;
		m_index = (m_index + 1) % m_size;
	}

	virtual T output() const { return m_sum / m_size; }

	virtual void setOutput(T value)
	{
		for (size_t i = 0; i < m_size; ++i)
		{
			m_window[i] = value;
		}
		m_index = 0;
		m_sum = value * m_size;
	}

	virtual void reset() { setOutput(0); }

	int size() const { return m_size; }

	void resize(size_t size)
	{
		if (size == 0)
		{
			return;
		}
		if (size > WindowSize)
		{
			m_size = WindowSize;
			reset();
			return;
		}
		m_size = size;
		reset();
	}
};


/**
 * @brief Median filter
 */
template <typename T, size_t WindowSize>
class MedianFilter : public IFilter<T>
{
private:
	CircularBuffer<T, WindowSize> m_window;
	T m_out;

	MedianFilter(const MedianFilter& other);		// no copy constructor
	MedianFilter& operator=(const MedianFilter& other);	// no copy assignment operator

public:
	MedianFilter()
	{
		EMB_STATIC_ASSERT((WindowSize % 2) == 1);
		reset();
	}

	virtual void push(T value)
	{
		m_window.push(value);
		Array<T, WindowSize> windowSorted;
		emb::copy(m_window.begin(), m_window.end(), windowSorted.begin());
		std::sort(windowSorted.begin(), windowSorted.end());
		m_out = windowSorted[WindowSize/2];
	}

	virtual T output() const { return m_out; }

	virtual void setOutput(T value)
	{
		m_window.fill(value);
		m_out = value;
	}

	virtual void reset() { setOutput(0); }
};


/**
 * @brief Exponential filter
 */
template <typename T>
class ExponentialFilter : public IFilter<T>
{
private:
	float m_smoothFactor;
	T m_out;
	T m_outPrev;

	ExponentialFilter(const ExponentialFilter& other);		// no copy constructor
	ExponentialFilter& operator=(const ExponentialFilter& other);	// no copy assignment operator

public:
	ExponentialFilter()
		: m_smoothFactor(0)
	{
		reset();
	}

	ExponentialFilter(float smoothFactor)
		: m_smoothFactor(smoothFactor)
	{
		reset();
	}

	virtual void push(T value)
	{
		m_out = m_outPrev + m_smoothFactor * (value - m_outPrev);
		m_outPrev = m_out;
	}

	virtual T output() const { return m_out; }

	virtual void setOutput(T value)
	{
		m_out = value;
		m_outPrev = value;
	}

	virtual void reset() { setOutput(0); }

	void setSmoothFactor(float smoothFactor) { m_smoothFactor = smoothFactor; }
};


/**
 * @brief Exponential + Median filter
 */
template <typename T, size_t WindowSize>
class ExponentialMedianFilter : public IFilter<T>
{
private:
	CircularBuffer<T, WindowSize> m_window;
	float m_smoothFactor;
	T m_out;
	T m_outPrev;

	ExponentialMedianFilter(const ExponentialMedianFilter& other);			// no copy constructor
	ExponentialMedianFilter& operator=(const ExponentialMedianFilter& other);	// no copy assignment operator

public:
	ExponentialMedianFilter()
		: m_smoothFactor(0)
	{
		EMB_STATIC_ASSERT((WindowSize % 2) == 1);
		reset();
	}

	ExponentialMedianFilter(float smoothFactor)
		: m_smoothFactor(smoothFactor)
	{
		EMB_STATIC_ASSERT((WindowSize % 2) == 1);
		reset();
	}

	virtual void push(T value)
	{
		m_window.push(value);
		Array<T, WindowSize> windowSorted;
		emb::copy(m_window.begin(), m_window.end(), windowSorted.begin());
		std::sort(windowSorted.begin(), windowSorted.end());
		value = windowSorted[WindowSize/2];

		m_out = m_outPrev + m_smoothFactor * (value - m_outPrev);
		m_outPrev = m_out;
	}

	virtual T output() const { return m_out; }

	virtual void setOutput(T value)
	{
		m_window.fill(value);
		m_out = value;
		m_outPrev = value;
	}

	virtual void reset() { setOutput(0); }

	void setSmoothFactor(float smoothFactor) { m_smoothFactor = smoothFactor; }
};


} // namespace emb


