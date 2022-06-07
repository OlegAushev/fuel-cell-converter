///
#pragma once


#include <stdint.h>
#include <stddef.h>

#include "motorcontrol/math.h"
#include "motorcontrol/clarke.h"
#include "motorcontrol/park.h"
#include "motorcontrol/ipark.h"

#include "emb_math.h"
#include "emb_array.h"


namespace emb {


/**
 * @brief
 */
class motorspeed_t
{
private:
	const int POLE_PAIRS;
	float m_radps_elec;
public:
	explicit motorspeed_t(int polePairs)
		: POLE_PAIRS(polePairs)
		, m_radps_elec(0)
	{}

	motorspeed_t(float radpsElec, int polePairs)
		: POLE_PAIRS(polePairs)
		, m_radps_elec(radpsElec)
	{}

	float to_radps() const { return m_radps_elec; }
	float to_rpm() const { return 60 * m_radps_elec / (2 * PI * POLE_PAIRS); }
	float to_radps_mech() const { return m_radps_elec / POLE_PAIRS; }

	void from_radps(float value) { m_radps_elec = value; }
	void from_rpm(float value) { m_radps_elec = 2 * PI * POLE_PAIRS * value / 60; }
};


/**
 * @brief
 */
inline float to_radps(float speed_rpm, int polePairs) { return 2 * PI * polePairs * speed_rpm / 60; }


/**
 * @brief
 */
inline float to_radps(float speed_rpm) { return 2 * PI * speed_rpm / 60; }


/**
 * @brief
 */
inline float to_rpm(float speed_radps, int polePairs) { return 60 * speed_radps / (2 * PI * polePairs); }


/**
 * @brief
 */
inline emb::Array<float, 3> calculate_svpwm(float voltageMag, float voltageAngle, float voltageDc)
{
	voltageAngle = normalize_2pi(voltageAngle);
	voltageMag = clamp<float>(voltageMag, 0, voltageDc / SQRT_3);

	int32_t sector = static_cast<int32_t>(voltageAngle / PI_OVER_3);
	float theta = voltageAngle - float(sector) * PI_OVER_3;

	// base vector times calculation
	float tb1 = SQRT_3 * (voltageMag / voltageDc) * sinf(PI_OVER_3 - theta);
	float tb2 = SQRT_3 * (voltageMag / voltageDc) * sinf(theta);
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

	for(uint32_t i = 0; i < 3; ++i)
	{
		pulseTimes[i] = clamp<float>(pulseTimes[i], 0.f, 1.f);
	}
	return pulseTimes;
}


/**
 * @brief
 */
struct DQPair
{
	float d;
	float q;
	DQPair() {}
	DQPair(float d_, float q_) : d(d_), q(q_) {}
};


/**
 * @brief
 */
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


} // namespace emb













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






