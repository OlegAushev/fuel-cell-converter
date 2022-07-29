/**
 * @defgroup fuel_cell_fsm Fuel Cell FSM
 *
 * @file
 * @ingroup fuel_cell_fsm
 */


#pragma once


#include "../fuelcell_def.h"


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
protected:
	IState(ConverterState stateId) : STATE_ID(stateId) {}
	void changeState(Converter* converter, IState* state);
public:
	virtual ~IState() {}
	ConverterState id() const { return STATE_ID; }
	virtual void start(Converter* converter) = 0;
	virtual void run(Converter* converter) = 0;
	virtual void stop(Converter* converter) = 0;
	virtual void emergencyStop(Converter* converter) = 0;
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
	virtual void start(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stop(Converter* converter);
	virtual void emergencyStop(Converter* converter);
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
	virtual void start(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stop(Converter* converter);
	virtual void emergencyStop(Converter* converter);
};


/**
 * @brief
 */
class POWERUP_State : public IState
{
private:
	static POWERUP_State s_instance;
	POWERUP_State() : IState(CONVERTER_POWERUP) {}
public:
	static POWERUP_State* instance() { return &s_instance; }
	virtual void start(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stop(Converter* converter);
	virtual void emergencyStop(Converter* converter);
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
	virtual void start(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stop(Converter* converter);
	virtual void emergencyStop(Converter* converter);
};


/**
 * @brief
 */
class STARTING_State : public IState
{
private:
	static STARTING_State s_instance;
	STARTING_State() : IState(CONVERTER_STARTING) {}
public:
	static STARTING_State* instance() { return &s_instance; }
	virtual void start(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stop(Converter* converter);
	virtual void emergencyStop(Converter* converter);
};


/**
 * @brief
 */
class IN_OPERATION_State : public IState
{
private:
	static IN_OPERATION_State s_instance;
	IN_OPERATION_State() : IState(CONVERTER_IN_OPERATION) {}
public:
	static IN_OPERATION_State* instance() { return &s_instance; }
	virtual void start(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stop(Converter* converter);
	virtual void emergencyStop(Converter* converter);
};


/**
 * @brief
 */
class STOPPING_State : public IState
{
private:
	static STOPPING_State s_instance;
	STOPPING_State() : IState(CONVERTER_STOPPING) {}
public:
	static STOPPING_State* instance() { return &s_instance; }
	virtual void start(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stop(Converter* converter);
	virtual void emergencyStop(Converter* converter);
};


/**
 * @brief
 */
class POWERDOWN_State : public IState
{
private:
	static POWERDOWN_State s_instance;
	POWERDOWN_State() : IState(CONVERTER_POWERDOWN) {}
public:
	static POWERDOWN_State* instance() { return &s_instance; }
	virtual void start(Converter* converter);
	virtual void run(Converter* converter);
	virtual void stop(Converter* converter);
	virtual void emergencyStop(Converter* converter);
};


/// @}
} // namespace fuelcell


