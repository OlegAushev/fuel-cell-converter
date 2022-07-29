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
#include "mcu/system/mcu_system.h"
#include "mcu/ipc/mcu_ipc.h"


/// @addtogroup syslog
/// @{


namespace Fault {


/// System faults
enum Fault
{
	OVP_IN,
	UVP_IN,
	OVP_OUT,
	OCP_IN,
	DRIVER_FLT,
	MODULE_OVERTEMP,
	HEATSINK_OVERTEMP,
	CONNECTION_LOST,
	CAN_BUS_ERROR,
	RUNTIME_ERROR,
	EMERGENCY_STOP,
	FUELCELL_START_FAILED,
	FUELCELL_OVERHEAT,
	FUELCELL_BATT_LOWCHARGE,
	FUELCELL_NOCONNECTION,
	FUELCELL_LOWPRESSURE,
};


const uint32_t CRITICAL_FAULTS = (1UL << DRIVER_FLT)
				| (1UL << RUNTIME_ERROR);


} // namespace Fault


namespace Warning {


/// System warnings
enum Warning
{
	BATTERY_CHARGED,
	CAN_BUS_WARNING,
	MODULE_OVERHEATING,
	CASE_OVERHEATING,
};


const uint32_t CRITICAL_WARNINGS = 0;


} // namespace Warning


/**
 * @brief System logger class.
 */
class Syslog : public emb::Monostate<Syslog>
{
public:
	/// System messages
	enum Message
	{
		NO_MESSAGE,
		DEVICE_BOOT_CPU1,
		DEVICE_CPU1_BOOT_SUCCESS,
		DEVICE_BOOT_CPU2,
		DEVICE_CPU2_BOOT_SUCCESS,
		DEVICE_CPU1_READY,
		DEVICE_CPU2_READY,
		DEVICE_READY,
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

	struct IpcFlags
	{
		mcu::IpcFlag RESET;
		mcu::IpcFlag ADD_MESSAGE;
		mcu::IpcFlag POP_MESSAGE;
	};

private:
	Syslog();				// no constructor
	Syslog(const Syslog& other);		// no copy constructor
	Syslog& operator=(const Syslog& other);	// no copy assignment operator

private:
	static emb::Queue<Syslog::Message, 32> m_messages;
#ifdef DUALCORE
	static Syslog::Message m_cpu2Message;
#endif

	struct Data
	{
		uint32_t faults;
		uint32_t warnings;
		uint32_t enabledFaultMask;	// enabled faults
		uint32_t criticalFaultMask;	// faults that cannot be reseted by reset()
		uint32_t criticalWarningMask;	// warnings that cannot be reseted by reset()
	};

	static Data m_cpu1Data;
#ifdef DUALCORE
	static Data m_cpu2Data;
#endif

	static Data* m_thisCpuData;

	// IPC flags
	static mcu::IpcFlag RESET_FAULTS_AND_WARNINGS;
	static mcu::IpcFlag ADD_MESSAGE;
	static mcu::IpcFlag POP_MESSAGE;

public:
	static const char* DEVICE_NAME;
	static const uint32_t FIRMWARE_VERSION;
	static const char* BUILD_CONFIGURATION;

	/**
	 * @brief Initializes Syslog.
	 * @param (none)
	 * @return (none)
	 */
	static void init(const IpcFlags& ipcFlags)
	{
		if (initialized())
		{
			return;
		}
		m_messages.clear();

#ifdef CPU1
		m_thisCpuData = &m_cpu1Data;
#endif
#ifdef CPU2
		m_thisCpuData = &m_cpu2Data;
#endif

		m_thisCpuData->faults = 0;
		m_thisCpuData->warnings = 0;
		m_thisCpuData->enabledFaultMask = 0xFFFFFFFF;
		m_thisCpuData->criticalFaultMask = Fault::CRITICAL_FAULTS;
		m_thisCpuData->criticalWarningMask = Warning::CRITICAL_WARNINGS;

		RESET_FAULTS_AND_WARNINGS = ipcFlags.RESET;
		ADD_MESSAGE = ipcFlags.ADD_MESSAGE;
		POP_MESSAGE = ipcFlags.POP_MESSAGE;

		setInitialized();
	}

	/**
	 * @brief Adds message to message queue or generates IPC signal (CPU2).
	 * @param msg - message to be added
	 * @return (none)
	 */
	static void addMessage(Syslog::Message msg)
	{
		mcu::CRITICAL_SECTION;
#ifdef CPU1
		if (!m_messages.full())
		{
			m_messages.push(msg);
		}
#else
		if (mcu::isLocalIpcFlagSet(ADD_MESSAGE.local))
		{
			return;
		}
		m_cpu2Message = msg;
		mcu::setLocalIpcFlag(ADD_MESSAGE.local);
#endif
	}

