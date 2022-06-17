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


///
enum IpcMode
{
	IPC_MODE_SINGLECORE,
	IPC_MODE_DUALCORE
};


/**
 * @brief Local IPC signal.
 */
struct LocalIpcSignal
{
	uint32_t flag;
	LocalIpcSignal() : flag(0) {}
	explicit LocalIpcSignal(uint32_t flagNo)
		: flag(1UL << flagNo)
	{
		assert(flagNo < 32);
	}
};


/**
 * @brief Remote IPC signal.
 */
struct RemoteIpcSignal
{
	uint32_t flag;
	RemoteIpcSignal() : flag(0) {}
	explicit RemoteIpcSignal(uint32_t flagNo)
		: flag(1UL << flagNo)
	{
		assert(flagNo < 32);
	}
};


/**
 * @brief Local-Remote signal pair for objects which are created on both CPUs.
 */
struct IpcSignalPair
{
	LocalIpcSignal local;
	RemoteIpcSignal remote;
	IpcSignalPair() {}
	explicit IpcSignalPair(uint32_t flagNo)
		: local(flagNo)
		, remote(flagNo)
	{}
};


/**
 * @brief Sends IPC signal by setting local IPC flag.
 * @param ipcFlag
 * @return (none)
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


/**
 * @brief Checks if remote IPC signal has been sent.
 * @param ipcFlag
 * @return \c true if remote IPC signal has been sent, \c false otherwise.
 */
inline bool remoteIpcSignalSent(RemoteIpcSignal ipcFlag)
{
	if (IPCRtoLFlagBusy(ipcFlag.flag))
	{
		return true;
	}
	return false;
}


/**
 * @brief Checks if local IPC signal has been sent.
 * @param ipcFlag
 * @return \c true if local IPC signal has been sent, \c false otherwise.
 */
inline bool localIpcSignalSent(LocalIpcSignal ipcFlag)
{
	if (IPCLtoRFlagBusy(ipcFlag.flag))
	{
		return true;
	}
	return false;
}


/**
 * @brief Acknowledges remote IPC signal.
 * @param ipcFlag
 * @return (none)
 */
inline void acknowledgeRemoteIpcSignal(RemoteIpcSignal ipcFlag)
{
	IPCRtoLFlagAcknowledge(ipcFlag.flag);
}


/**
 * @brief Revokes local IPC signal.
 * @param ipcFlag
 * @return (none)
 */
inline void revokeLocalIpcSignal(LocalIpcSignal ipcFlag)
{
	IPCLtoRFlagClear(ipcFlag.flag);
}


/**
 * @brief Checks local or remote flag according to ipc mode.
 * @param ipcSignalPair - IPC signal pair
 * @param mode - IPC mode
 * @return \c true if flag is set, \c false otherwise.
 */
inline bool ipcSignalSent(const IpcSignalPair& ipcSignalPair, IpcMode mode)
{
	switch (mode)
	{
	case mcu::IPC_MODE_SINGLECORE:
		return localIpcSignalSent(ipcSignalPair.local);
	case mcu::IPC_MODE_DUALCORE:
		return remoteIpcSignalSent(ipcSignalPair.remote);
	}
	return false;
}


/**
 * @brief Resets local or remote flag according to ipc mode.
 * @param ipcSignalPair - IPC signal pair
 * @param mode - IPC mode
 * @return (none)
 */
inline void resetIpcSignal(const IpcSignalPair& ipcSignalPair, IpcMode mode)
{
	switch (mode)
	{
	case mcu::IPC_MODE_SINGLECORE:
		revokeLocalIpcSignal(ipcSignalPair.local);
		return;
	case mcu::IPC_MODE_DUALCORE:
		acknowledgeRemoteIpcSignal(ipcSignalPair.remote);
		return;
	}
}


/// IPC interrupts
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

extern const mcu::RemoteIpcSignal CPU2_BOOTED;
extern const mcu::RemoteIpcSignal CPU2_PERIPHERY_CONFIGURED;
#endif


#if (defined(DUALCORE) && defined(CPU2))
extern const mcu::RemoteIpcSignal CPU1_PERIPHERY_CONFIGURED;

extern const mcu::LocalIpcSignal CPU2_BOOTED;
extern const mcu::LocalIpcSignal CPU2_PERIPHERY_CONFIGURED;
#endif


#if (!defined(DUALCORE) && defined(CPU1))
extern const mcu::LocalIpcSignal CPU1_PERIPHERY_CONFIGURED;
#endif


/// @}


