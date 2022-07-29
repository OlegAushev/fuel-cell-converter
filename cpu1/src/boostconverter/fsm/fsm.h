/**
 * @defgroup boost_converter_fsm FSM
 * @ingroup boost_converter
 *
 * @file
 * @ingroup boost_converter boost_converter_fsm
 */


#pragma once


#include "../boostconverterdef.h"


/// Forward declaration.
class BoostConverter;


namespace boostconverter {
/// @addtogroup boost_converter
/// @{


/**
 * @brief
 */
class IState
{
private:
	const State STATE_ID;
protected:
	IState(State stateId) : STATE_ID(stateId) {}
	void changeState(BoostConverter* converter, IState* state);
public:
	virtual ~IState() {}
	State id() const { return STATE_ID; }
	virtual void start(BoostConverter* converter) = 0;
	virtual void run(BoostConverter* converter) = 0;
	virtual void stop(BoostConverter* converter) = 0;
};


/**
 * @brief
 */
class STANDBY_State : public IState
{
private:
	static STANDBY_State s_instance;
	STANDBY_State() : IState(STANDBY) {}
public:
	static STANDBY_State* instance() { return &s_instance; }
	virtual void start(BoostConverter* drive);
	virtual void run(BoostConverter* drive);
	virtual void stop(BoostConverter* drive);
};


/**
 * @brief
 */
class POWERUP_State : public IState
{
private:
	static POWERUP_State s_instance;
	POWERUP_State() : IState(POWERUP) {}
public:
	static POWERUP_State* instance() { return &s_instance; }
	virtual void start(BoostConverter* drive);
	virtual void run(BoostConverter* drive);
	virtual void stop(BoostConverter* drive);
};


/**
 * @brief
 */
class READY_State : public IState
{
private:
	static READY_State s_instance;
	READY_State() : IState(READY) {}
public:
	static READY_State* instance() { return &s_instance; }
	virtual void start(BoostConverter* drive);
	virtual void run(BoostConverter* drive);
	virtual void stop(BoostConverter* drive);
};


/**
 * @brief
 */
class STARTING_State : public IState
{
private:
	static STARTING_State s_instance;
	STARTING_State() : IState(STARTING) {}
public:
	static STARTING_State* instance() { return &s_instance; }
	virtual void start(BoostConverter* drive);
	virtual void run(BoostConverter* drive);
	virtual void stop(BoostConverter* drive);
};


/**
 * @brief
 */
class IN_OPERATION_State : public IState
{
private:
	static IN_OPERATION_State s_instance;
	IN_OPERATION_State() : IState(IN_OPERATION) {}
public:
	static IN_OPERATION_State* instance() { return &s_instance; }
	virtual void start(BoostConverter* drive);
	virtual void run(BoostConverter* drive);
	virtual void stop(BoostConverter* drive);
};


/**
 * @brief
 */
class STOPPING_State : public IState
{
private:
	static STOPPING_State s_instance;
	STOPPING_State() : IState(STOPPING) {}
public:
	static STOPPING_State* instance() { return &s_instance; }
	virtual void start(BoostConverter* drive);
	virtual void run(BoostConverter* drive);
	virtual void stop(BoostConverter* drive);
};


/**
 * @brief
 */
class POWERDOWN_State : public IState
{
private:
	static POWERDOWN_State s_instance;
	POWERDOWN_State() : IState(POWERDOWN) {}
public:
	static POWERDOWN_State* instance() { return &s_instance; }
	virtual void start(BoostConverter* drive);
	virtual void run(BoostConverter* drive);
	virtual void stop(BoostConverter* drive);
};


/// @}
} // namespace boostconverter