	/**
	 * @brief Reads front message from message queue.
	 * @param (none)
	 * @return Front message from Syslog message queue.
	 */
	static Syslog::Message readMessage()
	{
		if (m_messages.empty())
		{
			return Syslog::NO_MESSAGE;
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
		mcu::setLocalIpcFlag(POP_MESSAGE.local);
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

	/**
	 * @brief Checks and processes Syslog IPC signals.
	 * @param (none)
	 * @return (none)
	 */
	static void processIpcSignals()
	{
#ifdef DUALCORE
#ifdef CPU1
		if (mcu::isRemoteIpcFlagSet(POP_MESSAGE.remote))
		{
			popMessage();
			mcu::acknowledgeRemoteIpcFlag(POP_MESSAGE.remote);
		}

		if (mcu::isRemoteIpcFlagSet(ADD_MESSAGE.remote))
		{
			addMessage(m_cpu2Message);
			mcu::acknowledgeRemoteIpcFlag(ADD_MESSAGE.remote);
		}
#endif
#ifdef CPU2
		if (mcu::isRemoteIpcFlagSet(RESET_FAULTS_AND_WARNINGS.remote))
		{
			resetFaultsAndWarnings();
			mcu::acknowledgeRemoteIpcFlag(RESET_FAULTS_AND_WARNINGS.remote);
		}
#endif
#endif
	}

	/**
	 * @brief Enables specified fault.
	 * @param fault - fault to be enabled
	 * @return (none)
	 */
	static void enableFault(Fault::Fault fault)
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuData->enabledFaultMask = m_thisCpuData->enabledFaultMask | (1UL << fault);
	}

	/**
	 * @brief Enables all faults.
	 * @param (none)
	 * @return (none)
	 */
	static void enableAllFaults()
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuData->enabledFaultMask = 0xFFFFFFFF;
	}

	/**
	 * @brief Disables specified fault.
	 * @param fault - fault to be disabled
	 * @return (none)
	 */
	static void disableFault(Fault::Fault fault)
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuData->enabledFaultMask = m_thisCpuData->enabledFaultMask & ((1UL << fault) ^ 0xFFFFFFFF);
	}

	/**
	 * @brief Disables all faults.
	 * @param (none)
	 * @return (none)
	 */
	static void disableAllFaults()
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuData->enabledFaultMask = 0;
	}

	/**
	 * @brief Sets specified fault in FaultData structure.
	 * @param fault  - fault to be set
	 * @return (none)
	 */
	static void setFault(Fault::Fault fault)
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuData->faults = m_thisCpuData->faults | ((1UL << fault) & m_thisCpuData->enabledFaultMask);
	}

	/**
	 * @brief Checks specified fault in FaultData structure.
	 * @param fault - warning to be checked
	 * @return \c true if fault is set, \c false otherwise.
	 */
	static bool hasFault(Fault::Fault fault)
	{
#ifdef DUALCORE
		return (m_cpu1Data.faults | m_cpu2Data.faults) & (1UL << fault);
#else
		return m_thisCpuData->faults & (1UL << fault);
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
		m_thisCpuData->faults = m_thisCpuData->faults & ((1UL << fault) ^ 0xFFFFFFFF);
	}

	/**
	 * @brief Returns current system fault code.
	 * @param (none)
	 * @return Current system fault code.
	 */
	static uint32_t faults()
	{
#ifdef DUALCORE
		return m_cpu1Data.faults | m_cpu2Data.faults;
#else
		return m_thisCpuData->faults;
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
		return (m_cpu1Data.faults & m_cpu1Data.criticalFaultMask) || (m_cpu2Data.faults & m_cpu2Data.criticalFaultMask);
#else
		return m_thisCpuData->faults & m_thisCpuData->criticalFaultMask;
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
		m_thisCpuData->warnings = m_thisCpuData->warnings | (1UL << warning);
	}

	/**
	 * @brief Checks specified warning in FaultData structure.
	 * @param warning - warning to be checked
	 * @return \c true if warning is set, \c false otherwise.
	 */
	static bool hasWarning(Warning::Warning warning)
	{
#ifdef DUALCORE
		return (m_cpu1Data.warnings | m_cpu2Data.warnings) & (1UL << warning);
#else
		return m_thisCpuData->warnings & (1UL << warning);
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
		m_thisCpuData->warnings = m_thisCpuData->warnings & ((1UL << warning) ^ 0xFFFFFFFF);
	}

	/**
	 * @brief Returns current system warning code.
	 * @param (none)
	 * @return Current system warning code.
	 */
	static uint32_t warnings()
	{
#ifdef DUALCORE
		return m_cpu1Data.warnings | m_cpu2Data.warnings;
#else
		return m_thisCpuData->warnings;
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
		m_thisCpuData->faults = m_thisCpuData->faults & m_thisCpuData->criticalFaultMask;
		m_thisCpuData->warnings = m_thisCpuData->warnings & m_thisCpuData->criticalWarningMask;
#if (defined(CPU1) && defined(DUALCORE))
		mcu::setLocalIpcFlag(RESET_FAULTS_AND_WARNINGS.local);
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
		m_thisCpuData->criticalFaultMask = 0;
		m_thisCpuData->criticalWarningMask = 0;
	}

	/**
	 * @brief Enables critical faults and  warnings masks.
	 * @param (none)
	 * @return (none)
	 */
	static void enableCriticalMasks()
	{
		mcu::CRITICAL_SECTION;
		m_thisCpuData->criticalFaultMask = Fault::CRITICAL_FAULTS;
		m_thisCpuData->criticalWarningMask = Warning::CRITICAL_WARNINGS;
	}
};


/// @}


