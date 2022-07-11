/**
 * @defgroup mco_tpdo_service TPDO Service
 * @ingroup microcanopen
 *
 * @file
 * @ingroup microcanopen mco_tpdo_service
 */


#pragma once


#include "driverlib.h"
#include "device.h"

#include "../mcodef.h"
#include "mcu/can/mcucan.h"

// APP-SPECIFIC headers
#include "syslog/syslog.h"
#include "boostconverter/boostconverter.h"


namespace microcanopen {
/// @addtogroup mco_tpdo_service
/// @{


/* ========================================================================== */
/* ======================= APPLICATION-SPECIFIC BEGIN ======================= */
/* ========================================================================== */

/**
 * @ingroup mco_app_spec
 * @brief TPDO1 message data.
 */
struct CobTpdo1
{
	union
	{
		struct
		{
			uint16_t statusRun : 1;			// bit 0
			uint16_t statusFault : 1;		// bit 1
			uint16_t statusWarning : 1;		// bit 2
			uint16_t statusOverheat : 1;		// bit 3
			uint16_t referenceType : 1;		// bit 4
			uint16_t controlLoopType : 1;		// bit 5
			uint16_t statusReserved : 2;		// bit 6..7
			uint8_t driveState : 8;
			int16_t speed : 16;
			int8_t currentS : 8;
			int8_t power : 8;
			uint8_t voltageDC : 8;
			uint8_t currentF : 8;
		} can1;
		struct
		{
			uint64_t reserved;
		} can2;
	};
	CobTpdo1()
	{
		uint64_t rawMsg = 0;
		memcpy(this, &rawMsg, sizeof(CobTpdo1));
	}
	CobTpdo1(uint64_t rawMsg) { memcpy(this, &rawMsg, sizeof(CobTpdo1)); }
	uint64_t all() const
	{
		uint64_t data = 0;
		memcpy(&data, this, sizeof(CobTpdo1));
		return data;
	}
};


/**
 * @ingroup mco_app_spec
 * @brief TPDO2 message data.
 */
struct CobTpdo2
{
	union
	{
		struct
		{
			uint8_t tempMotorS : 8;
			uint8_t reserved1 : 8;
			uint8_t voltageOut : 8;
			uint8_t reserved2 : 7;
			uint16_t torqueLimitation : 1;
			uint8_t tempMotorFw : 8;
			uint8_t tempHeatsink : 8;
			uint8_t tepmCaseAir : 8;
			uint8_t reserved3 : 8;
		} can1;
		struct
		{
			uint64_t reserved;
		} can2;
	};
	CobTpdo2()
	{
		uint64_t rawMsg = 0;
		memcpy(this, &rawMsg, sizeof(CobTpdo2));
	}
	CobTpdo2(uint64_t rawMsg) { memcpy(this, &rawMsg, sizeof(CobTpdo2)); }
	uint64_t all() const
	{
		uint64_t data = 0;
		memcpy(&data, this, sizeof(CobTpdo2));
		return data;
	}
};


/**
 * @ingroup mco_app_spec
 * @brief TPDO3 message data.
 */
struct CobTpdo3
{
	union
	{
		struct
		{
			uint8_t voltagePosHousing : 8;
			uint8_t voltageNegHousing : 8;
			uint16_t statusReserved1 : 1;			// bit 0
			uint16_t statusInsulationLow : 1;		// bit 1
			uint16_t statusInsulationLowWoFilter : 1;	// bit 2
			uint16_t statusReserved2 : 1;			// bit 3
			uint16_t reserved1 : 4;
			int8_t currentDC : 8;
			uint32_t syslogInfo : 32;			// by protocol: uint32_t reserved2 : 32;
		} can1;
		struct
		{
			uint64_t reserved;
		} can2;
	};
	CobTpdo3()
	{
		uint64_t rawMsg = 0;
		memcpy(this, &rawMsg, sizeof(CobTpdo3));
	}
	CobTpdo3(uint64_t rawMsg) { memcpy(this, &rawMsg, sizeof(CobTpdo3)); }
	uint64_t all() const
	{
		uint64_t data = 0;
		memcpy(&data, this, sizeof(CobTpdo3));
		return data;
	}
};


/**
 * @ingroup mco_app_spec
 * @brief TPDO4 message data.
 */
struct CobTpdo4
{
	union
	{
		struct
		{
			uint32_t faults : 32;
			uint32_t warnings : 32;
		} can1;
		struct
		{
			uint64_t reserved;
		} can2;
	};
	CobTpdo4()
	{
		uint64_t rawMsg = 0;
		memcpy(this, &rawMsg, sizeof(CobTpdo4));
	}
	CobTpdo4(uint64_t rawMsg) { memcpy(this, &rawMsg, sizeof(CobTpdo4)); }
	uint64_t all() const
	{
		uint64_t data = 0;
		memcpy(&data, this, sizeof(CobTpdo4));
		return data;
	}
};

/* ========================================================================== */
/* ======================== APPLICATION-SPECIFIC END ======================== */
/* ========================================================================== */


// APP-SPECIFIC defines
#if (defined(ACIM_MOTOR_SIX_PHASE))
#define TPDO_DRIVE(module, ipc, mode) TpdoService<module, ipc, mode>::drive6Ph
#elif (defined(ACIM_MOTOR_THREE_PHASE))
#define TPDO_DRIVE(module, ipc, mode) TpdoService<module, ipc, mode>::drive3Ph_1
#elif (defined(ACIM_TWO_MOTORS))
#define TPDO_DRIVE(module, ipc, mode) TpdoService<module, ipc, mode>::drive3Ph_1
#endif


/**
 * @brief TPDO-service class.
 */
template <mcu::CanModule Module, mcu::IpcMode Ipc, emb::MasterSlaveMode Mode>
class TpdoService
{
	friend class TpdoServiceTest;
private:
	// APP-SPECIFIC objects
	const BoostConverter* converter;

private:
	TpdoService(const TpdoService& other);			// no copy constructor
	TpdoService& operator=(const TpdoService& other);	// no copy assignment operator
public:
	/**
	 * @ingroup mco_app_spec
	 * @brief Configures service.
	 */
	TpdoService(BoostConverter* _converter)
	{
		EMB_STATIC_ASSERT(Mode == emb::MODE_MASTER);
		// APP-SPECIFIC BEGIN
		converter = _converter;
		// APP-SPECIFIC END
	}

