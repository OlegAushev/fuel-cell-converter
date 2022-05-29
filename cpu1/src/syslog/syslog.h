/**
 * @defgroup syslog Syslog
 *
 * @file
 * @ingroup syslog
 */


#pragma once


#include <stdint.h>
#include <stddef.h>
#include "emb/emb_common.h"
#include "emb/emb_queue.h"
#include "mcu/system/mcusystem.h"
#include "mcu/ipc/mcuipc.h"


/// @addtogroup syslog
/// @{


namespace Fault {
/// System faults
enum Fault
{
	DC_UNDERVOLTAGE,
	DC_OVERVOLTAGE,
	PHASE_OVERCURRENT,
	DRIVER_ALL_FLT,
	DRIVER_UVW_FLT,
	DRIVER_XYZ_FLT,
	DRIVER_U_FLT,
	DRIVER_V_FLT,
	DRIVER_W_FLT,
	DRIVER_X_FLT,
	DRIVER_Y_FLT,
	DRIVER_Z_FLT,
	MODULE_OVERTEMP,
	CASE_OVERTEMP,
	CONNECTION_LOST,
	CAN_BUS_ERROR,
	RUNTIME_ERROR,
	EEPROM_ERROR,
	CURRENT_SENSOR_FAULT,
	EMERGENCY_STOP,
};

const uint32_t CRITICAL_FAULTS = (1UL << DRIVER_ALL_FLT)
				| (1UL << DRIVER_UVW_FLT)
				| (1UL << DRIVER_XYZ_FLT)
				| (1UL << DRIVER_U_FLT)
				| (1UL << DRIVER_V_FLT)
				| (1UL << DRIVER_W_FLT)
				| (1UL << DRIVER_X_FLT)
				| (1UL << DRIVER_Y_FLT)
				| (1UL << DRIVER_Z_FLT)
				| (1UL << EEPROM_ERROR)
				| (1UL << CURRENT_SENSOR_FAULT);
}  // namespace Fault


namespace Warning {
/// System warnings
enum Warning
{
	CAN_BUS_WARNING,
	MODULE_OVERHEATING,
	CASE_OVERHEATING,
	FLUX_WEAKENING
};

const uint32_t CRITICAL_WARNINGS = 0;
}


namespace SyslogMsg {
/// System messages
enum SyslogMsg
{
	NO_MESSAGE,
	DEVICE_BOOT_SUCCESS,
	DEVICE_BUSY,
	DEVICE_SW_RESET,
	CONFIGS_READ_SUCCESS,
	CONFIGS_READ_FAIL,
	CONFIGS_RESET_SUCCESS,
	CONFIGS_RESET_FAIL,
	CONFIGS_APPLY_SUCCESS,
	CONFIGS_APPLY_FAIL,
	SDO_REQUEST_LOST,
};
} // namespace SyslogMsg


/**
 * @brief System logger class.
 */
class Syslog : public emb::Monostate<Syslog>
{
private:
	Syslog();				// no constructor
	Syslog(const Syslog& other);		// no copy constructor
	Syslog& operator=(const Syslog& other);	// no copy assignment operator
private:
	static emb::Queue<SyslogMsg::SyslogMsg, 32> m_messages;
#ifdef DUALCORE
	static SyslogMsg::SyslogMsg  m_cpu2Message;
#endif

	struct FaultData
	{
		uint32_t faults;
		uint32_t warnings;
		uint32_t enabledFaultMask;	// enabled faults
		uint32_t criticalFaultMask;	// faults that cannot be reseted by reset()
		uint32_t criticalWarningMask;	// warnings that cannot be reseted by reset()
	};

	static FaultData m_cpu1FaultData;
#ifdef DUALCORE
	static FaultData m_cpu2FaultData;
#endif

	static FaultData* m_thisCpuFaultData;

public:
	static const char* DEVICE_NAME;
	static const uint32_t SOFTWARE_VERSION;
	static const char* BUILD_CONFIGURATION;

