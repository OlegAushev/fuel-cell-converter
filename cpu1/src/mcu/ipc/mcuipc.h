/**
 * @defgroup mcu_ipc IPC
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_ipc
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "F2837xD_Ipc_drivers.h"


namespace mcu {
/// @addtogroup mcu_ipc
/// @{


struct LocalIpcSignal
{
	const uint32_t flag;
	explicit LocalIpcSignal(uint32_t flagNo)
		: flag(1UL << flagNo)
	{
		ASSERT(flagNo < 32);
	}
};

struct RemoteIpcSignal
{
	const uint32_t flag;
	explicit RemoteIpcSignal(uint32_t flagNo)
		: flag(1UL << flagNo)
	{
		ASSERT(flagNo < 32);
	}
};

/**
 * @brief Sends IPC signal by setting local IPC flag.
 * @param ipcFlag
 * @return (none)
 * \callgraph
 * \callergraph
 */
inline void sendIpcSignal(LocalIpcSignal ipcFlag)
{
	IPCLtoRFlagSet(ipcFlag.flag);
}

/**
 * @brief Waits for IPC signal in blocking-mode.
 * @param ipcFlag
 * @return (none)
 */
inline void waitForIpcSignal(RemoteIpcSignal ipcFlag)
{
	while(!IPCRtoLFlagBusy(ipcFlag.flag));
	IPCRtoLFlagAcknowledge(ipcFlag.flag);
}

#ifdef DUALCORE
/**
 * @brief Checks if remote IPC flag is set.
 * @param ipcFlag
 * @return (none)
 */
inline bool isIpcSignalSet(RemoteIpcSignal ipcFlag)
{
	if (IPCRtoLFlagBusy(ipcFlag.flag))
	{
		return true;
	}
	return false;
}
#endif

/**
 * @brief Checks if local IPC flag is set.
 * @param ipcFlag
 * @return (none)
 */
inline bool isIpcSignalSet(LocalIpcSignal ipcFlag)
{
	if (IPCLtoRFlagBusy(ipcFlag.flag))
	{
		return true;
	}
	return false;
}

#ifdef DUALCORE
/**
 * @brief Acknowledges remote IPC flag.
 * @param ipcFlag
 * @return (none)
 */
inline void acknowledgeIpcSignal(RemoteIpcSignal ipcFlag)
{
	IPCRtoLFlagAcknowledge(ipcFlag.flag);
}
#endif

/**
 * @brief Clears local IPC flag.
 * @param ipcFlag
 * @return (none)
 */
inline void acknowledgeIpcSignal(LocalIpcSignal ipcFlag)
{
	IPCLtoRFlagClear(ipcFlag.flag);
}


enum IpcInterrupt
{
	IPC_INTERRUPT_0 = INT_IPC_0,
	IPC_INTERRUPT_1 = INT_IPC_1,
	IPC_INTERRUPT_2 = INT_IPC_2,
	IPC_INTERRUPT_3 = INT_IPC_3,
};

/**
 * @brief Registers IPC ISR.
 * @param ipcInterrupt - IPC interrupt
 * @param func - pointer to ISR
 * @return (none)
 */
inline void registerIpcInterruptHandler(IpcInterrupt ipcInterrupt, void (*handler)(void))
{
	Interrupt_register(ipcInterrupt, handler);
	Interrupt_enable(ipcInterrupt);
}


/// @}
} // namespace mcu


/// @addtogroup mcu_ipc
/// @{
#if (defined(DUALCORE) && defined(CPU1))
extern const mcu::LocalIpcSignal CPU1_PERIPHERY_CONFIGURED;
extern const mcu::LocalIpcSignal CAN_TSDO_READY;
extern const mcu::LocalIpcSignal SYSLOG_RESET;

extern const mcu::RemoteIpcSignal CPU2_BOOTED;
extern const mcu::RemoteIpcSignal CPU2_PERIPHERY_CONFIGURED;
extern const mcu::RemoteIpcSignal REPORT_SENT;
extern const mcu::RemoteIpcSignal SETUPMSG_RECEIVED;
extern const mcu::RemoteIpcSignal SYSLOG_POP_MESSAGE;
extern const mcu::RemoteIpcSignal SYSLOG_ADD_MESSAGE;

extern const mcu::RemoteIpcSignal CAN_FRAME_RECEIVED;
extern const mcu::RemoteIpcSignal CAN_RPDO1_RECEIVED;
extern const mcu::RemoteIpcSignal CAN_RPDO2_RECEIVED;
extern const mcu::RemoteIpcSignal CAN_RPDO3_RECEIVED;
extern const mcu::RemoteIpcSignal CAN_RPDO4_RECEIVED;
extern const mcu::RemoteIpcSignal CAN_RSDO_RECEIVED;
#endif

#if (defined(DUALCORE) && defined(CPU2))
extern const mcu::RemoteIpcSignal CPU1_PERIPHERY_CONFIGURED;
extern const mcu::RemoteIpcSignal CAN_TSDO_READY;
extern const mcu::RemoteIpcSignal SYSLOG_RESET;

extern const mcu::LocalIpcSignal CPU2_BOOTED;
extern const mcu::LocalIpcSignal CPU2_PERIPHERY_CONFIGURED;
extern const mcu::LocalIpcSignal REPORT_SENT;
extern const mcu::LocalIpcSignal SETUPMSG_RECEIVED;
extern const mcu::LocalIpcSignal SYSLOG_POP_MESSAGE;
extern const mcu::LocalIpcSignal SYSLOG_ADD_MESSAGE;

extern const mcu::LocalIpcSignal CAN_FRAME_RECEIVED;
extern const mcu::LocalIpcSignal CAN_RPDO1_RECEIVED;
extern const mcu::LocalIpcSignal CAN_RPDO2_RECEIVED;
extern const mcu::LocalIpcSignal CAN_RPDO3_RECEIVED;
extern const mcu::LocalIpcSignal CAN_RPDO4_RECEIVED;
extern const mcu::LocalIpcSignal CAN_RSDO_RECEIVED;
#endif

#if (!defined(DUALCORE) && defined(CPU1))
extern const mcu::LocalIpcSignal CPU1_PERIPHERY_CONFIGURED;
extern const mcu::LocalIpcSignal REPORT_SENT;
extern const mcu::LocalIpcSignal SETUPMSG_RECEIVED;

extern const mcu::LocalIpcSignal CAN_TSDO_READY;
extern const mcu::LocalIpcSignal CAN_FRAME_RECEIVED;
extern const mcu::LocalIpcSignal CAN_RPDO1_RECEIVED;
extern const mcu::LocalIpcSignal CAN_RPDO2_RECEIVED;
extern const mcu::LocalIpcSignal CAN_RPDO3_RECEIVED;
extern const mcu::LocalIpcSignal CAN_RPDO4_RECEIVED;
extern const mcu::LocalIpcSignal CAN_RSDO_RECEIVED;
#endif
/// @}



