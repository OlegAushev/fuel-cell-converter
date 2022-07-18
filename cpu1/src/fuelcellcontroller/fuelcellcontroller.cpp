/**
 * @file
 * @ingroup fuel_cell_controller
 */


#include "fuelcellcontroller.h"


const mcu::IpcFlag FuelCellController::SIG_START(21);
const mcu::IpcFlag FuelCellController::SIG_STOP(22);


///
///
///
FuelCellController::FuelCellController(const BoostConverter* converter,
		const mcu::GpioPin& txPin, const mcu::GpioPin& rxPin, mcu::GpioPin& clkPin)
	: m_converter(converter)
	, m_transceiver(txPin, rxPin, clkPin, 125000, canbygpio::tag::enable_bit_stuffing()) // TODO disable bit stuffing
{
	EMB_STATIC_ASSERT(sizeof(FuelCellTpdo) == 4);
	EMB_STATIC_ASSERT(sizeof(FuelCellRpdo) == 4);
}


///
///
///
void FuelCellController::_runTx()
{
	static uint64_t timeTxPrev = 0;
	FuelCellTpdo tpdo;
	uint16_t tpdoBytes[8];

	if (mcu::SystemClock::now() >= (timeTxPrev + TPDO_PERIOD))
	{
		if (mcu::isRemoteIpcFlagSet(SIG_STOP.remote))
		{
			tpdo.cmd = 0x69;
		}
		else if (mcu::isRemoteIpcFlagSet(SIG_START.remote))
		{
			tpdo.cmd = 0x96;
		}
		else
		{
			tpdo.cmd = 0;
		}

		tpdo.voltage = emb::clamp(m_converter->voltageOut(), 0.f, float(USHRT_MAX));
		tpdo.current = emb::clamp(m_converter->currentIn(), 0.f, float(USHRT_MAX));

		// TODO remove this later
		tpdo.reserved = mcu::SystemClock::now();

		emb::c28x::to_bytes8<FuelCellTpdo>(tpdoBytes, tpdo);

		if (m_transceiver.send(TPDO_FRAME_ID, tpdoBytes, 8) == 8)
		{
			// transmission begins successfully
			if (mcu::isRemoteIpcFlagSet(SIG_STOP.remote))
			{
				mcu::acknowledgeRemoteIpcFlag(SIG_STOP.remote);
				mcu::acknowledgeRemoteIpcFlag(SIG_START.remote);
			}
			else if (mcu::isRemoteIpcFlagSet(SIG_START.remote))
			{
				mcu::acknowledgeRemoteIpcFlag(SIG_START.remote);
			}

			timeTxPrev = mcu::SystemClock::now();
		}
	}
}


///
///
///
void FuelCellController::_runRx()
{
	unsigned int rpdoId;
	uint16_t rpdoBytes[8];
	FuelCellRpdo rpdo;

	if (m_transceiver.recv(rpdoId, rpdoBytes) == 8)
	{
		emb::c28x::from_bytes8(rpdo, rpdoBytes);

		switch(rpdoId)
		{
		case 0x180:

			break;
		case 0x181:

			break;
		case 0x182:

			break;
		case 0x183:

			break;
		case 0x184:

			break;
		}
	}
}