	/**
	 * @brief Initializes Syslog.
	 * @param (none)
	 * @return (none)
	 */
	static void init()
	{
		if (initialized())
		{
			return;
		}
		m_messages.clear();

#ifdef CPU1
		m_thisCpuFaultData = &m_cpu1FaultData;
#endif
#ifdef CPU2
		m_thisCpuFaultData = &m_cpu2FaultData;
#endif

		m_thisCpuFaultData->faults = 0;
		m_thisCpuFaultData->warnings = 0;
		m_thisCpuFaultData->enabledFaultMask = 0xFFFFFFFF;
		m_thisCpuFaultData->criticalFaultMask = Fault::CRITICAL_FAULTS;
		m_thisCpuFaultData->criticalWarningMask = Warning::CRITICAL_WARNINGS;

		setInitialized();
	}

	/**
	 * @brief Adds message to message queue or generates IPC signal (CPU2).
	 * @param msg - message to be added
	 * @return (none)
	 */
	static void addMessage(SyslogMsg::SyslogMsg msg)
	{
		mcu::CRITICAL_SECTION;
#ifdef CPU1
		if (!m_messages.full())
		{
			m_messages.push(msg);
		}
#else
		m_cpu2Message = msg;
		mcu::sendIpcSignal(SYSLOG_ADD_MESSAGE);
#endif
	}

	/**
	 * @brief Reads front message from message queue.
	 * @param (none)
	 * @return Front message from Syslog message queue.
	 */
	static SyslogMsg::SyslogMsg readMessage()
	{
		if (m_messages.empty())
		{
			return SyslogMsg::NO_MESSAGE;
		}
		return m_messages.front();
	}

	/**
	 * @brief Pops message from message queue (CPU1) or generates IPC signal (CPU2).
	 * @param (none)
	 * @return (none)
	 */
	static void popMessage()
	{
		mcu::CRITICAL_SECTION;
#ifdef CPU1
		if (!m_messages.empty())
		{
			m_messages.pop();
		}
#else
		mcu::sendIpcSignal(SYSLOG_POP_MESSAGE);
#endif
	}

	/**
	 * @brief Clears message queue.
	 * @param (none)
	 * @return (none)
	 */
	static void clearMessages()
	{
		mcu::CRITICAL_SECTION;
		m_messages.clear();
	}

#if (defined(DUALCORE) && defined(CPU1))
	/**
	 * @brief Checks and processes Syslog IPC signals.
	 * @param (none)
	 * @return (none)
	 */
	static void processIpcSignals()
	{
		if (mcu::isIpcSignalSet(SYSLOG_POP_MESSAGE))
		{
			popMessage();
			mcu::acknowledgeIpcSignal(SYSLOG_POP_MESSAGE);
		}

		if (mcu::isIpcSignalSet(SYSLOG_ADD_MESSAGE))
		{
			addMessage(m_cpu2Message);
			mcu::acknowledgeIpcSignal(SYSLOG_ADD_MESSAGE);
		}
	}
#endif


