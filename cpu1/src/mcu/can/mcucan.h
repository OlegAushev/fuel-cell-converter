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
#include "../system/mcusystem.h"
#include "../gpio/mcugpio.h"
#include "emb/emb_common.h"


namespace mcu {
/// @addtogroup mcu_can
/// @{


/// CAN modules
enum CanModule
{
	CANA,
	CANB
};


/// CAN bitrates
enum CanBitrate
{
	CAN_BITRATE_125K = 125000,
	CAN_BITRATE_500K = 500000,
	CAN_BITRATE_1M = 1000000,
};


///
enum CanMode
{
	CAN_NORMAL_MODE = 0,
	CAN_SILENT_MODE = CAN_TEST_SILENT,
	CAN_LOOPBACK_MODE = CAN_TEST_LBACK,
	CAN_EXLOOPBACK_MODE = CAN_TEST_EXL,
	CAN_SILENTLOOPBACK_MODE = CAN_TEST_SILENT | CAN_TEST_LBACK
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


namespace detail {


/**
 * @brief CAN module implementation.
 */
struct CanModuleImpl
{
	uint32_t base;
	uint32_t pieIntNum;
	CanModuleImpl(uint32_t _base, uint32_t _pieIntNum)
		: base(_base), pieIntNum(_pieIntNum) {}
};


extern const uint32_t canBases[2];
extern const uint32_t canPieIntNums[2];


} // namespace detail


/**
 * @brief CAN unit class.
 */
template <CanModule Module>
class CanUnit : public emb::c28x::Singleton<CanUnit<Module> >
{
private:
	detail::CanModuleImpl m_module;

	CanUnit(const CanUnit& other);			// no copy constructor
	CanUnit& operator=(const CanUnit& other);	// no copy assignment operator
public:
	/**
	 * @brief Initializes MCU CAN unit.
	 * @param txPin - MCU CAN-TX pin config
	 * @param rxPin	- MCU CAN-RX pin config
	 * @param bitrate - CAN bus bitrate
	 */
	CanUnit(const GpioPinConfig& txPin, const GpioPinConfig& rxPin,
			CanBitrate bitrate, CanMode mode)
		: emb::c28x::Singleton<CanUnit<Module> >(this)
		, m_module(detail::canBases[Module], detail::canPieIntNums[Module])
	{
#ifdef CPU1
		_initPins(txPin, rxPin);
#endif

		CAN_initModule(m_module.base);
		CAN_selectClockSource(m_module.base, CAN_CLOCK_SOURCE_SYS);

		switch (bitrate)
		{
		case CAN_BITRATE_125K:
		case CAN_BITRATE_500K:
			CAN_setBitRate(m_module.base, mcu::sysclkFreq(), static_cast<uint32_t>(bitrate), 16);
			break;
		case CAN_BITRATE_1M:
			CAN_setBitRate(m_module.base, mcu::sysclkFreq(), static_cast<uint32_t>(bitrate), 10);
			break;
		}

		CAN_setAutoBusOnTime(m_module.base, 0);
		CAN_enableAutoBusOn(m_module.base);

		if (mode != CAN_NORMAL_MODE)
		{
			CAN_enableTestMode(m_module.base, static_cast<uint16_t>(mode));
		}

		CAN_startModule(m_module.base);
	}

#ifdef CPU1
	/**
	 * @brief Transfers control over CAN unit to CPU2.
	 * @param txPin - MCU CAN-TX pin config
	 * @param rxPin - MCU CAN-RX pin config
	 * @return (none)
	 */
	static void transferControlToCpu2(const GpioPinConfig& txPin, const GpioPinConfig& rxPin)
	{
		_initPins(txPin, rxPin);
		GPIO_setMasterCore(txPin.no, GPIO_CORE_CPU2);
		GPIO_setMasterCore(rxPin.no, GPIO_CORE_CPU2);

		SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL8_CAN,
				static_cast<uint16_t>(Module)+1, SYSCTL_CPUSEL_CPU2);
	}
#endif

	/**
	 * @brief Returns base of CAN-unit.
	 * @param (none)
	 * @return Base of CAN-unit.
	 */
	uint32_t base() const { return m_module.base; }

	/**
	 * @brief Retrieves received data.
	 * @param objId - message object ID
	 * @param dataDest - pointer to data destination
	 * @return \c true if new data was retrieved, \c false otherwise.
	 */
	bool recv(uint32_t objId, uint16_t* dataDest) const
	{
		return CAN_readMessage(m_module.base, objId, dataDest);
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
		CAN_sendMessage(m_module.base, objId, dataLen, dataSrc);
	}

	/**
	 * @brief Setups message object.
	 * @param msgObj - message object
	 * @return (none)
	 */
	void setupMessageObject(CanMessageObject& msgObj) const
	{
		CAN_setupMessageObject(m_module.base, msgObj.objId, msgObj.frameId, msgObj.frameType,
				msgObj.objType, msgObj.frameIdMask, msgObj.flags, msgObj.dataLen);
	}

	/**
	 * @brief Registers interrupt handler.
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	void registerInterruptHandler(void (*handler)(void)) const
	{
		Interrupt_register(m_module.pieIntNum, handler);
		CAN_enableInterrupt(m_module.base, CAN_INT_IE0 | CAN_INT_ERROR | CAN_INT_STATUS);
		CAN_enableGlobalInterrupt(m_module.base, CAN_GLOBAL_INT_CANINT0);
	}

	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts() const { Interrupt_enable(m_module.pieIntNum); }

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts() const { Interrupt_disable(m_module.pieIntNum); }

protected:
#ifdef CPU1
	static void _initPins(const GpioPinConfig& txPin, const GpioPinConfig& rxPin)
	{
		GPIO_setPinConfig(txPin.mux);
		GPIO_setPinConfig(rxPin.mux);
	}
#endif
};


/// @}
} // namespace mcu


