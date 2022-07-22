/**
 * @defgroup fuel_cell_controller Fuel Cell Controller
 *
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
#include "boostconverter/boostconverter.h"
#include "canbygpio/canbygpio.h"


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


/// Fuel cell status
enum Status
{
	FUELCELL_NA,
	FUELCELL_STANDBY,
	FUELCELL_READY,
	FUELCELL_INOP,
	FUELCELL_OVERHEAT,
	FUELCELL_BATT_LOWCHARGE,
	FUELCELL_NOCONNECTION,
	FUELCELL_LOWPRESSURE,
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
	emb::Array<Status, FUELCELL_COUNT> status;
	emb::Array<float, FUELCELL_COUNT> current;
};


class Controller
{
private:
	const BoostConverter* m_converter;
	canbygpio::Transceiver m_transceiver;
	static const mcu::IpcFlag SIG_START;
	static const mcu::IpcFlag SIG_STOP;

	static Data s_data;

	static const uint64_t TPDO_PERIOD = 200;
	static const unsigned int TPDO_FRAME_ID = 0x200;

public:
	Controller(const BoostConverter* converter,
			const mcu::Gpio& txPin, const mcu::Gpio& rxPin, mcu::Gpio& clkPin);

	void run()
	{
		_runTx();
		_runRx();
	}

	static void start()
	{
		mcu::setLocalIpcFlag(SIG_START.local);
	}

	static void stop()
	{
		mcu::setLocalIpcFlag(SIG_STOP.local);
	}

	static const Data& data()
	{
		return s_data;
	}

private:
	void _runTx();
	void _runRx();
};


/// @}
} // namespace fuelcell


