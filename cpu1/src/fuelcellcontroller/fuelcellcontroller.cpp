/**
 * @file
 * @ingroup fuel_cell_controller
 */


#include "fuelcellcontroller.h"


namespace fuelcell {


const mcu::IpcFlag Controller::SIG_START(21);
const mcu::IpcFlag Controller::SIG_STOP(22);


Data Controller::s_data __attribute__((section("SHARED_FUELCELL_DATA"), retain));


///
///
///
Controller::Controller(const BoostConverter* converter,
		const mcu::GpioPin& txPin, const mcu::GpioPin& rxPin, mcu::GpioPin& clkPin)
	: m_converter(converter)
	, m_transceiver(txPin, rxPin, clkPin, 125000, canbygpio::tag::disable_bit_stuffing()) // TODO disable bit stuffing
{
	EMB_STATIC_ASSERT(sizeof(TpdoMessage) == 4);
	EMB_STATIC_ASSERT(sizeof(RpdoMessage) == 4);

	s_data.temperature.fill(0);
	s_data.cellVoltage.fill(0);
	s_data.battVoltage.fill(0);
	s_data.status.fill(FUELCELL_NA);
	s_data.temperature.fill(0);
}


///
///
///
void Controller::_runTx()
{
	static uint64_t timeTxPrev = 0;
	TpdoMessage tpdo;
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
		//tpdo.reserved = mcu::SystemClock::now();

		emb::c28x::to_bytes8<TpdoMessage>(tpdoBytes, tpdo);

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
void Controller::_runRx()
{
	static uint64_t frameCount = 0;
	static uint64_t errorCount = 0;
	unsigned int rpdoId;
	uint16_t rpdoBytes[8] = {0};
	RpdoMessage rpdo;

	if (m_transceiver.recv(rpdoId, rpdoBytes) == 8)
	{
		if ((rpdoId < 0x180) || (rpdoId > 0x184))
		{
			++errorCount;
			return;
		}

		++frameCount;
		emb::c28x::from_bytes8<RpdoMessage>(rpdo, rpdoBytes);
		size_t cell = rpdoId - 0x180;

		s_data.temperature[cell] = -40.0f + rpdo.temperature;
		s_data.cellVoltage[cell] = 0.1f * rpdo.cellVoltage;
		s_data.battVoltage[cell] = 15.0f + 0.1f * rpdo.battVoltage;

		if (emb::Range<unsigned int>(0,7).contains(rpdo.status))
		{
			s_data.status[cell] = static_cast<Status>(rpdo.status);
		}

		s_data.current[cell] = 0.1f * rpdo.current;
	}
}


} // namespace fuelcell