	/**
	 * @ingroup mco_app_spec
	 * @brief Creates TPDO1 message data. Used by McoServer as callback.
	 * @param (none)
	 * @return TPDO1 message raw data.
	 */
	uint64_t makeTpdo1()
	{
		CobTpdo1 msg;
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case MCO_CAN1:
			//saveDriveState(msg, TPDO_DRIVE(Module, Ipc, Mode)->state());
			saveRunStatus(msg, converter->state());
			saveFaultStatus(msg, Syslog::faults());
			saveWarningStatus(msg, Syslog::warnings());
			// TODO tpdoService.saveOverheatStatus(msg,
			//saveReferenceType(msg, TPDO_DRIVE(Module, Ipc, Mode)->model.reference());
			//saveControlLoopType(msg, TPDO_DRIVE(Module, Ipc, Mode)->model.controlLoopType());
			/* TODO
			tpdoService.saveTorquePU(msg, m_drive->model.torque() * m_drive->positionSensor.signOfPositiveDirection() / m_drive->model.torqueMax());
			tpdoService.saveSpeed(msg, m_drive->positionSensor.speed(srm::PositionSensor::CAPTURE_BASIS).rpm() * m_drive->positionSensor.signOfPositiveDirection());
			tpdoService.saveStatorCurrent(msg, m_drive->model.currentPhaseRms());
			tpdoService.savePower(msg, m_drive->powerMech());
			tpdoService.saveDcVoltage(msg, m_drive->converter.voltageDC());
			tpdoService.saveFieldCurrent(msg, m_drive->converter.currents()[srm::CURRENT_FIELD]);
			tpdoService.saveRunStatus(msg, m_drive->converter.state());
			tpdoService.saveFaultStatus(msg, syslog.faults());
			tpdoService.saveWarningStatus(msg, syslog.warnings());
			tpdoService.saveOverheatStatus(msg,
					syslog.hasWarning(Warning::JUNCTION_OVERHEATING)
					|| syslog.hasWarning(Warning::CASE_AIR_OVERHEATING)
					|| syslog.hasWarning(Warning::STATOR_OVERHEATING)
					|| syslog.hasWarning(Warning::FIELD_WINDING_OVERHEATING));
			*/
			break;
		case MCO_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		return msg.all();
	}

