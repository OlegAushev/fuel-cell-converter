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
Transceiver::Transceiver(const mcu::GpioPin& txPin, const mcu::GpioPin& rxPin,
		mcu::GpioPin& clkPin, uint32_t bitrate)
	: emb::c28x::Singleton<Transceiver>(this)
	, m_txPin(txPin)
	, m_rxPin(rxPin)
	, m_clkPin(clkPin)
{
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
	m_rxBitCount = STREAM_SIZE;
	m_rxIdx = 0;
	m_rxDataReady = false;

	m_clkFlag = 0;
}


///
///
///
__interrupt void Transceiver::onClockInterrupt()
{
	Transceiver* tranceiver = Transceiver::instance();
	tranceiver->m_clkFlag = 1 - tranceiver->m_clkFlag;

	if ((tranceiver->m_txActive) && tranceiver->m_clkFlag)
	{
		if (tranceiver->m_txIdx < tranceiver->m_txBitCount)
		{
			uint32_t out = (txCanBitStream[tranceiver->m_txIdx++] == 0) ? 0 : 1;
			GPIO_writePin(tranceiver->m_txPin.config().no, out);
		}
		else
		{
			tranceiver->m_txActive = false;
		}
	}

	if ((tranceiver->m_rxActive) && (tranceiver->m_clkFlag == tranceiver->m_rxSyncFlag))
	{
		if (tranceiver->m_rxIdx < tranceiver->m_rxBitCount)
		{
			rxCanBitStream[tranceiver->m_rxIdx++] =
					GPIO_readPin(tranceiver->m_rxPin.config().no);
		}
		else
		{
			tranceiver->m_rxActive = false;
			tranceiver->m_rxPin.enableInterrupts(); // ready for new frame;
			tranceiver->m_rxDataReady = true;
			GPIO_togglePin(tranceiver->m_clkPin.config().no);
		}
	}
}


///
///
///
__interrupt void Transceiver::onRxStart()
{
	Transceiver* tranceiver = Transceiver::instance();
	tranceiver->m_rxSyncFlag = 1 - tranceiver->m_clkFlag; // begin receiving on next CLK INT
	tranceiver->m_rxIdx = 0;
	tranceiver->m_rxActive = true;
	tranceiver->m_rxPin.disableInterrupts();

	tranceiver->m_rxPin.acknowledgeInterrupt();
	GPIO_togglePin(tranceiver->m_clkPin.config().no);
}


///
///
///
int Transceiver::_generateTxCanFrame(int frameId, int dataLen, uint16_t* data)
{
	LOG_DURATION_VIA_PIN_ONOFF(m_clkPin.config().no);

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
	int16_t prevBit = txBitStream[0];
	txCanBitStream[0] = txBitStream[0];
	size_t canIdx = 1;

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
int Transceiver::_parseRxCanFrame(int& frameId, uint16_t* data)
{
	LOG_DURATION_VIA_PIN_ONOFF(m_clkPin.config().no);

	// init bit stream
	rxBitStream.fill(-1);

	//Destuff bits: check for 5 consecutive bit states
	//then skip bit if this occurs
	size_t bitCount = rxCanBitStream.size();
	int16_t prevBit = rxBitStream[0];
	rxBitStream[0] = rxCanBitStream[0];
	size_t canIdx = 1;
	size_t idx = 1;

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
	int dataLen = 0;
	for (size_t i = 0; i < 4; ++i)
	{
		dataLen |= rxBitStream[idx++] << (3 - i);
	}

	// DATA
	for (size_t i = 0; i < dataLen; ++i)
	{
		for (size_t j = 0; j < 8; ++j)
		{
			data[i] |= rxBitStream[idx++] << (7 - j);
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

	return dataLen;
}








} // namespace canbygpio


