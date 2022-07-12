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
 * @brief Local IPC flag.
 */
struct LocalIpcFlag
{
	uint32_t mask;
	LocalIpcFlag() : mask(0) {}
	explicit LocalIpcFlag(uint32_t flagNo)
		: mask(1UL << flagNo)
	{
		assert(flagNo < 32);
	}
};


/**
 * @brief Remote IPC flag.
 */
struct RemoteIpcFlag
{
	uint32_t mask;
	RemoteIpcFlag() : mask(0) {}
	explicit RemoteIpcFlag(uint32_t flagNo)
		: mask(1UL << flagNo)
	{
		assert(flagNo < 32);
	}
};


/**
 * @brief Local-Remote flag pair for objects which are created on both CPUs.
 */
struct IpcFlagPair
{
	LocalIpcFlag local;
	RemoteIpcFlag remote;
	IpcFlagPair() {}
	explicit IpcFlagPair(uint32_t flagNo)
		: local(flagNo)
		, remote(flagNo)
	{}
};


/**
 * @brief Sends IPC signal by setting local IPC flag.
 * @param ipcFlag
 * @return (none)
 */
inline void setLocalIpcFlag(LocalIpcFlag ipcFlag)
{
	IPCLtoRFlagSet(ipcFlag.mask);
}


/**
 * @brief Waits for IPC signal in blocking-mode.
 * @param ipcFlag
 * @return (none)
 */
inline void waitForRemoteIpcFlag(RemoteIpcFlag ipcFlag)
{
	while(!IPCRtoLFlagBusy(ipcFlag.mask));
	IPCRtoLFlagAcknowledge(ipcFlag.mask);
}


/**
 * @brief Checks if remote IPC signal has been sent.
 * @param ipcFlag
 * @return \c true if remote IPC signal has been sent, \c false otherwise.
 */
inline bool isRemoteIpcFlagSet(RemoteIpcFlag ipcFlag)
{
	return IPCRtoLFlagBusy(ipcFlag.mask);
}


/**
 * @brief Checks if local IPC signal has been sent.
 * @param ipcFlag
 * @return \c true if local IPC signal has been sent, \c false otherwise.
 */
inline bool isLocalIpcFlagSet(LocalIpcFlag ipcFlag)
{
	return IPCLtoRFlagBusy(ipcFlag.mask);
}


/**
 * @brief Acknowledges remote IPC signal.
 * @param ipcFlag
 * @return (none)
 */
inline void acknowledgeRemoteIpcFlag(RemoteIpcFlag ipcFlag)
{
	IPCRtoLFlagAcknowledge(ipcFlag.mask);
}


/**
 * @brief Revokes local IPC signal.
 * @param ipcFlag
 * @return (none)
 */
inline void resetLocalIpcFlag(LocalIpcFlag ipcFlag)
{
	IPCLtoRFlagClear(ipcFlag.mask);
}


/**
 * @brief Checks local or remote flag according to ipc mode.
 * @param ipcFlagPair - IPC flag pair
 * @param mode - IPC mode
 * @return \c true if flag is set, \c false otherwise.
 */
inline bool isIpcFlagSet(const IpcFlagPair& ipcFlagPair, IpcMode mode)
{
	switch (mode)
	{
	case mcu::IPC_MODE_SINGLECORE:
		return isLocalIpcFlagSet(ipcFlagPair.local);
	case mcu::IPC_MODE_DUALCORE:
		return isRemoteIpcFlagSet(ipcFlagPair.remote);
	}
	return false;
}


/**
 * @brief Resets local or remote flag according to ipc mode.
 * @param ipcFlagPair - IPC flag pair
 * @param mode - IPC mode
 * @return (none)
 */
inline void resetIpcFlag(const IpcFlagPair& ipcFlagPair, IpcMode mode)
{
	switch (mode)
	{
	case mcu::IPC_MODE_SINGLECORE:
		resetLocalIpcFlag(ipcFlagPair.local);
		return;
	case mcu::IPC_MODE_DUALCORE:
		acknowledgeRemoteIpcFlag(ipcFlagPair.remote);
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
extern const mcu::LocalIpcFlag CPU1_PERIPHERY_CONFIGURED;

extern const mcu::RemoteIpcFlag CPU2_BOOTED;
extern const mcu::RemoteIpcFlag CPU2_PERIPHERY_CONFIGURED;
#endif


#if (defined(DUALCORE) && defined(CPU2))
extern const mcu::RemoteIpcFlag CPU1_PERIPHERY_CONFIGURED;

extern const mcu::LocalIpcFlag CPU2_BOOTED;
extern const mcu::LocalIpcFlag CPU2_PERIPHERY_CONFIGURED;
#endif


#if (!defined(DUALCORE) && defined(CPU1))
extern const mcu::LocalIpcFlag CPU1_PERIPHERY_CONFIGURED;
#endif


/// @}


