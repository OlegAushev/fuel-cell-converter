/**
 * @file
 * @ingroup fuel_cell_controller
 */


#include "fuelcell_controller.h"


namespace fuelcell {


const mcu::IpcFlag Controller::SIG_START(21);
const mcu::IpcFlag Controller::SIG_STOP(22);

uint64_t Controller::m_errorRegTimestamp = 0;
bool Controller::m_isErrorRegistered = false;


Data Controller::s_data __attribute__((section("SHARED_FUELCELL_DATA"), retain));


///
///
///
Controller::Controller(const Converter* converter,
		const mcu::GpioInput& rxPin, const mcu::GpioOutput& txPin, mcu::GpioOutput& clkPin)
	: m_converter(converter)
	, m_transceiver(rxPin, txPin, clkPin, 125000, canbygpio::tag::disable_bit_stuffing()) // TODO disable bit stuffing
{
	EMB_STATIC_ASSERT(sizeof(TpdoMessage) == 4);
	EMB_STATIC_ASSERT(sizeof(RpdoMessage) == 4);

	s_data.temperature.fill(0);

	for (size_t i = 0; i < FUELCELL_COUNT; ++i)
	{
		s_data.cellVoltage[i].setSmoothFactor(0.1);
		s_data.cellVoltage[i].reset();
	}

	s_data.battVoltage.fill(0);
	s_data.current.fill(0);

	s_data.statusError =  false;
	s_data.statusStart.fill(false);
	s_data.statusRun.fill(false);
	s_data.statusOverheat.fill(false);
	s_data.statusLowCharge.fill(false);
	s_data.statusNoConnection = false;
	s_data.statusLowPressure = false;
	s_data.statusHydroError = false;
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
		s_data.cellVoltage[cell].push(0.1f * rpdo.cellVoltage);
		s_data.battVoltage[cell] = 0.1f * rpdo.battVoltage;

		if (cell == 0)
		{
			s_data.statusError = rpdo.statusError;
			s_data.statusNoConnection = rpdo.statusNoConnection;
			s_data.statusLowPressure = rpdo.statusLowPressure;
			s_data.statusHydroError = rpdo.statusHydroError;
		}

		s_data.statusStart[cell] = rpdo.statusStart;
		s_data.statusRun[cell] = rpdo.statusRun;
		s_data.statusOverheat[cell] = rpdo.statusOverheat;
		s_data.statusLowCharge[cell] = rpdo.statusLowCharge;

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


///
///
///
bool Controller::checkErrors()
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

	if (minCellVoltage() < ABSOLUTE_MIN_VOLTAGE)
	{
		error = true;
		if (errorDelayExpired)
		{
			Syslog::setError(sys::Error::FUELCELL_UV);
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


} // namespace fuelcell


