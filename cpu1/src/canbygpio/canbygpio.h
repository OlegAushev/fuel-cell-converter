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

#include "mcu/gpio/mcu_gpio.h"
#include "mcu/cputimers/mcu_cputimers.h"
#include "profiler/profiler.h"


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


/// Tags for tag dispatching
namespace tag {

struct enable_bit_stuffing {};
struct disable_bit_stuffing {};

} // namespace tag


/**
 * @brief CAN-by-GPIO transceiver.
 * Uses mcu::HighResolutionClock.
 */
class Transceiver : emb::c28x::Singleton<Transceiver>
{
private:
	const bool BIT_STUFFING_ENABLED;

	mcu::Gpio m_rxPin;
	mcu::Gpio m_txPin;
	mcu::Gpio m_clkPin;

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
	/**
	 * @brief Configures CAN-BY-GPIO transceiver with enabled bit stuffing.
	 * @param txPin - TX pin
	 * @param rxPin - RX pin
	 * @param clkPin - aux CLK pin
	 * @param bitrate - bitrate
	 */
	Transceiver(const mcu::Gpio& rxPin, const mcu::Gpio& txPin,
			const mcu::Gpio& clkPin, uint32_t bitrate, tag::enable_bit_stuffing);

	/**
	 * @brief Configures CAN-BY-GPIO transceiver with disabled bit stuffing.
	 * @param txPin - TX pin
	 * @param rxPin - RX pin
	 * @param clkPin - aux CLK pin
	 * @param bitrate - bitrate
	 */
	Transceiver(const mcu::Gpio& rxPin, const mcu::Gpio& txPin,
			const mcu::Gpio& clkPin, uint32_t bitrate, tag::disable_bit_stuffing);

	/**
	 * @brief Resets transceiver.
	 * @param (none)
	 * @return (none)
	 */
	void reset();

	/**
	 * @brief Sends a CAN frame.
	 * @param frameId - CAN frame ID
	 * @param buf - CAN frame data buffer
	 * @param len - CAN frame data length
	 * @return Number of bytes sent, or error code if an error occurred.
	 */
	int send(unsigned int frameId, const uint16_t* buf, size_t len)
	{
		if (m_txActive)
		{
			++m_txError;
			return 0;
		}

		m_txBitCount = generateTxCanFrame(frameId, buf, len, BIT_STUFFING_ENABLED);
		m_txIdx = 0;
		m_txActive = true;
		return len;
	}

	/**
	 * @brief Receives a CAN frame.
	 * @param frameId - CAN frame ID
	 * @param buf - frame data buffer
	 * @return Number of bytes received, or error code if an error occurred.
	 */
	int recv(unsigned int& frameId, uint16_t* buf)
	{
		int retval = 0;
		if (m_rxDataReady)
		{
			retval = parseRxCanFrame(frameId, buf, BIT_STUFFING_ENABLED);
			m_rxDataReady = false;
		}
		return retval;
	}

protected:
	void _init(const mcu::Gpio& rxPin, const mcu::Gpio& txPin,
			const mcu::Gpio& clkPin, uint32_t bitrate);
	int generateTxCanFrame(unsigned int frameId, const uint16_t* buf, size_t len, bool bitStuffingEnabled);
	int parseRxCanFrame(unsigned int& frameId, uint16_t* buf, bool bitStuffingEnabled);
	void terminateRx();
	static __interrupt void onClockInterrupt();
	static __interrupt void onRxStart();

};


/// @}
} // namespace canbygpio