	/**
	 * @ingroup mco_app_spec
	 * @brief Creates TPDO2 message data. Used by McoServer as callback.
	 * @param (none)
	 * @return TPDO2 message raw data.
	 */
	uint64_t makeTpdo2()
	{
		CobTpdo2 msg;
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case MCO_CAN1:
			/* TODO
			tpdoService.saveMotorTemp(msg, m_drive->motor.temperature(srm::Motor::STATOR));
			tpdoService.saveOutputVoltagePU(msg, m_drive->model.pwmDutyCycle());
			tpdoService.saveTorqueLimitationStatus(msg, 0);	// TODO
			tpdoService.saveFwMotorTemp(msg, m_drive->motor.temperature(srm::Motor::FIELD_WINDING));
			#ifdef CRD300
			tpdoService.saveHeatsinkTemp(msg, m_drive->converter.temperature(srm::Converter::MODULE_PHASE_B));
			#else
			tpdoService.saveHeatsinkTemp(msg, m_drive->converter.temperature(srm::Converter::AIR));
			#endif
			tpdoService.saveCaseAirTemp(msg, m_drive->converter.temperature(srm::Converter::AIR));
			*/
			break;
		case MCO_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		return msg.all();
	}

	/**
	 * @ingroup mco_app_spec
	 * @brief Creates TPDO3 message data. Used by McoServer as callback.
	 * @param (none)
	 * @return TPDO3 message raw data.
	 */
	uint64_t makeTpdo3()
	{
		CobTpdo3 msg;
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case MCO_CAN1:
			msg.can1.syslogInfo = static_cast<uint32_t>(Syslog::readMessage());
			Syslog::popMessage();
			break;
		case MCO_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		return msg.all();
	}

	/**
	 * @ingroup mco_app_spec
	 * @brief Creates TPDO4 message data. Used by McoServer as callback.
	 * @param (none)
	 * @return TPDO4 message raw data.
	 */
	uint64_t makeTpdo4()
	{
		CobTpdo4 msg;
		// APP-SPECIFIC BEGIN
		switch (Module)
		{
		case MCO_CAN1:
			saveFaults(msg, Syslog::faults());
			saveWarnings(msg, Syslog::warnings());
			break;
		case MCO_CAN2:
			// RESERVED;
			break;
		}
		// APP-SPECIFIC END
		return msg.all();
	}

/* ========================================================================== */
/* ======================= APPLICATION-SPECIFIC BEGIN ======================= */
/* ========================================================================== */
private:
	static const float SPEED_RPM_LIMIT = 8000;
	static const float STATOR_CURRENT_LIMIT = 650;
	static const float POWER_LIMIT = 150000;
	static const float DC_VOLTAGE_LIMIT = 1620;
	static const float FIELD_CURRENT_LIMIT = 35;

	static const float TEMP_OFFSET = 40;

	static const float HOUSING_VOLTAGE_LIMIT = 1620;
	static const float DC_CURRENT_LIMIT = 200;

/* ========================================================================== */
/* =========================== CobTpdo1-message ============================= */
/* ========================================================================== */
	/**
	 * @brief
	 * @param msg
	 * @param state
	 */
	/*static void saveDriveState(CobTpdo1& msg, acim::DriveState state)
	{
		msg.can1.driveState = static_cast<uint8_t>(state);
	}*/

	/**
	 *
	 * @param msg
	 * @param speedRpm
	 */
	static void saveSpeed(CobTpdo1& msg, float speedRpm)
	{
		if (speedRpm >= 0)
		{
			msg.can1.speed = ((speedRpm > SPEED_RPM_LIMIT) ?
					32767 : (speedRpm / SPEED_RPM_LIMIT) * 32767);
		}
		else
		{
			msg.can1.speed = ((speedRpm < -SPEED_RPM_LIMIT) ?
					-32768 : (speedRpm / SPEED_RPM_LIMIT) * 32768);
		}
	}

	/**
	 *
	 * @param msg
	 * @param current
	 */
	static void saveStatorCurrent(CobTpdo1& msg, float current)
	{
		if (current >= 0)
		{
			msg.can1.currentS = ((current > STATOR_CURRENT_LIMIT) ?
					127 : (current / STATOR_CURRENT_LIMIT) * 127);
		}
		else
		{
			msg.can1.currentS = ((current < -STATOR_CURRENT_LIMIT) ?
					-128 : (current / STATOR_CURRENT_LIMIT) * 128);
		}
	}

