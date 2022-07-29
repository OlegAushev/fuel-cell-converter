/**
 * @file
 * @ingroup can_by_gpio
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
Transceiver::Transceiver(const mcu::Gpio& txPin, const mcu::Gpio& rxPin,
		const mcu::Gpio& clkPin, uint32_t bitrate, tag::enable_bit_stuffing)
	: emb::c28x::Singleton<Transceiver>(this)
	, BIT_STUFFING_ENABLED(true)
{
	_init(txPin, rxPin, clkPin, bitrate);
}


///
///
///
Transceiver::Transceiver(const mcu::Gpio& txPin, const mcu::Gpio& rxPin,
		const mcu::Gpio& clkPin, uint32_t bitrate, tag::disable_bit_stuffing)
	: emb::c28x::Singleton<Transceiver>(this)
	, BIT_STUFFING_ENABLED(false)
{
	_init(txPin, rxPin, clkPin, bitrate);
}


///
///
///
void Transceiver::_init(const mcu::Gpio& txPin, const mcu::Gpio& rxPin,
		const mcu::Gpio& clkPin, uint32_t bitrate)
{
	m_txPin = txPin;
	m_rxPin = rxPin;
	m_clkPin = clkPin;
	m_rxPin.registerInterruptHandler(GPIO_INT_XINT5, GPIO_INT_TYPE_FALLING_EDGE, onRxStart);

	reset();

	mcu::HighResolutionClock::init(1000000 / (2 * bitrate));
	mcu::HighResolutionClock::registerInterruptHandler(onClockInterrupt);
	mcu::HighResolutionClock::start();

	m_rxPin.enableInterrupts();
}


///
///
///
void Transceiver::reset()
{
	m_txActive = false;
	m_txBitCount = 0;
	m_txIdx = 0;
	m_txError = 0;

	m_rxActive = false;
	m_rxSyncFlag = 0;
	m_rxBitCount = 0;
	m_rxIdx = 0;
	m_rxDataReady = false;

	m_clkFlag = 0;
}


///
///
///
__interrupt void Transceiver::onClockInterrupt()
{
	Transceiver* transceiver = Transceiver::instance();
	transceiver->m_clkFlag = 1 - transceiver->m_clkFlag;

	if ((transceiver->m_txActive) && transceiver->m_clkFlag)
	{
		if (transceiver->m_txIdx < transceiver->m_txBitCount)
		{
			uint32_t out = (txCanBitStream[transceiver->m_txIdx++] == 0) ? 0 : 1;
			GPIO_writePin(transceiver->m_txPin.config().no, out);
		}
		else
		{
			transceiver->m_txActive = false;
		}
	}

	if ((transceiver->m_rxActive) && (transceiver->m_clkFlag == transceiver->m_rxSyncFlag))
	{
		static int prevBit = 0;
		static int sameBits = 0;

		if (transceiver->m_rxIdx < STREAM_SIZE)
		{
			int bit = GPIO_readPin(transceiver->m_rxPin.config().no);
			rxCanBitStream[transceiver->m_rxIdx++] = bit;
			if (bit == prevBit)
			{
				if(!sameBits)
					sameBits = 2;
				else
					++sameBits;
			}
			else
			{
				sameBits = 0;
			}
			prevBit = bit;
			if (sameBits == 10)
			{
				prevBit = 0;
				sameBits = 0;
				transceiver->terminateRx();
			}
		}
		else
		{
			prevBit = 0;
			sameBits = 0;
			transceiver->terminateRx();
		}
	}
}


///
///
///
void Transceiver::terminateRx()
{
	m_rxActive = false;
	m_rxBitCount = m_rxIdx;
	m_rxPin.enableInterrupts();	// ready for new frame;
	m_rxDataReady = true;		// RX data can be read by recv()
	GPIO_togglePin(m_clkPin.config().no);
}


///
///
///
__interrupt void Transceiver::onRxStart()
{
	Transceiver* transceiver = Transceiver::instance();
	transceiver->m_rxSyncFlag = 1 - transceiver->m_clkFlag;	// begin receiving on next CLK INT
	transceiver->m_rxIdx = 0;
	transceiver->m_rxActive = true;
	transceiver->m_rxPin.disableInterrupts();		// no interrupts until this frame will be received

	transceiver->m_rxPin.acknowledgeInterrupt();
	GPIO_togglePin(transceiver->m_clkPin.config().no);
}


///
///
///
int Transceiver::generateTxCanFrame(unsigned int frameId, const uint16_t* buf, size_t len, bool bitStuffingEnabled)
{
	assert(len <= 9);
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
		txBitStream[idx++] = ((len >> (3 - i)) & 1) ? 1 : 0;
	}

	// DATA
	for (size_t i = 0; i < len; ++i)
	{
		for (size_t j = 0; j < 8; ++j)
		{
			txBitStream[idx++] = ((buf[i] >> (7 - j)) & 1) ? 1 : 0;
		}
	}

	// CRC
	size_t bitCount = idx;
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
	size_t canIdx = 0;

	if (bitStuffingEnabled)
	{
		int16_t prevBit = txBitStream[0];
		txCanBitStream[0] = txBitStream[0];
		++canIdx;

		int sameBits = 0;
		int stuffBits = 0;

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
	}
	else
	{
		for (size_t i = 0; i < bitCount; ++i)
		{
			txCanBitStream[canIdx++] = txBitStream[i];
		}
	}

	// Append 14 recessive bits at the end of the bitstream
	for (size_t i = 0; i < 14; ++i)
	{
		txCanBitStream[canIdx++] = 1;
	}

	return canIdx;
}


///
///
///
int Transceiver::parseRxCanFrame(unsigned int& frameId, uint16_t* buf, bool bitStuffingEnabled)
{
	// init bit stream
	rxBitStream.fill(-1);

	//Destuff bits: check for 5 consecutive bit states
	//then skip bit if this occurs
	size_t bitCount = rxCanBitStream.size();
	size_t canIdx = 0;
	size_t idx = 0;

	if (bitStuffingEnabled)
	{
		int16_t prevBit = rxBitStream[0];
		rxBitStream[0] = rxCanBitStream[0];
		++canIdx;
		++idx;

		int sameBits = 0;
		int stuffBits = 0;

		while (canIdx < bitCount)
		{
			rxBitStream[idx++] = rxCanBitStream[canIdx++];

			if(prevBit == rxCanBitStream[canIdx-1])
			{
				if (!sameBits)
					sameBits = 2;
				else
					++sameBits;

				if (sameBits == 5)
				{
					++canIdx;
					sameBits = 0;
					++stuffBits;
				}
			}
			else
			{
				sameBits = 0;
			}

			prevBit = rxCanBitStream[canIdx - 1];
		}
	}
	else
	{
		while (canIdx < bitCount)
		{
			rxBitStream[idx++] = rxCanBitStream[canIdx++];
		}
	}

	idx = 0;

	// SOF
	if (rxBitStream[idx++] != 0) return -1;

	// ID
	frameId = 0;
	for (size_t i = 0; i < 11; ++i)
	{
		frameId |= rxBitStream[idx++] << (10 - i);
	}

	// RTR, IDE, r0
	if (rxBitStream[idx++] != 0) return -2;
	if (rxBitStream[idx++] != 0) return -3;
	if (rxBitStream[idx++] != 0) return -4;

	// DLC
	int len = 0;
	for (size_t i = 0; i < 4; ++i)
	{
		len |= rxBitStream[idx++] << (3 - i);
	}

	// DATA
	for (size_t i = 0; i < len; ++i)
	{
		for (size_t j = 0; j < 8; ++j)
		{
			buf[i] |= rxBitStream[idx++] << (7 - j);
		}
	}

	// CRC
	bitCount = idx;
	uint16_t crcReg = 0;
	int16_t crcNext;
	for (size_t i = 0; i < bitCount; ++i)
	{
		crcNext = rxBitStream[i] ^ ((crcReg & 0x4000) >> 14);
		crcReg = (crcReg << 1) & 0x7FFE;
		if (crcNext)
		{
			crcReg = crcReg ^ 0x4599;	// CAN-15 CRC polynomial
		}
	}

	uint16_t crcRegRx = 0;
	for (size_t i = 0; i < 15; ++i)
	{
		crcRegRx |= rxBitStream[idx++] << (14 - i);
	}

	if (crcReg != crcRegRx) return -5;

	return len;
}



} // namespace canbygpio


