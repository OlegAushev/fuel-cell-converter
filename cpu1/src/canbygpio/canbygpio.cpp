/**
 * @file
 * @ingroup mcu can_by_gpio
 */


#include "canbygpio.h"


namespace canbygpio {


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
__interrupt void onClockInterrupt()
{

}







} // namespace canbygpio