	/**
	 *
	 * @param msg
	 * @param power
	 */
	static void savePower(CobTpdo1& msg, float power)
	{
		{
			if (power >= 0)
			{
				msg.can1.power = ((power > POWER_LIMIT) ?
					127 : (power / POWER_LIMIT) * 127);
			}
			else
			{
				msg.can1.power = ((power < -POWER_LIMIT) ?
						-128 : (power / POWER_LIMIT) * 128);
			}
		}
	}

	/**
	 *
	 * @param msg
	 * @param voltage
	 */
	static void saveDcVoltage(CobTpdo1& msg, float voltage)
	{
		if (voltage < 0)
		{
			msg.can1.voltageDC = 0;
		}
		else
		{
			msg.can1.voltageDC = ((voltage > DC_VOLTAGE_LIMIT) ?
					255 : (voltage / DC_VOLTAGE_LIMIT) * 255);
		}
	}

	/**
	 *
	 * @param msg
	 * @param current
	 */
	static void saveFieldCurrent(CobTpdo1& msg, float current)
	{
		if (current < 0)
		{
			msg.can1.currentF = 0;
		}
		else
		{
			msg.can1.currentF = ((current > FIELD_CURRENT_LIMIT) ?
					255 : (current / FIELD_CURRENT_LIMIT) * 255);
		}
	}

	/**
	 *
	 * @param msg
	 * @param run
	 */
	static void saveRunStatus(CobTpdo1& msg, uint32_t run) { msg.can1.statusRun = ((run == 0) ? 0 : 1); }

	/**
	 *
	 * @param msg
	 * @param fault
	 */
	static void saveFaultStatus(CobTpdo1& msg, uint32_t fault) { msg.can1.statusFault = ((fault == 0) ? 0 : 1); }

	/**
	 *
	 * @param msg
	 * @param warning
	 */
	static void saveWarningStatus(CobTpdo1& msg, uint32_t warning) { msg.can1.statusWarning = ((warning == 0) ? 0 : 1); }

	/**
	 *
	 * @param msg
	 * @param overheat
	 */
	static void saveOverheatStatus(CobTpdo1& msg, uint32_t overheat) { msg.can1.statusOverheat = ((overheat == 0) ? 0 : 1); }

	/**
	 *
	 * @param msg
	 * @param ref
	 */
	/*static void saveReferenceType(CobTpdo1& msg, acim::Reference ref)
	{
		switch (ref)
		{
		case acim::SPEED_REFERENCE:
			msg.can1.referenceType = 0;
			break;
		case acim::TORQUE_REFERENCE:
			msg.can1.referenceType = 1;
			break;
		}
	}*/

	/**
	 *
	 * @param msg
	 * @param ref
	 */
	/*static void saveControlLoopType(CobTpdo1& msg, acim::ControlLoopType loopType)
	{
		switch (loopType)
		{
		case acim::OPEN_LOOP:
			msg.can1.controlLoopType = 0;
			break;
		case acim::CLOSED_LOOP:
			msg.can1.controlLoopType = 1;
			break;
		}
	}*/

/* ========================================================================== */
/* =========================== CobTpdo2-message ============================= */
/* ========================================================================== */
	/**
	 *
	 * @param msg
	 * @param temp
	 */
	void saveMotorTemp(CobTpdo2& msg, float temp) const
	{
		if (temp < -TEMP_OFFSET)
		{
			msg.can1.tempMotorS = 0;
		}
		else
		{
			msg.can1.tempMotorS = (((temp + TEMP_OFFSET) > 255) ? 255 : temp + TEMP_OFFSET);
		}
	}

	/**
	 *
	 * @param msg
	 * @param voltagePU
	 */
	void saveOutputVoltagePU(CobTpdo2& msg, float voltagePU) const
	{
		if (voltagePU > 1)
		{
			msg.can1.voltageOut = 255;
		}
		else if (voltagePU < 0)
		{
			msg.can1.voltageOut = 0;
		}
		else
		{
			msg.can1.voltageOut = voltagePU * 255;
		}
	}

