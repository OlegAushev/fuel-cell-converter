/**
 * @file
 * @ingroup mcu can_by_gpio
 */


#include "canbygpio.h"


namespace canbygpio {


const size_t STREAM_SIZE = 200;
static emb::Array<int, STREAM_SIZE> txBitStream;
static emb::Array<int, STREAM_SIZE> txCanBitStream;
static emb::Array<int, STREAM_SIZE> rxBitStream;
static emb::Array<int, STREAM_SIZE> rxCanBitStream;


///
///
///
Transceiver::Transceiver(const mcu::GpioPin& txPin, const mcu::GpioPin& rxPin, uint32_t bitrate)
	: m_txPin(txPin)
	, m_rxPin(rxPin)
{
	mcu::HighResolutionClock::init(1000000 / (2 * bitrate));
	mcu::HighResolutionClock::registerInterruptHandler(onClockInterrupt);
}


///
///
///
void Transceiver::send(uint64_t data)
{
	if (m_txActive)
	{
		++m_txError;
		return;
	}
	//emb::c28x::to_8bit_bytes(dest, src)

}


///
///
///
__interrupt void Transceiver::onClockInterrupt()
{

}


///
///
///
int Transceiver::_generateTxCanFrame(uint16_t* data, unsigned int dataLen, unsigned int frameId)
{
	assert(dataLen <= 9);
	assert(frameId <= 0x7FF);

	size_t idx = 0;

	// init bit streams
	txBitStream.fill(-1);
	txCanBitStream.fill(-1);

	// SOF
	txBitStream[idx++] = 0;

	// ID
	for (size_t i = 0; i < 11; ++i)
	{
		txBitStream[idx++] = ((frameId >> (10 - i)) & 1) ? 1 : 0;
	}

	// RTR, IDE, r0
	txBitStream[idx++] = 0;
	txBitStream[idx++] = 0;
	txBitStream[idx++] = 0;

	// DLC
	for (size_t i = 0; i < 4; ++i)
	{
		txBitStream[idx++] = ((dataLen >> (3 - i)) & 1) ? 1 : 0;
	}

	// DATA
	for (size_t i = 0; i < dataLen; ++i)
	{
		for (size_t j = 0; j < 8; ++j)
		{
			txBitStream[idx++] = ((data[i] >> (7 - j)) & 1) ? 1 : 0;
		}
	}

	// CRC
	uint16_t bitCount = idx;
	uint16_t crcReg = 0;
	int16_t crcNext;
	for (size_t i = 0; i < bitCount; ++i)
	{
		crcNext = txBitStream[i] ^ ((crcReg & 0x4000) >> 14);
		crcReg = (crcReg << 1) & 0x7FFE;
		if (crcNext)
		{
			crcReg = crcReg ^ 0x4599;	// CAN-15 CRC polynomial
		}
	}

	for (size_t i = 0; i < 15; ++i)
	{
		txBitStream[idx++] = ((crcReg >> (14 - i)) & 1) ? 1 : 0;
	}

	// CRC delimiter
	txBitStream[idx++] = 1;

	//Stuff bits: check for 5 consecutive bit states
	//then insert opposite bit state if this occurs
	bitCount = idx;
	int16_t prevBit = txBitStream[0];
	txCanBitStream[0] = txBitStream[0];
	size_t canIdx = 1;

	int16_t sameBits = 0;
	int16_t stuffBits = 0;

	for (size_t i = 1; i < bitCount; ++i)
	{
		txCanBitStream[canIdx++] = txBitStream[i];

		if(prevBit == txBitStream[i])
		{
			if (!sameBits)
				sameBits = 2;
			else
				++sameBits;

			if (sameBits == 5)
			{
				txCanBitStream[canIdx++] = (txBitStream[i] == 1) ? 0 : 1;
				sameBits = 0;
				++stuffBits;
			}
		}
		else
		{
			sameBits = 0;
		}

		prevBit = txCanBitStream[canIdx - 1];
	}

	// Append 14 recessive bits at the end of the bitstream
	for (size_t i = 0; i < 14; ++i)
	{
		txCanBitStream[canIdx++] = 1;
	}

	return canIdx;
}










} // namespace canbygpio


