///
#pragma once


#include "../emb_common.h"


namespace emb {


/**
 * @brief UART interface class.
 */
class IUart
{
private:
	IUart(const IUart& other);		// no copy constructor
	IUart& operator=(const IUart& other);	// no copy assignment operator
public:
	IUart() {}
	virtual ~IUart() {}

	virtual void reset() = 0;
	virtual bool hasRxError() const = 0;

	virtual int recv(char& ch) = 0;
	virtual int recv(char* buf, size_t bufLen) = 0;

	virtual int send(char ch) = 0;
	virtual int send(const char* buf, uint16_t len) = 0;

	virtual void registerRxInterruptHandler(void (*handler)(void)) = 0;
	virtual void enableRxInterrupts() = 0;
	virtual void disableRxInterrupts() = 0;
	virtual void acknowledgeRxInterrupt() = 0;
};



} // namespace emb


