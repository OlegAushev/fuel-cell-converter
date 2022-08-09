/**
 * @file
 * @ingroup fuel_cell_controller
 */


#pragma once


#include "emb/emb_common.h"
#include "emb/emb_algorithm.h"
#include "emb/emb_array.h"
#include "emb/emb_math.h"
#include "mcu/cputimers/mcu_cputimers.h"
#include "mcu/ipc/mcu_ipc.h"
#include "canbygpio/canbygpio.h"
#include "../fuelcell_def.h"
#include "../converter/fuelcell_converter.h"
#include "sys/syslog/syslog.h"


namespace fuelcell {
/// @addtogroup fuel_cell_controller
/// @{


/**
 * @brief TPDO message.
 */
struct TpdoMessage
{
	uint64_t cmd : 8;
	uint64_t voltage : 16;
	uint64_t current : 16;
	uint64_t reserved : 24;

	TpdoMessage()
	{
		uint64_t rawMsg = 0;
		memcpy(this, &rawMsg, sizeof(TpdoMessage));
	}
};


/**
 * @brief RPDO message.
 */
struct RpdoMessage
{
	uint64_t temperature : 8;
	uint64_t cellVoltage : 16;
	uint64_t battVoltage : 16;

	uint64_t statusError : 1;
	uint64_t statusStart : 1;
	uint64_t statusRun : 1;
	uint64_t statusOverheat : 1;
	uint64_t statusLowCharge : 1;
	uint64_t statusNoConnection : 1;
	uint64_t statusLowPressure : 1;
	uint64_t statusHydroError : 1;

	int16_t current : 16;
};


const size_t FUELCELL_COUNT = 5;


/**
 * @brief Fuel cell data.
 */
struct Data
{
	emb::Array<float, FUELCELL_COUNT> temperature;
	emb::Array<float, FUELCELL_COUNT> cellVoltage;
	emb::Array<float, FUELCELL_COUNT> battVoltage;
	emb::Array<float, FUELCELL_COUNT> current;

	bool statusError;
	emb::Array<bool, FUELCELL_COUNT> statusStart;
	emb::Array<bool, FUELCELL_COUNT> statusRun;
	emb::Array<bool, FUELCELL_COUNT> statusOverheat;
	emb::Array<bool, FUELCELL_COUNT> statusLowCharge;
	bool statusNoConnection;
	bool statusLowPressure;
	bool statusHydroError;

};


class Controller
{
private:
	const Converter* m_converter;
	canbygpio::Transceiver m_transceiver;
	static const mcu::IpcFlag SIG_START;
	static const mcu::IpcFlag SIG_STOP;

	static Data s_data;

	static const uint64_t TPDO_PERIOD = 200;
	static const unsigned int TPDO_FRAME_ID = 0x200;

	static uint64_t m_errorRegTimestamp;
	static bool m_isErrorRegistered;
	static const uint64_t m_errorDelay = 5000;

public:
	static const float MIN_OPERATING_VOLTAGE = 35;
	static const float MAX_OPERATING_VOLTAGE = 42;

