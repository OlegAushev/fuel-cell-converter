/**
 * @defgroup can_by_gpio CAN-by-GPIO
 *
 * @file
 * @ingroup can_by_gpio
 */


#pragma once


#include "emb/emb_common.h"

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
	uint16_t EOF : 7;		// End-of-frame, Must be recessive (1)
	uint16_t ACK_Delimiter;		// ACK delimiter, Must be recessive (1)
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
	uint16_t SOF : 1;		// Start-of-frame = 0
};


/**
 * @brief CAN-by-GPIO transceiver.
 * Uses mcu::Systick
 */
class Transceiver
{
private:
	mcu::GpioPin m_txPin;
	mcu::GpioPin m_rxPin;

	bool m_txActive;
	uint64_t m_txError;
	unsigned int m_txBitCounter;

public:
	Transceiver(const mcu::GpioPin& txPin, const mcu::GpioPin& rxPin, uint32_t bitrate);
	void send(uint64_t data);
private:
	static __interrupt void onClockInterrupt();


};













/// @}
} // namespace canbygpio


