///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "motorcontrol/math.h"
#include "motorcontrol/clarke.h"
#include "motorcontrol/park.h"
#include "motorcontrol/ipark.h"
#include "emb_array.h"
#include "algorithm"


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
inline float to_radps(float speedRpm, int polePairs) { return 2 * PI * polePairs * speedRpm / 60; }

/**
 * @brief
 */
inline float to_radps(float speedRpm) { return 2 * PI * speedRpm / 60; }

/**
 * @brief
 */
inline float to_rpm(float speedRadps, int polePairs) { return 60 * speedRadps / (2 * PI * polePairs); }

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
inline void calculate_svpwm(emb::Array<T, 3>& pwmCmpValues,
		float voltageMag, float voltageAngle,
		float voltageDC, T pwmCounterPeriod)
{
	voltageAngle = normalize_2pi(voltageAngle);

	int32_t sector = static_cast<int32_t>(voltageAngle / PI_OVER_3);
	float theta = voltageAngle - float(sector) * PI_OVER_3;

	if (voltageMag > (voltageDC / SQRT_3))
	{
		voltageMag = voltageDC / SQRT_3;
	}
	else if (voltageMag < 0)
	{
		voltageMag = 0;
	}

	// base vector times calculation
	float tb1 = SQRT_3 * (voltageMag / voltageDC) * sinf(PI_OVER_3 - theta);
	float tb2 = SQRT_3 * (voltageMag / voltageDC) * sinf(theta);
	float tb0 = (1.f - tb1 - tb2) / 2.f;

	emb::Array<float, 3> pulseTimes;
	switch (sector)
	{
	case 0:
		pulseTimes[0] = tb1 + tb2 + tb0;
		pulseTimes[1] = tb2 + tb0;
		pulseTimes[2] = tb0;
		break;
	case 1:
		pulseTimes[0] = tb1 + tb0;
		pulseTimes[1] = tb1 + tb2 + tb0;
		pulseTimes[2] = tb0;
		break;
	case 2:
		pulseTimes[0] = tb0;
		pulseTimes[1] = tb1 + tb2 + tb0;
		pulseTimes[2] = tb2 + tb0;
		break;
	case 3:
		pulseTimes[0] = tb0;
		pulseTimes[1] = tb1 + tb0;
		pulseTimes[2] = tb1 + tb2 + tb0;
		break;
	case 4:
		pulseTimes[0] = tb2 + tb0;
		pulseTimes[1] = tb0;
		pulseTimes[2] = tb1 + tb2 + tb0;
		break;
	case 5:
		pulseTimes[0] = tb1 + tb2 + tb0;
		pulseTimes[1] = tb0;
		pulseTimes[2] = tb1 + tb0;
		break;
	default:
		break;
	}

	for(uint32_t i = 0; i < 3; i++)
	{
		if (pulseTimes[i] < 0.f)
		{
			pulseTimes[i] = 0.f;
		}
		else if (pulseTimes[i] > 1.f)
		{
			pulseTimes[i] = 1.f;
		}
		pwmCmpValues[i] = (T)(pulseTimes[i] * pwmCounterPeriod);
	}
}

struct DQPair
{
	float d;
	float q;
	DQPair() {}
	DQPair(float d_, float q_) : d(d_), q(q_) {}
};

struct AlphaBetaPair
{
	float alpha;
	float beta;
	AlphaBetaPair() {}
	AlphaBetaPair(float alpha_, float beta_) : alpha(alpha_), beta(beta_) {}
};

/**
 * @brief
 */
inline DQPair park_transform(float alpha, float beta, float sine, float cosine)
{
	PARK parkStruct =
	{
		.Alpha = alpha,
		.Beta = beta,
		.Sine = sine,
		.Cosine = cosine
	};
	runPark(&parkStruct);
	return DQPair(parkStruct.Ds, parkStruct.Qs);
}

/**
 * @brief
 */
inline AlphaBetaPair invpark_transform(float d, float q, float sine, float cosine)
{
	IPARK iparkStruct =
	{
		.Ds = d,
		.Qs = q,
		.Sine = sine,
		.Cosine = cosine
	};
	runIPark(&iparkStruct);
	return AlphaBetaPair(iparkStruct.Alpha, iparkStruct.Beta);
}

/**
 * @brief
 */
inline AlphaBetaPair clarke_transform(float a, float b, float c)
{
	CLARKE clarkeStruct =
	{
		.As = a,
		.Bs = b,
		.Cs = c
	};
	runClarke1(&clarkeStruct);
	return AlphaBetaPair(clarkeStruct.Alpha, clarkeStruct.Beta);
}

/**
 * @brief
 */
class MotorAngularSpeed
{
private:
	const int POLE_PAIRS;
	float m_radpsElec;
public:
	explicit MotorAngularSpeed(int polePairs)
		: POLE_PAIRS(polePairs)
		, m_radpsElec(0)
	{}

	MotorAngularSpeed(float radpsElec, int polePairs)
		: POLE_PAIRS(polePairs)
		, m_radpsElec(radpsElec)
	{}

	float radps() const { return m_radpsElec; }
	float rpm() const { return 60 * m_radpsElec / (2 * PI * POLE_PAIRS); }
	float radpsMech() const { return m_radpsElec / POLE_PAIRS; }

	void setRadps(float value) { m_radpsElec = value; }
	void setRpm(float value) { m_radpsElec = 2 * PI * POLE_PAIRS * value / 60; }
};

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










#ifdef OBSOLETE
/**
 * @brief
 */
void CompensatePwm(const ArrayN<float, 3>& phase_currents)
{
	float uznam __attribute__((unused));
	uznam = pwm_compensation.udc - pwm_compensation.uvt + pwm_compensation.uvd;
	float dt2 = pwm_compensation.dt;

	if(phase_currents.data[PHASE_A] > 0){
		pulse_times.data[0] += dt2;
	}else{
		pulse_times.data[0] -= dt2;
	}
	if(phase_currents.data[PHASE_B] > 0){
		pulse_times.data[1] += dt2;
	}else{
		pulse_times.data[1] -= dt2;
	}
	if(phase_currents.data[PHASE_C] > 0){
		pulse_times.data[2] += dt2;
	}else{
		pulse_times.data[2] -= dt2;
	}
	if(pulse_times.data[0] < 0.f){
		switch_times.data[0] = 0.f;
	}else {
		if(pulse_times.data[0] > 1.0f){
			switch_times.data[0] = 1.0f;
		}
	}
	if(pulse_times.data[1] < 0.f){
		pulse_times.data[1] = 0.f;
	}else {
		if(pulse_times.data[1] > 1.0f){
			pulse_times.data[1] = 1.0f;
		}
	}
	if(pulse_times.data[2] < 0.f){
		pulse_times.data[2] = 0.f;
	}else {
		if(pulse_times.data[2] > 1.0f){
			pulse_times.data[2] = 1.0f;
		}
	}
	for(int i = 0; i < 3; i++){
		switch_times.data[i] = (uint32_t)(pulse_times.data[i]*pwm_counter_period_);
	}
}
#endif

} // namespace emb