	static const float ABSOLUTE_MIN_VOLTAGE = 32;
	static const float ABSOLUTE_MAX_VOLTAGE = 45;

public:
	Controller(const Converter* converter,
			const mcu::Gpio& rxPin, const mcu::Gpio& txPin, mcu::Gpio& clkPin);

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	void run()
	{
		runTx();
		runRx();
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static void start()
	{
		mcu::setLocalIpcFlag(SIG_START.local);
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static void stop()
	{
		disableErrors();	// errors after stop must not be registered
		mcu::setLocalIpcFlag(SIG_STOP.local);
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static const Data& data()
	{
		return s_data;
	}

	/**
	 * @brief
	 * @param
	 * @return
	 */
	static bool hasError()
	{
		return s_data.statusError;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool isStarting()
	{
		return emb::count(s_data.statusStart.begin(), s_data.statusStart.end(), true) == FUELCELL_COUNT;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool isRunning()
	{
		return emb::count(s_data.statusRun.begin(), s_data.statusRun.end(), true) == FUELCELL_COUNT;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool hasOverheat()
	{
		return emb::count(s_data.statusOverheat.begin(), s_data.statusOverheat.end(), true) > 0;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool hasLowCharge()
	{
		return emb::count(s_data.statusLowCharge.begin(), s_data.statusLowCharge.end(), true) > 0;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool hasNoConnection()
	{
		return s_data.statusNoConnection;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool hasLowPressure()
	{
		return s_data.statusLowPressure;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool hasHydroError()
	{
		return s_data.statusHydroError;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool checkErrors()
	{
		static bool errorPrev = false;
		bool errorDelayExpired = m_isErrorRegistered
				&& (mcu::SystemClock::now() - m_errorRegTimestamp > m_errorDelay);
		bool error = false;

		if (hasError())
		{
			error = true;
			if (errorDelayExpired)
			{
				Syslog::setError(sys::Error::FUELCELL_ERROR);
			}
		}

		if (hasOverheat())
		{
			error = true;
			if (errorDelayExpired)
			{
				Syslog::setError(sys::Error::FUELCELL_OVERHEAT);
			}
		}

		if (hasLowCharge())
		{
			error = true;
			if (errorDelayExpired)
			{
				Syslog::setError(sys::Error::FUELCELL_BATT_LOWCHARGE);
			}
		}

		if (hasNoConnection())
		{
			error = true;
			if (errorDelayExpired)
			{
				Syslog::setError(sys::Error::FUELCELL_NOCONNECTION);
			}
		}

		if (hasLowPressure())
		{
			error = true;
			if (errorDelayExpired)
			{
				Syslog::setError(sys::Error::FUELCELL_LOWPRESSURE);
			}
		}

		if (hasHydroError())
		{
			error = true;
			if (errorDelayExpired)
			{
				Syslog::setError(sys::Error::FUELCELL_HYDROERROR);
			}
		}

		for (size_t i = 0; i < s_data.cellVoltage.size(); ++i)
		{
			if (s_data.cellVoltage[i] < ABSOLUTE_MIN_VOLTAGE)
			{
				error = true;
				if (errorDelayExpired)
				{
					Syslog::setError(sys::Error::FUELCELL_UV);
				}
			}
			else if (s_data.cellVoltage[i] > ABSOLUTE_MAX_VOLTAGE)
			{
				error = true;
				if (errorDelayExpired)
				{
					Syslog::setError(sys::Error::FUELCELL_OV);
				}
			}
		}

		if (error && !errorPrev)
		{
			m_isErrorRegistered = true;
			m_errorRegTimestamp = mcu::SystemClock::now();
		}
		else if (!error)
		{
			m_isErrorRegistered = false;
		}

		errorPrev = error;
		return error;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static float minCellVoltage()
	{
		return *emb::min_element(s_data.cellVoltage.begin(), s_data.cellVoltage.end());
	}

	/**
	 * @brief Returns state code of fuel cells.
	 * @param (none)
	 * @return State code of fuel cells.
	 */
	static uint32_t state()
	{
//		uint32_t bitError = hasError() ? 1 : 0;
//		uint32_t bitReady = isStarting() ? 1 : 0;
//		uint32_t bitInop = isRunning() ? 1 : 0;
//		uint32_t bitOverheat = hasOverheat() ? 1 : 0;
//		uint32_t bitLowvo = hasLowCharge() ? 1 : 0;
//		uint32_t bitNoconn = hasNoConnection() ? 1 : 0;
//		uint32_t bitLowpr = hasLowPressure() ? 1 : 0;

		uint32_t ret = 0;

		//ret = bitError | (bitReady << 1) | (bitInop << 2) | (bitOverheat << 3)
		//		| (bitLowvo << 4) | (bitNoconn << 5) | (bitLowpr << 6);
		return ret;
	}

	/**
	 * @brief
	 * @param
	 * @return
	 */
	static void disableErrors()
	{
		Syslog::disableError(sys::Error::FUELCELL_ERROR);
		Syslog::disableError(sys::Error::FUELCELL_OVERHEAT);
		Syslog::disableError(sys::Error::FUELCELL_BATT_LOWCHARGE);
		Syslog::disableError(sys::Error::FUELCELL_NOCONNECTION);
		Syslog::disableError(sys::Error::FUELCELL_LOWPRESSURE);
		Syslog::disableError(sys::Error::FUELCELL_HYDROERROR);
		Syslog::disableError(sys::Error::FUELCELL_UV);
		Syslog::disableError(sys::Error::FUELCELL_OV);
	}

	/**
	 * @brief
	 * @param
	 * @return
	 */
	static void enableErrors()
	{
		Syslog::enableError(sys::Error::FUELCELL_ERROR);
		Syslog::enableError(sys::Error::FUELCELL_OVERHEAT);
		Syslog::enableError(sys::Error::FUELCELL_BATT_LOWCHARGE);
		Syslog::enableError(sys::Error::FUELCELL_NOCONNECTION);
		Syslog::enableError(sys::Error::FUELCELL_LOWPRESSURE);
		Syslog::enableError(sys::Error::FUELCELL_HYDROERROR);
		Syslog::enableError(sys::Error::FUELCELL_UV);
		Syslog::enableError(sys::Error::FUELCELL_OV);
	}

private:
	void runTx();
	void runRx();
};


/// @}
} // namespace fuelcell


