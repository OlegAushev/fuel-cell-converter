/**
 * @file
 * @ingroup fuel_cell_controller
 */


#include "fuelcell_controller.h"


namespace fuelcell {


const mcu::IpcFlag Controller::SIG_START(21);
const mcu::IpcFlag Controller::SIG_STOP(22);


Data Controller::s_data __attribute__((section("SHARED_FUELCELL_DATA"), retain));


///
///
///
Controller::Controller(const Converter* converter,
		const mcu::Gpio& rxPin, const mcu::Gpio& txPin, mcu::Gpio& clkPin)
	: m_converter(converter)
	, m_transceiver(rxPin, txPin, clkPin, 125000, canbygpio::tag::disable_bit_stuffing()) // TODO disable bit stuffing
{
	EMB_STATIC_ASSERT(sizeof(TpdoMessage) == 4);
	EMB_STATIC_ASSERT(sizeof(RpdoMessage) == 4);

	s_data.temperature.fill(0);
	s_data.cellVoltage.fill(0);
	s_data.battVoltage.fill(0);
	s_data.current.fill(0);

	s_data.statusError.fill(false);
	s_data.statusReady.fill(false);
	s_data.statusInoperation.fill(false);
	s_data.statusOverheat.fill(false);
	s_data.statusLowvoltage.fill(false);
	s_data.statusNoconnection.fill(false);
	s_data.statusLowpressure.fill(false);
	s_data.statusHydroerr.fill(false);
}


///
///
///
void Controller::runTx()
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
void Controller::runRx()
{
	static uint64_t frameCount = 0;
	static uint64_t errInvalidId = 0;
	static uint64_t errSOF = 0;
	static uint64_t errRTR = 0;
	static uint64_t errIDE = 0;
	static uint64_t errR0 = 0;
	static uint64_t errCRC = 0;
	unsigned int rpdoId;
	uint16_t rpdoBytes[8] = {0};
	RpdoMessage rpdo;

	int recvRetval = m_transceiver.recv(rpdoId, rpdoBytes);
	switch (recvRetval)
	{
	case 8:
	{
		if ((rpdoId < 0x180) || (rpdoId > 0x184))
		{
			++errInvalidId;
			return;
		}

		++frameCount;
		emb::c28x::from_bytes8<RpdoMessage>(rpdo, rpdoBytes);
		size_t cell = rpdoId - 0x180;

		s_data.temperature[cell] = rpdo.temperature;
		s_data.cellVoltage[cell] = 0.1f * rpdo.cellVoltage;
		s_data.battVoltage[cell] = 0.1f * rpdo.battVoltage;

		s_data.statusError[cell] = rpdo.statusError;
		s_data.statusReady[cell] = rpdo.statusReady;
		s_data.statusInoperation[cell] = rpdo.statusInoperation;
		s_data.statusOverheat[cell] = rpdo.statusOverheat;
		s_data.statusLowvoltage[cell] = rpdo.statusLowvoltage;
		s_data.statusNoconnection[cell] = rpdo.statusNoconnection;
		s_data.statusLowpressure[cell] = rpdo.statusLowpressure;
		s_data.statusHydroerr[cell] = rpdo.statusHydroerr;

		s_data.current[cell] = 0.1f * float(rpdo.current);
		break;
	}
	case -1:
		++errSOF;
		break;
	case -2:
		++errRTR;
		break;
	case -3:
		++errIDE;
		break;
	case -4:
		++errR0;
		break;
	case -5:
		++errCRC;
		break;
	}
}


} // namespace fuelcell


