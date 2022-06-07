///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include "emb_algorithm.h"
#include "float.h"


namespace emb {


/*
 * @brief PI controller interface
 */
class IPiController
{
private:
	IPiController(const IPiController& other);		// no copy constructor
	IPiController& operator=(const IPiController& other);	// no copy assignment operator

protected:
	float m_kP;		// proportional gain
	float m_kI;		// integral gain
	float m_dt;		// time slice
	float m_sumI;		// integrator sum;
	float m_outMin;		// PI output minimum limit
	float m_outMax;		// PI output maximum limit
	float m_out;		// PI output;

public:
	IPiController(float kP, float kI, float dt, float outMin, float outMax)
		: m_kP(kP)
		, m_kI(kI)
		, m_dt(dt)
		, m_sumI(0)
		, m_outMin(outMin)
		, m_outMax(outMax)
		, m_out(0)
	{}

	virtual ~IPiController() {}
	virtual void process(float ref, float meas) = 0;
	virtual void reset()
	{
		m_sumI = 0;
		m_out = 0;
	}
	float output() const { return m_out; }
	void setOutputMin(float value) { m_outMin = value; }
	void setOutputMax(float value) { m_outMax = value; }
	float outputMin() const { return m_outMin; }
	float outputMax() const { return m_outMax; }

	void setKp(float value) { m_kP = value; }
	void setKi(float value) { m_kI = value; }
	float kP() const { return m_kP; }
	float kI() const { return m_kI; }
	float sumI() const { return m_sumI; }
};


/*
 * @brief PI controller with back-calculation
 */
class PiControllerBC : public IPiController
{
private:
	PiControllerBC(const PiControllerBC& other);		// no copy constructor
	PiControllerBC& operator=(const PiControllerBC& other);	// no copy assignment operator

protected:
	float m_kC;		// anti-windup gain

public:
	PiControllerBC(float kP, float kI, float dt, float kC, float outMin, float outMax)
		: IPiController(kP, kI, dt, outMin, outMax)
		, m_kC(kC)
	{}

	virtual void process(float ref, float meas)
	{
		float error = ref - meas;
		float out = emb::clamp(error * m_kP + m_sumI, -FLT_MAX, FLT_MAX);

		if (out > m_outMax)
		{
			m_out = m_outMax;
		}
		else if (out < m_outMin)
		{
			m_out = m_outMin;
		}
		else
		{
			m_out = out;
		}

		m_sumI = emb::clamp(m_sumI + m_kI * m_dt * error - m_kC * (out - m_out), -FLT_MAX, FLT_MAX);
	}
};


/*
 * @brief PI controller with clamping
 */
class PiControllerCl : public IPiController
{
private:
	PiControllerCl(const PiControllerCl& other);		// no copy constructor
	PiControllerCl& operator=(const PiControllerCl& other);	// no copy assignment operator

protected:
	float m_error;

public:
	PiControllerCl(float kP, float kI, float dt, float outMin, float outMax)
		: IPiController(kP, kI, dt, outMin, outMax)
		, m_error(0)
	{}

	virtual void process(float ref, float meas)
	{
		float error = ref - meas;
		float outp = error * m_kP;
		float sumI = (error + m_error) * 0.5f * m_kI * m_dt + m_sumI;
		m_error = error;
		float out = outp + sumI;

		if (out > m_outMax)
		{
			m_out =  m_outMax;
			if (outp < m_outMax)
			{
				m_sumI = m_outMax - outp;
			}
		}
		else if (out < m_outMin)
		{
			m_out =  m_outMin;
			if (outp > m_outMin)
			{
				m_sumI = m_outMin - outp;
			}
		}
		else
		{
			m_out = out;
			m_sumI = sumI;
		}
	}

	virtual void reset()
	{
		m_sumI = 0;
		m_error = 0;
		m_out = 0;
	}
};


} // namespace emb