	/**
	 *
	 * @param msg
	 * @param status
	 */
	void saveTorqueLimitationStatus(CobTpdo2& msg, uint32_t status) const
	{
		msg.can1.torqueLimitation = ((status == 0) ? 0 : 1);
	}

	/**
	 *
	 * @param msg
	 * @param temp
	 */
	void saveFwMotorTemp(CobTpdo2& msg, float temp) const
	{
		if (temp < -TEMP_OFFSET)
		{
			msg.can1.tempMotorFw = 0;
		}
		else
		{
			msg.can1.tempMotorFw = (((temp + TEMP_OFFSET) > 255) ? 255 : temp + TEMP_OFFSET);
		}
	}

	/**
	 *
	 * @param msg
	 * @param temp
	 */
	void saveHeatsinkTemp(CobTpdo2& msg, float temp) const
	{
		if (temp < -TEMP_OFFSET)
		{
			msg.can1.tempHeatsink = 0;
		}
		else
		{
			msg.can1.tempHeatsink = (((temp + TEMP_OFFSET) > 255) ? 255 : temp + TEMP_OFFSET);
		}
	}

	/**
	 *
	 * @param msg
	 * @param temp
	 */
	void saveCaseAirTemp(CobTpdo2& msg, float temp) const
	{
		if (temp < -TEMP_OFFSET)
		{
			msg.can1.tepmCaseAir = 0;
		}
		else
		{
			msg.can1.tepmCaseAir = (((temp + TEMP_OFFSET) > 255) ? 255 : temp + TEMP_OFFSET);
		}
	}

/* ========================================================================== */
/* =========================== CobTpdo3-message ============================= */
/* ========================================================================== */
	/**
	 *
	 * @param msg
	 * @param voltage
	 */
	void savePosHousingVoltage(CobTpdo3& msg, float voltage) const
	{
		if (voltage < 0)
		{
			msg.can1.voltagePosHousing = 0;
		}
		else
		{
			msg.can1.voltagePosHousing = ((voltage > HOUSING_VOLTAGE_LIMIT) ?
					255 : (voltage / HOUSING_VOLTAGE_LIMIT) * 255);
		}
	}

	/**
	 *
	 * @param msg
	 * @param voltage
	 */
	void saveNegHousingVoltage(CobTpdo3& msg, float voltage) const
	{
		if (voltage < 0)
		{
			msg.can1.voltageNegHousing = 0;
		}
		else
		{
			msg.can1.voltageNegHousing = ((voltage > HOUSING_VOLTAGE_LIMIT) ?
					255 : (voltage / HOUSING_VOLTAGE_LIMIT) * 255);
		}
	}

	/**
	 *
	 * @param msg
	 * @param status
	 */
	void saveInsulationLowStatus(CobTpdo3& msg, uint32_t status) const
	{
		msg.can1.statusInsulationLow = ((status == 0) ? 0 : 1);
	}

	/**
	 *
	 * @param msg
	 * @param status
	 */
	void saveInsulationLowStatusWoFilter(CobTpdo3& msg, uint32_t status) const
	{
		msg.can1.statusInsulationLowWoFilter = ((status == 0) ? 0 : 1);
	}

	/**
	 *
	 * @param msg
	 * @param current
	 */
	void saveDcCurrent(CobTpdo3& msg, float current) const
	{
		if (current >= 0)
		{
			msg.can1.currentDC = ((current > DC_CURRENT_LIMIT) ?
					127 : (current / DC_CURRENT_LIMIT) * 127);
		}
		else
		{
			msg.can1.currentDC = ((current < -DC_CURRENT_LIMIT) ?
					-128 : (current / DC_CURRENT_LIMIT) * 128);
		}
	}

/* ========================================================================== */
/* =========================== CobTpdo4-message ================================ */
/* ========================================================================== */
	/**
	 *
	 * @param msg
	 * @param faults
	 */
	static void saveFaults(CobTpdo4& msg, uint32_t faults) { msg.can1.faults = faults; }

	/**
	 *
	 * @param msg
	 * @param warnings
	 */
	static void saveWarnings(CobTpdo4& msg, uint32_t warnings) { msg.can1.warnings = warnings; }
/* ========================================================================== */
/* ======================== APPLICATION-SPECIFIC END ======================== */
/* ========================================================================== */
};


/// @}
} // namespace microcanopen


