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
	void changeState(Converter* converter, IState* state);
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
class STARTCHARGING_State : public IState
{
private:
	static STARTCHARGING_State s_instance;
	STARTCHARGING_State() : IState(CONVERTER_START_CHARGING)
	{
		resetState();
	}
public:
	static STARTCHARGING_State* instance() { return &s_instance; }
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
class INOPERATION_State : public IState
{
private:
	static INOPERATION_State s_instance;
	INOPERATION_State() : IState(CONVERTER_IN_OPERATION) {}
public:
	static INOPERATION_State* instance() { return &s_instance; }
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
class STOPCHARGING_State : public IState
{
private:
	static STOPCHARGING_State s_instance;
	STOPCHARGING_State() : IState(CONVERTER_STOP_CHARGING) {}
public:
	static STOPCHARGING_State* instance() { return &s_instance; }
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
class IDLE_State : public IState
{
private:
	static IDLE_State s_instance;
	IDLE_State() : IState(CONVERTER_IDLE) {}
public:
	static IDLE_State* instance() { return &s_instance; }
	virtual void startup(Converter* converter);
	virtual void shutdown(Converter* converter);
	virtual void startCharging(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stopCharging(Converter* converter);
	virtual void emergencyShutdown(Converter* converter);
};


/// @}
} // namespace fuelcell


