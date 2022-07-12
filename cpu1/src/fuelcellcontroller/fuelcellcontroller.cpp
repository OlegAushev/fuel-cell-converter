/**
 * @file
 * @ingroup fuel_cell_controller
 */


#include "fuelcellcontroller.h"


const mcu::IpcFlagPair FuelCellController::SIG_START(21);
const mcu::IpcFlagPair FuelCellController::SIG_STOP(22);


///
///
///
FuelCellController::FuelCellController(const BoostConverter* converter,
		const mcu::GpioPin& txPin, const mcu::GpioPin& rxPin,
		mcu::GpioPin& clkPin, uint32_t bitrate)
	: m_converter(converter)
	, m_transceiver(txPin, rxPin, clkPin, bitrate, canbygpio::tag::disable_bit_stuffing())
{

}


///
///
///
void FuelCellController::run()
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


