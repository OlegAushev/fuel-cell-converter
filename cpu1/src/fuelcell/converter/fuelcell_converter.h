/**
 * @file
 * @ingroup fuel_cell_converter
 */


#pragma once


#include "emb/emb_filter.h"
#include "emb/emb_pair.h"
#include "emb/emb_picontroller.h"
#include "mcu_c28x/pwm/mcu_pwm.h"
#include "../fuelcell_def.h"
#include "../fsm/fsm.h"
#include "sensors/currentsensors.h"
#include "sensors/voltagesensors.h"
#include "sensors/temperaturesensors.h"
#include "sys/syslog/syslog.h"

#include "profiler/profiler.h"


namespace fuelcell {
/// @addtogroup fuel_cell_converter
/// @{


/**
 * @brief Converter config.
 */
struct ConverterConfig
{
	float uvpVoltageIn;
	float ovpVoltageIn;
	float ovpVoltageOut;
	float ocpCurrentIn;

	float otpTempJunction;
	float otpTempHeatsink;
	float fanTempThOn;
	float fanTempThOff;

	float kP_dutycycle;
	float kI_dutycucle;
	float kP_current;
	float kI_current;

	float fuelCellVoltageMin;
	float cvVoltageIn;
	float currentInMin;
	float currentInMax;
	float batteryChargedVoltage;

	mcu::GpioInput fltPin;
};


/**
 * @brief Converter class.
 */
class Converter : public emb::c28x::Singleton<Converter>
{
	friend class IState;
	friend class STANDBY_State;
	friend class IDLE_State;
	friend class STARTUP_State;
	friend class READY_State;
	friend class CHARGING_START_State;
	friend class CHARGING_State;
	friend class CHARGING_STOP_State;
	friend class SHUTDOWN_State;
	friend class WAIT_State;
private:
	IState* m_state;
	ConverterState m_stateId;

	ConverterConfig m_config;

	static const float VDC_SMOOTH_FACTOR = 0.001;
	emb::ExponentialMedianFilter<float, 3> m_voltageInFilter;
	emb::ExponentialMedianFilter<float, 3> m_voltageOutFilter;
	emb::Pair<float, float> m_currentIn;	// inductor current measured twice per PWM period
	static const float IDC_SMOOTH_FACTOR = 0.1;
	emb::ExponentialMedianFilter<float, 3> m_currentInFilter;
	static const float TEMP_SMOOTH_FACTOR = 0.001;
	emb::ExponentialMedianFilter<float, 5> m_tempHeatsinkFilter;

	emb::PiControllerCl<emb::CONTROLLER_DIRECT> m_dutycycleController;
	emb::PiControllerCl<emb::CONTROLLER_INVERSE> m_currentController;

	const mcu::GpioInput FLT_PIN;
	const mcu::GpioOutput RST_PIN;
	const mcu::GpioOutput ERR_PIN;
	const mcu::GpioOutput REL_PIN;
public:
	mcu::Pwm<mcu::PWM_ONE_PHASE> pwm;
	InVoltageSensor inVoltageSensor;
	OutVoltageSensor outVoltageSensor;
	InCurrentSensor inCurrentSensor;
	TemperatureSensor tempSensor;

private:
	Converter(const Converter& other);		// no copy constructor
	Converter& operator=(const Converter& other);	// no copy assignment operator
public:
	/**
	 * @brief Constructs a new Converter object.
	 * @param converterConfig - converter config
	 * @param pwmConfig - PWM config
	 */
	Converter(const ConverterConfig& converterConfig,
			const mcu::PwmConfig<mcu::PWM_ONE_PHASE>& pwmConfig);

	// FSM methods
	void startup() { m_state->startup(this); }
	void shutdown() { m_state->shutdown(this); }
	void startCharging() { m_state->startCharging(this); }
	void run() { m_state->run(this); }
	void stopCharging() { m_state->stopCharging(this); }
	void emergencyShutdown() { m_state->emergencyShutdown(this); }

	/**
	 * @brief Returns converter state.
	 * @param (none)
	 * @return Converter state.
	 */
	ConverterState state() const { return m_stateId; }

private:
	/**
	 * @brief Starts converter.
	 * @param (none)
	 * @return (none)
	 */
	void start()
	{
		if ((Syslog::errors() == 0)
				&& (!Syslog::hasWarning(sys::Warning::BATTERY_CHARGED)
				&& (pwm.state() == mcu::PWM_OFF)))
		{
			pwm.start();
		}
	}

	/**
	 * @brief Stops converter.
	 * @param (none)
	 * @return (none)
	 */
	void stop()
	{
		pwm.stop();
		m_currentController.reset();
		m_dutycycleController.reset();
	}

	/**
	 * @brief Resets controller.
	 * @param (none)
	 * @return (none)
	 */
	void reset();

public:
	float voltageIn() const { return m_voltageInFilter.output(); }
	float voltageOut() const { return m_voltageOutFilter.output(); }
	float currentIn() const { return m_currentInFilter.output(); }
	float tempHeatsink() const { return m_tempHeatsinkFilter.output(); }

	void setCurrentIn(float value)
	{
		m_currentController.setOutputMax(
				emb::clamp(value, m_config.currentInMin, m_config.currentInMax));
	}

	/**
	 * @brief Checks if temperature measurements are completed and processes them.
	 * @param (none)
	 * @return (none)
	 */
	void processTemperatureMeasurements();

	const ConverterConfig& config() const { return m_config; }

	void turnRelayOn() const
	{
#ifndef CRD300
		REL_PIN.set();
#endif
	}
	void turnRelayOff() const
	{
#ifndef CRD300
		REL_PIN.reset();
#endif
	}

protected:
	static __interrupt void onPwmEventInterrupt();
	static __interrupt void onPwmTripInterrupt();
	static __interrupt void onAdcVoltageInInterrupt();
	static __interrupt void onAdcVoltageOutInterrupt();
	static __interrupt void onAdcCurrentInFirstInterrupt();
	static __interrupt void onAdcCurrentInSecondInterrupt();
	static __interrupt void onAdcTempHeatsinkInterrupt();

private:
	void changeState(IState* state)
	{
		m_state = state;
		m_stateId = state->id();
	}
};


/// @}
} // namespace fuelcell

