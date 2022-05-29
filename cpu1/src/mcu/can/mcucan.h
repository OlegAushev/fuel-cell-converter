/**
 * @defgroup mcu_can CAN
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_can
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "emb/emb_common.h"
#include "../system/mcusystem.h"


namespace mcu {
/// @addtogroup mcu_can
/// @{


/// CAN modules
enum CanModule
{
	CANA,
	CANB
};

/// CAN TX pins
enum CanTxPin
{
	CANA_TX_GPIO_19,
	CANA_TX_GPIO_31,
	CANB_TX_GPIO_12
};

/// CAN RX pins
enum CanRxPin
{
	CANA_RX_GPIO_18,
	CANA_RX_GPIO_30,
	CANB_RX_GPIO_17
};

/// CAN bitrates
enum CanBitrate
{
	CAN_BITRATE_125K = 125000,
	CAN_BITRATE_500K = 500000,
	CAN_BITRATE_1M = 1000000,
};

/**
 * @brief CAN module implementation.
 */
struct CanModuleImpl
{
	const uint32_t base;
	uint32_t txPin;
	uint32_t txPinMux;
	uint32_t rxPin;
	uint32_t rxPinMux;
	const uint32_t pieIntNo;
};

/**
 * @brief CAN message object.
 */
struct CanMessageObject
{
	uint32_t objId;
	uint32_t frameId;
	CAN_MsgFrameType frameType;
	CAN_MsgObjType objType;
	uint32_t frameIdMask;
	uint32_t flags;
	uint16_t dataLen;
	uint16_t data[8];
};

/**
 * @brief CAN unit class.
 */
template <CanModule Module>
class CanUnit : public emb::c28x::Singleton<CanUnit<Module> >
{
private:
	CanUnit(const CanUnit& other);			// no copy constructor
	CanUnit& operator=(const CanUnit& other);	// no copy assignment operator

public:
	/// CAN module
	static CanModuleImpl module;

	/**
	 * @brief Initializes MCU CAN unit.
	 * @param txPin - MCU CAN-TX pin
	 * @param rxPin	- MCU CAN-RX pin
	 * @param bitrate - CAN bus bitrate
	 */
	CanUnit(CanTxPin txPin, CanRxPin rxPin, CanBitrate bitrate);

#ifdef CPU1
	/**
	 * @brief Transfers control over CAN unit to CPU2.
	 * @param txPin - MCU CAN-TX pin
	 * @param rxPin - MCU CAN-RX pin
	 * @return (none)
	 */
	static void transferControlToCpu2(CanTxPin txPin, CanRxPin rxPin);
#endif

	/**
	 * @brief Retrieves received data.
	 * @param objId - message object ID
	 * @param dataDest - pointer to data destination
	 * @return \c true if new data was retrieved, \c false otherwise.
	 */
	bool recv(uint32_t objId, uint16_t* dataDest) const
	{
		return CAN_readMessage(module.base, objId, dataDest);
	}

	/**
	 * @brief Sends data.
	 * @param objId - message object ID
	 * @param dataSrc - pointer to data source
	 * @param dataLen - data length
	 * @return (none)
	 */
	void send(uint32_t objId, const uint16_t* dataSrc, uint16_t dataLen) const
	{
		CAN_sendMessage(module.base, objId, dataLen, dataSrc);
	}

	/**
	 * @brief Setups message object.
	 * @param msgObj - message object
	 * @return (none)
	 */
	void setupMessageObject(CanMessageObject& msgObj) const
	{
		CAN_setupMessageObject(module.base, msgObj.objId, msgObj.frameId, msgObj.frameType,
				msgObj.objType, msgObj.frameIdMask, msgObj.flags, msgObj.dataLen);
	}

	/**
	 * @brief Registers Rx-interrupt handler.
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	void registerRxInterruptHandler(void (*handler)(void)) const
	{
		Interrupt_register(module.pieIntNo, handler);
		CAN_enableInterrupt(module.base, CAN_INT_IE0 | CAN_INT_ERROR | CAN_INT_STATUS);
		CAN_enableGlobalInterrupt(module.base, CAN_GLOBAL_INT_CANINT0);
	}

	/**
	 * @brief Enables Rx-interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void enableRxInterrupt() const { Interrupt_enable(module.pieIntNo); }

	/**
	 * @brief Disables Rx-interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void disableRxInterrupt() const { Interrupt_disable(module.pieIntNo); }
};


/// @}
} /* namespace mcu */


