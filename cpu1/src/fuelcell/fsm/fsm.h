/**
 * @defgroup fuel_cell_fsm Fuel Cell FSM
 *
 * @file
 * @ingroup fuel_cell_fsm
 */


#pragma once


#include "stdint.h"
#include "../fuelcell_def.h"
#include "emb/emb_algorithm.h"


namespace fuelcell {
/// @addtogroup fuel_cell_fsm
/// @{


extern unsigned int failureCount;


/// Forward declaration.
class Converter;


/**
 * @brief
 */
class IState
{
private:
	const ConverterState STATE_ID;
	static uint64_t s_timestamp;
protected:
	IState(ConverterState stateId) : STATE_ID(stateId) {}
	static void changeState(Converter* converter, IState* state);
public:
	virtual ~IState() {}
	ConverterState id() const { return STATE_ID; }
	uint64_t timestamp() const { return s_timestamp; }
	virtual void startup(Converter* converter) = 0;
	virtual void shutdown(Converter* converter) = 0;
	virtual void startCharging(Converter* converter) = 0;
	virtual void run(Converter* converter) = 0;
	virtual void stopCharging(Converter* converter) = 0;
	virtual void emergencyShutdown(Converter* converter) = 0;
};


/**
 * @brief
 */
class POWERUP_State : public IState
{
private:
	static POWERUP_State s_instance;
	POWERUP_State() : IState(CONVERTER_STANDBY) {}
public:
	static POWERUP_State* instance() { return &s_instance; }
	virtual void startup(Converter* converter);
	virtual void shutdown(Converter* converter);
	virtual void startCharging(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stopCharging(Converter* converter);
	virtual void emergencyShutdown(Converter* converter);
};


/**
 * @brief
 */
class STANDBY_State : public IState
{
private:
	static STANDBY_State s_instance;
	STANDBY_State() : IState(CONVERTER_STANDBY) {}
public:
	static STANDBY_State* instance() { return &s_instance; }
	virtual void startup(Converter* converter);
	virtual void shutdown(Converter* converter);
	virtual void startCharging(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stopCharging(Converter* converter);
	virtual void emergencyShutdown(Converter* converter);
};


/**
 * @brief
 */
class STARTUP_State : public IState
{
private:
	static STARTUP_State s_instance;
	STARTUP_State() : IState(CONVERTER_STARTUP) {}
public:
	static STARTUP_State* instance() { return &s_instance; }
	virtual void startup(Converter* converter);
	virtual void shutdown(Converter* converter);
	virtual void startCharging(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stopCharging(Converter* converter);
	virtual void emergencyShutdown(Converter* converter);
};


/**
 * @brief
 */
class READY_State : public IState
{
private:
	static READY_State s_instance;
	READY_State() : IState(CONVERTER_READY) {}
public:
	static READY_State* instance() { return &s_instance; }
	virtual void startup(Converter* converter);
	virtual void shutdown(Converter* converter);
	virtual void startCharging(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stopCharging(Converter* converter);
	virtual void emergencyShutdown(Converter* converter);
};


/**
 * @brief
 */
class CHARGING_START_State : public IState
{
private:
	static CHARGING_START_State s_instance;
	CHARGING_START_State() : IState(CONVERTER_CHARGING_START)
	{
		resetState();
	}
public:
	static CHARGING_START_State* instance() { return &s_instance; }
	virtual void startup(Converter* converter);
	virtual void shutdown(Converter* converter);
	virtual void startCharging(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stopCharging(Converter* converter);
	virtual void emergencyShutdown(Converter* converter);
private:
	void resetState()
	{
		m_currentInRef = 0;
	}
	float m_currentInRef;
};


/**
 * @brief
 */
class CHARGING_State : public IState
{
private:
	static CHARGING_State s_instance;
	CHARGING_State() : IState(CONVERTER_CHARGING) {}
public:
	static CHARGING_State* instance() { return &s_instance; }
	virtual void startup(Converter* converter);
	virtual void shutdown(Converter* converter);
	virtual void startCharging(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stopCharging(Converter* converter);
	virtual void emergencyShutdown(Converter* converter);
};


/**
 * @brief
 */
class CHARGING_STOP_State : public IState
{
private:
	static CHARGING_STOP_State s_instance;
	CHARGING_STOP_State() : IState(CONVERTER_CHARGING_STOP) {}
public:
	static CHARGING_STOP_State* instance() { return &s_instance; }
	virtual void startup(Converter* converter);
	virtual void shutdown(Converter* converter);
	virtual void startCharging(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stopCharging(Converter* converter);
	virtual void emergencyShutdown(Converter* converter);
};


/**
 * @brief
 */
class SHUTDOWN_State : public IState
{
private:
	static SHUTDOWN_State s_instance;
	SHUTDOWN_State() : IState(CONVERTER_SHUTDOWN) {}
public:
	static SHUTDOWN_State* instance() { return &s_instance; }
	virtual void startup(Converter* converter);
	virtual void shutdown(Converter* converter);
	virtual void startCharging(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stopCharging(Converter* converter);
	virtual void emergencyShutdown(Converter* converter);
};


/**
 * @brief
 */
class WAIT_State : public IState
{
	friend void changeStateAfterWait(Converter* converter, IState* nextState, uint64_t delay);
private:
	static WAIT_State s_instance;
	WAIT_State() : IState(CONVERTER_WAIT) {}
	static WAIT_State* instance() { return &s_instance; }	// waitNgoToState() must be used for going to this state
public:
	virtual void startup(Converter* converter);
	virtual void shutdown(Converter* converter);
	virtual void startCharging(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stopCharging(Converter* converter);
	virtual void emergencyShutdown(Converter* converter);
private:
	uint64_t m_waitStart;
	uint64_t m_waitTime;
	IState* m_nextState;
};


/// @}
} // namespace fuelcell