	/**
	 * @brief Enables specified fault.
	 * @param fault - fault to be enabled
	 * @return (none)
	 */
	static void enableFault(Fault::Fault fault)
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuFaultData->enabledFaultMask = m_thisCpuFaultData->enabledFaultMask | (1UL << fault);
	}

	/**
	 * @brief Enables all faults.
	 * @param (none)
	 * @return (none)
	 */
	static void enableAllFaults()
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuFaultData->enabledFaultMask = 0xFFFFFFFF;
	}

	/**
	 * @brief Disables specified fault.
	 * @param fault - fault to be disabled
	 * @return (none)
	 */
	static void disableFault(Fault::Fault fault)
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuFaultData->enabledFaultMask = m_thisCpuFaultData->enabledFaultMask & ((1UL << fault) ^ 0xFFFFFFFF);
	}

	/**
	 * @brief Disables all faults.
	 * @param (none)
	 * @return (none)
	 */
	static void disableAllFaults()
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuFaultData->enabledFaultMask = 0;
	}

	/**
	 * @brief Sets specified fault in FaultData structure.
	 * @param fault  - fault to be set
	 * @return (none)
	 */
	static void setFault(Fault::Fault fault)
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuFaultData->faults = m_thisCpuFaultData->faults | ((1UL << fault) & m_thisCpuFaultData->enabledFaultMask);
	}

	/**
	 * @brief Checks specified fault in FaultData structure.
	 * @param fault - warning to be checked
	 * @return \c true if fault is set, \c false otherwise.
	 */
	static bool hasFault(Fault::Fault fault)
	{
#ifdef DUALCORE
		return (m_cpu1FaultData.faults | m_cpu2FaultData.faults) & (1UL << fault);
#else
		return m_thisCpuFaultData->faults & (1UL << fault);
#endif
	}

	/**
	 * @brief Resets specified fault in FaultData structure.
	 * @param fault - fault to be reset
	 * @return (none)
	 */
	static void resetFault(Fault::Fault fault)
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuFaultData->faults = m_thisCpuFaultData->faults & ((1UL << fault) ^ 0xFFFFFFFF);
	}

	/**
	 * @brief Returns current system fault code.
	 * @param (none)
	 * @return Current system fault code.
	 */
	static uint32_t faults()
	{
#ifdef DUALCORE
		return m_cpu1FaultData.faults | m_cpu2FaultData.faults;
#else
		return m_thisCpuFaultData->faults;
#endif
	}

	/**
	 * @brief Checks if system has critical faults.
	 * @param (none)
	 * @return True if system has critical faults.
	 */
	static bool criticalFaultDetected()
	{
#ifdef DUALCORE
		return (m_cpu1FaultData.faults & m_cpu1FaultData.criticalFaultMask) || (m_cpu2FaultData.faults & m_cpu2FaultData.criticalFaultMask);
#else
		return m_thisCpuFaultData->faults & m_thisCpuFaultData->criticalFaultMask;
#endif
	}

	/**
	 * @brief Sets specified warning in FaultData structure.
	 * @param warning - warning to be set
	 * @return (none)
	 */
	static void setWarning(Warning::Warning warning)
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuFaultData->warnings = m_thisCpuFaultData->warnings | (1UL << warning);
	}

	/**
	 * @brief Checks specified warning in FaultData structure.
	 * @param warning - warning to be checked
	 * @return \c true if warning is set, \c false otherwise.
	 */
	static bool hasWarning(Warning::Warning warning)
	{
#ifdef DUALCORE
		return (m_cpu1FaultData.warnings | m_cpu2FaultData.warnings) & (1UL << warning);
#else
		return m_thisCpuFaultData->warnings & (1UL << warning);
#endif
	}

	/**
	 * @brief Resets specified warning in FaultData structure.
	 * @param warning - warning to be reset
	 * @return (none)
	 */
	static void resetWarning(Warning::Warning warning)
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuFaultData->warnings = m_thisCpuFaultData->warnings & ((1UL << warning) ^ 0xFFFFFFFF);
	}

	/**
	 * @brief Returns current system warning code.
	 * @param (none)
	 * @return Current system warning code.
	 */
	static uint32_t warnings()
	{
#ifdef DUALCORE
		return m_cpu1FaultData.warnings | m_cpu2FaultData.warnings;
#else
		return m_thisCpuFaultData->warnings;
#endif
	}

	/**
	 * @brief Resets non-critical faults and warnings.
	 * @param tag - soft reset tag
	 * @return (none)
	 */
	static void resetFaultsAndWarnings()
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuFaultData->faults = m_thisCpuFaultData->faults & m_thisCpuFaultData->criticalFaultMask;
		m_thisCpuFaultData->warnings = m_thisCpuFaultData->warnings & m_thisCpuFaultData->criticalWarningMask;
#if (defined(CPU1) && defined(DUALCORE))
		mcu::sendIpcSignal(SYSLOG_RESET);
#endif
	}

	/**
	 * @brief Disables critical faults and  warnings masks.
	 * @param (none)
	 * @return (none)
	 */
	static void clearCriticalMasks()
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuFaultData->criticalFaultMask = 0;
		m_thisCpuFaultData->criticalWarningMask = 0;
	}

	/**
	 * @brief Enables critical faults and  warnings masks.
	 * @param (none)
	 * @return (none)
	 */
	static void enableCriticalMasks()
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuFaultData->criticalFaultMask = Fault::CRITICAL_FAULTS;
		m_thisCpuFaultData->criticalWarningMask = Warning::CRITICAL_WARNINGS;
	}

#ifdef CPU2
	/**
	 * @brief Reset faults and warnings IPC ISR for CPU2.
	 * @param (none)
	 * @return (none)
	 */
	static __interrupt void onFaultsAndWarningsReset()
	{
		Syslog::resetFaultsAndWarnings();
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
		mcu::acknowledgeIpcSignal(SYSLOG_RESET);
	}
#endif


};


/// @}





























