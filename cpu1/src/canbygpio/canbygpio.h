/**
 * @defgroup can_by_gpio CAN-by-GPIO
 *
 * @file
 * @ingroup can_by_gpio
 */


#pragma once


#include "emb/emb_common.h"
#include "emb/emb_array.h"
#include "emb/emb_math.h"

#include "mcu/gpio/mcugpio.h"
#include "mcu/cputimers/mcucputimers.h"


namespace canbygpio {
/// @addtogroup can_by_gpio
/// @{


/**
 * @brief CAN frame.
 */
struct Frame
{
	uint16_t IFS : 3;		// Inter-frame spacing, Must be recessive (1)
	uint16_t EndOfFrame : 7;	// End-of-frame, Must be recessive (1)
	uint16_t ACK_Delimiter : 1;	// ACK delimiter, Must be recessive (1)
	uint16_t ACK: 1;		// ACK slot, Transmitter sends recessive (1) and any receiver can assert a dominant (0)
	uint16_t CRC_Delimiter : 1;	// CRC delimiter, Must be recessive (1)
	uint16_t CRC : 15;		// Cyclic redundancy check
	uint16_t D7 : 8;
	uint16_t D6 : 8;
	uint16_t D5 : 8;
	uint16_t D4 : 8;
	uint16_t D3 : 8;
	uint16_t D2 : 8;
	uint16_t D1 : 8;
	uint16_t D0 : 8;
	uint16_t DLC : 4;		// Data length code (0-8 bytes)
	uint16_t r0 : 1;		// Reserved bit, Must be dominant (0)
	uint16_t IDE : 1;		// Identifier extension bit, Must be dominant (0) for base frame format with 11-bit identifiers
	uint16_t RTR : 1;		// Must be dominant (0) for data frames
	uint16_t ID : 11;		// Identifier
	uint16_t StartOfFrame : 1;	// Start-of-frame = 0
};


/**
 * @brief CAN-by-GPIO transceiver.
 * Uses mcu::Systick
 */
class Transceiver : emb::c28x::Singleton<Transceiver>
{
private:
	mcu::GpioPin m_txPin;
	mcu::GpioPin m_rxPin;
	mcu::GpioPin m_clkPin;

	bool m_txActive;
	int m_txBitCount;
	int m_txIdx;
	uint64_t m_txError;

	bool m_rxActive;
	unsigned int m_rxSyncFlag;
	int m_rxBitCount;
	int m_rxIdx;
	bool m_rxDataReady;
	uint64_t m_rxError;

	unsigned int m_clkFlag;

	uint16_t txData[8];
	uint16_t rxData[8];

public:
	Transceiver(const mcu::GpioPin& txPin, const mcu::GpioPin& rxPin,
			mcu::GpioPin& clkPin, uint32_t bitrate);
	void reset();

	template <typename T>
	void send(T data, unsigned int frameId)
	{
		EMB_STATIC_ASSERT(sizeof(T) <= 4);

		if (m_txActive)
		{
			++m_txError;
			return;
		}

		uint16_t dataBytes[2 * sizeof(T)];
		emb::c28x::to_8bit_bytes(dataBytes, data);
		m_txBitCount = _generateTxCanFrame(dataBytes, 2 * sizeof(T), frameId);
		m_txIdx = 0;
		m_txActive = true;
	}

	int recv(uint16_t* data, unsigned int& dataLen, unsigned int& frameId)
	{
		dataLen = 0;
		if (m_rxDataReady)
		{
			m_rxDataReady = false;
			return _parseRxCanFrame(data, dataLen, frameId);
		}
		return 1;
	}

protected:
	int _generateTxCanFrame(uint16_t* data, unsigned int dataLen, unsigned int frameId);
	int _parseRxCanFrame(uint16_t* data, unsigned int& dataLen, unsigned int& frameId);
	static __interrupt void onClockInterrupt();
	static __interrupt void onRxStart();

};













/// @}
} // namespace canbygpio


