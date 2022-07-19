///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include "emb_algorithm.h"
#include "float.h"


namespace emb {


/// Controller logic
enum ControllerLogic
{
	CONTROLLER_DIRECT,
	CONTROLLER_INVERSE
};


/*
 * @brief PI controller interface
 */
template <ControllerLogic Logic>
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

	static float _error(float ref, float meas);
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
	virtual void update(float ref, float meas) = 0;
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


inline float IPiController<CONTROLLER_DIRECT>::_error(float ref, float meas) { return ref - meas; }
inline float IPiController<CONTROLLER_INVERSE>::_error(float ref, float meas) { return meas - ref; }


/*
 * @brief PI controller with back-calculation
 */
template <ControllerLogic Logic>
class PiControllerBC : public IPiController<Logic>
{
private:
	PiControllerBC(const PiControllerBC& other);		// no copy constructor
	PiControllerBC& operator=(const PiControllerBC& other);	// no copy assignment operator

protected:
	float m_kC;		// anti-windup gain

public:
	PiControllerBC(float kP, float kI, float dt, float kC, float outMin, float outMax)
		: IPiController<Logic>(kP, kI, dt, outMin, outMax)
		, m_kC(kC)
	{}

	virtual void update(float ref, float meas)
	{
		float error = IPiController<Logic>::_error(ref, meas);
		float out = emb::clamp(error * this->m_kP + this->m_sumI, -FLT_MAX, FLT_MAX);

		if (out > this->m_outMax)
		{
			this->m_out = this->m_outMax;
		}
		else if (out < this->m_outMin)
		{
			this->m_out = this->m_outMin;
		}
		else
		{
			this->m_out = out;
		}

		this->m_sumI = emb::clamp(this->m_sumI + this->m_kI * this->m_dt * error - m_kC * (out - this->m_out),
				-FLT_MAX, FLT_MAX);
	}
};


/*
 * @brief PI controller with clamping
 */
template <ControllerLogic Logic>
class PiControllerCl : public IPiController<Logic>
{
private:
	PiControllerCl(const PiControllerCl& other);		// no copy constructor
	PiControllerCl& operator=(const PiControllerCl& other);	// no copy assignment operator

protected:
	float m_error;

public:
	PiControllerCl(float kP, float kI, float dt, float outMin, float outMax)
		: IPiController<Logic>(kP, kI, dt, outMin, outMax)
		, m_error(0)
	{}

	virtual void update(float ref, float meas)
	{
		float error = IPiController<Logic>::_error(ref, meas);
		float outp = error * this->m_kP;
		float sumI = (error + m_error) * 0.5f * this->m_kI * this->m_dt + this->m_sumI;
		m_error = error;
		float out = outp + sumI;

		if (out > this->m_outMax)
		{
			this->m_out = this->m_outMax;
			if (outp < this->m_outMax)
			{
				this->m_sumI = this->m_outMax - outp;
			}
		}
		else if (out < this->m_outMin)
		{
			this->m_out = this->m_outMin;
			if (outp > this->m_outMin)
			{
				this->m_sumI = this->m_outMin - outp;
			}
		}
		else
		{
			this->m_out = out;
			this->m_sumI = sumI;
		}
	}

	virtual void reset()
	{
		this->m_sumI = 0;
		m_error = 0;
		this->m_out = 0;
	}
};


} // namespace emb


