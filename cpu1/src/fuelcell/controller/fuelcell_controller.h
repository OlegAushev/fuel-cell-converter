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
	uint64_t status : 8;
	uint64_t current : 16;
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
	emb::Array<FuelcellStatus, FUELCELL_COUNT> status;
	emb::Array<float, FUELCELL_COUNT> current;
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
	 * @param (none)
	 * @return
	 */
	static bool inOperation()
	{
		return emb::count(s_data.status.begin(), s_data.status.end(), FUELCELL_INOP) == FUELCELL_COUNT;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool standby()
	{
		return emb::count(s_data.status.begin(), s_data.status.end(), FUELCELL_STANDBY) == FUELCELL_COUNT;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool hasOverheat()
	{
		bool fault = emb::count(s_data.status.begin(), s_data.status.end(), FUELCELL_OVERHEAT) > 0;
		if (fault)
		{
			Syslog::setError(sys::Error::FUELCELL_OVERHEAT);
		}
		return fault;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool hasBattLowCharge()
	{
		bool fault = emb::count(s_data.status.begin(), s_data.status.end(), FUELCELL_BATT_LOWCHARGE) > 0;
		if (fault)
		{
			Syslog::setError(sys::Error::FUELCELL_BATT_LOWCHARGE);
		}
		return fault;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool hasNoConnection()
	{
		bool fault = emb::count(s_data.status.begin(), s_data.status.end(), FUELCELL_NOCONNECTION) > 0;
		if (fault)
		{
			Syslog::setError(sys::Error::FUELCELL_NOCONNECTION);
		}
		return fault;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool hasLowPressure()
	{
		bool fault = emb::count(s_data.status.begin(), s_data.status.end(), FUELCELL_LOWPRESSURE) > 0;
		if (fault)
		{
			Syslog::setError(sys::Error::FUELCELL_LOWPRESSURE);
		}
		return fault;
	}

	/**
	 * @brief
	 * @param (none)
	 * @return
	 */
	static bool fault()
	{
		return hasOverheat() || hasBattLowCharge()
				|| hasNoConnection() || hasLowPressure();
	}

	/**
	 * @brief Returns state code of fuel cells.
	 * @param (none)
	 * @return State code of fuel cells.
	 */
	static uint32_t state()
	{
		uint32_t ret = 0;
		ret += 10000 * s_data.status[0];
		ret += 1000 * s_data.status[1];
		ret += 100 * s_data.status[2];
		ret += 10 * s_data.status[3];
		ret += 1 * s_data.status[4];
		return ret;
	}

private:
	void runTx();
	void runRx();
};


/// @}
} // namespace fuelcell


