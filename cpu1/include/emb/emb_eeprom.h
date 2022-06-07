///
#pragma once


#include <stdint.h>
#include <stddef.h>


namespace emb {


enum EepromStatus
{
	EEPROM_SUCCESS,
	EEPROM_READ_FAIL,
	EEPROM_WRITE_FAIL,
	EEPROM_TIMEOUT,
	EEPROM_NOT_DETECTED,
	EEPROM_INVALID_ADDRESS
};


struct EepromAddr
{
	unsigned int page;
	unsigned int offset;
	EepromAddr() : page(0), offset(0) {}
	explicit EepromAddr(unsigned int _page) : page(_page), offset(0) {}
	EepromAddr(unsigned int _page, unsigned int _offset) : page(_page), offset(_offset) {}
};


class IEeprom
{
public:
	virtual EepromStatus write(EepromAddr addr, const char* data, unsigned int nBytes, uint64_t timeoutMs) = 0;
	virtual EepromStatus read(EepromAddr addr, char* data, unsigned int nBytes, uint64_t timeoutMs) = 0;
};


class EepromStorage
{
private:
	IEeprom* m_eeprom;
	const unsigned int m_pageSize;
	const unsigned int m_nPages;

public:
	EepromStorage(IEeprom* eeprom, unsigned int pageSize, unsigned int nPages)
		: m_eeprom(eeprom)
		, m_pageSize(pageSize)
		, m_nPages(nPages)
	{}

	template <typename T>
	EepromStatus write(EepromAddr addr, const T& data, uint64_t timeoutMs)
	{
		if ((addr.page >= m_nPages/3) || (addr.offset + sizeof(T) >= m_pageSize))
		{
			return EEPROM_INVALID_ADDRESS;
		}

		char dataBuf[sizeof(T)];
		memcpy(dataBuf, &data, sizeof(T));

		EepromStatus s1 = m_eeprom->write(addr,
				dataBuf, sizeof(T), timeoutMs);
		EepromStatus s2 = m_eeprom->write(EepromAddr(addr.page + m_nPages/3, addr.offset),
				dataBuf, sizeof(T), timeoutMs);
		EepromStatus s3 = m_eeprom->write(EepromAddr(addr.page + 2*m_nPages/3, addr.offset),
				dataBuf, sizeof(T), timeoutMs);

		if ((s1 == s2 == EEPROM_SUCCESS)
				|| (s2 == s3 == EEPROM_SUCCESS)
				|| (s1 == s3 == EEPROM_SUCCESS))
		{
			return EEPROM_SUCCESS;
		}
		return EEPROM_WRITE_FAIL;
	}


	template <typename T>
	EepromStatus read(EepromAddr addr, T& data, uint64_t timeoutMs)
	{
		if ((addr.page >= m_nPages/3) || (addr.offset + sizeof(T) >= m_pageSize))
		{
			return EEPROM_INVALID_ADDRESS;
		}
	}
};

} // namespace emb


