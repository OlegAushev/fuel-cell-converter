/**
 * @defgroup fuel_cell_controller Fuel Cell Controller
 *
 * @file
 * @ingroup fuel_cell_controller
 */


#pragma once


#include "emb/emb_common.h"
#include "emb/emb_algorithm.h"
#include "mcu/cputimers/mcucputimers.h"
#include "mcu/ipc/mcuipc.h"
#include "boostconverter/boostconverter.h"
#include "canbygpio/canbygpio.h"


/**
 * @brief TPDO message.
 */
struct FuelCellTpdo
{
	uint64_t cmd : 8;
	uint64_t voltage : 16;
	uint64_t current : 16;
	uint64_t reserved : 24;
};


/**
 * @brief RPDO message.
 */
struct FuelCellRpdo
{
	uint64_t temperature : 8;
	uint64_t cellVoltage : 16;
	uint64_t battVoltage : 16;
	uint64_t status : 8;
	uint64_t current : 16;
};


class FuelCellController
{
private:
	const BoostConverter* m_converter;
	canbygpio::Transceiver m_transceiver;
	static const mcu::IpcFlag SIG_START;
	static const mcu::IpcFlag SIG_STOP;

	static const uint64_t TPDO_PERIOD = 200;
	static const unsigned int TPDO_FRAME_ID = 0x200;

public:
	FuelCellController(const BoostConverter* converter,
			const mcu::GpioPin& txPin, const mcu::GpioPin& rxPin, mcu::GpioPin& clkPin);

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

private:
	void _runTx();
	void _runRx();
};







