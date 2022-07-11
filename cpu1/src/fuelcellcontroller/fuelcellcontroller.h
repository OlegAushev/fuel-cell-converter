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


struct FuelCellTpdo
{
	uint64_t cmd : 8;
	uint64_t voltage : 16;
	uint64_t current : 16;
	uint64_t reserved : 24;
};


class FuelCellController
{
private:
	const BoostConverter* m_converter;
	canbygpio::Transceiver m_transceiver;
	static const mcu::IpcSignalPair SIG_START;
	static const mcu::IpcSignalPair SIG_STOP;
	static const uint64_t TPDO_PERIOD = 200;

public:
	FuelCellController(const BoostConverter* converter,
			const mcu::GpioPin& txPin, const mcu::GpioPin& rxPin,
			mcu::GpioPin& clkPin, uint32_t bitrate);

	void run();

	static void start()
	{
		mcu::sendIpcSignal(SIG_START.local);
	}

	static void stop()
	{
		mcu::sendIpcSignal(SIG_STOP.local);
	}
};







