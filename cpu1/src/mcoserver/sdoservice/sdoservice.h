/**
 * @defgroup mco_sdo_service SDO Service
 * @ingroup microcanopen
 *
 * @file
 * @ingroup microcanopen mco_sdo_service
 */


#pragma once


#include "driverlib.h"
#include "device.h"

#include <string.h>
#include <algorithm>
#include "../mcodef.h"
#include "../objectdictionary/objectdictionary.h"
#include "emb/emb_common.h"
#include "emb/emb_algorithm.h"
#include "mcu/can/mcucan.h"
#include "mcu/ipc/mcuipc.h"
#include "syslog/syslog.h"
#include "boostconverter/boostconverter.h"


namespace microcanopen {
/// @addtogroup mco_sdo_service
/// @{


/**
 * @brief SDO-service class.
 */
template <mcu::CanModule Module>
class SdoService
{
	friend class SdoServiceTest;
private:
	SdoService(const SdoService& other);			// no copy constructor
	SdoService& operator=(const SdoService& other);		// no copy assignment operator

	/// Data-storage for IPC
	static CobSdo m_rsdoData;

	/// Data-storage for IPC
	static CobSdo m_tsdoData;

	mcu::IpcSignalPair* RSDO_RECEIVED;
	mcu::IpcSignalPair* TSDO_READY;

public:
	/**
	 * @brief Configures IPC signals.
	 * @return (none)
	 */
	void initIpcSignals(mcu::IpcSignalPair* signalPairRsdoReceived,
			mcu::IpcSignalPair* signalPairTsdoReady)
	{
		RSDO_RECEIVED = signalPairRsdoReceived;
		TSDO_READY = signalPairTsdoReady;
	}

	/**
	 * @brief Saves RSDO messages and generates IPC signal. Used by McoServer's ISR as callback.
	 * @param rawMsg - RSDO message raw data
	 * @return (none)
	 */
	void processRsdo(uint64_t rawMsg);

	/**
	 * @brief Returns reference to TSDO data.
	 * @param (none)
	 * @return Reference to TSDO data.
	 */
	static const CobSdo& tsdoData() { return m_tsdoData; }

#ifdef CPU1
public:
	/**
	 * @brief Checks if there is new RSDO message and processes it. Used by McoServer's run().
	 * @param (none)
	 * @return (none)
	 */
	void processRequest();

private:
	/**
	 * @brief Processes RSDO and creates TSDO.
	 * @param rsdo - reference to CobSdo structure where SDO request is located
	 * @param tsdo - reference to CobSdo structure where SDO response should be placed
	 * @return (none)
	 */
	void _processRequest(const CobSdo& rsdo, CobSdo& tsdo);
#endif

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART BEGIN ====================== */
/* ========================================================================== */
public:
#ifdef CPU2
	/**
	 * @brief Configures service on CPU2.
	 */
	SdoService() {};
#endif

#ifdef CPU1
	/**
	 * @ingroup mco_app_spec
	 * @brief Configures service.
	 */
	SdoService(BoostConverter* converter)
	{
		od::converter = converter;

		std::sort(OBJECT_DICTIONARY, OBJECT_DICTIONARY_END);

		// Check OBJECT DICTIONARY correctness
		for (size_t i = 0; i < OD_SIZE; ++i)
		{
			// OD is sorted
			if (i < (OD_SIZE - 1))
			{
				assert(OBJECT_DICTIONARY[i] < OBJECT_DICTIONARY[i+1]);
			}

			for (size_t j = i+1; j < OD_SIZE; ++j)
			{
				// no od-entries with equal {index, subinex}
				assert((OBJECT_DICTIONARY[i].key.index != OBJECT_DICTIONARY[j].key.index)
					|| (OBJECT_DICTIONARY[i].key.subindex != OBJECT_DICTIONARY[j].key.subindex));

				// no od-entries with equal {category, subcategory, name}
				bool categoryEqual = ((strcmp(OBJECT_DICTIONARY[i].value.category, OBJECT_DICTIONARY[j].value.category) == 0) ? true : false);
				bool subcategoryEqual = ((strcmp(OBJECT_DICTIONARY[i].value.subcategory, OBJECT_DICTIONARY[j].value.subcategory) == 0) ? true : false);
				bool nameEqual = ((strcmp(OBJECT_DICTIONARY[i].value.name, OBJECT_DICTIONARY[j].value.name) == 0) ? true : false);
				assert(!categoryEqual || !subcategoryEqual || !nameEqual);
			}

			if (OBJECT_DICTIONARY[i].value.readAccess == true)
			{
				assert((OBJECT_DICTIONARY[i].value.readAccessFunc != OD_NO_READ_ACCESS)
						|| (OBJECT_DICTIONARY[i].value.dataPtr != OD_NO_DIRECT_ACCESS));
			}
			else
			{
				assert(OBJECT_DICTIONARY[i].value.readAccessFunc == OD_NO_READ_ACCESS
						&& (OBJECT_DICTIONARY[i].value.dataPtr == OD_NO_DIRECT_ACCESS));
			}

			if (OBJECT_DICTIONARY[i].value.writeAccess == true)
			{
				assert(OBJECT_DICTIONARY[i].value.writeAccessFunc != OD_NO_WRITE_ACCESS
						|| (OBJECT_DICTIONARY[i].value.dataPtr != OD_NO_DIRECT_ACCESS));
			}
			else
			{
				assert(OBJECT_DICTIONARY[i].value.writeAccessFunc == OD_NO_WRITE_ACCESS
						&& (OBJECT_DICTIONARY[i].value.dataPtr == OD_NO_DIRECT_ACCESS));
			}
		}
	}
#endif
/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART END ======================== */
/* ========================================================================== */
};


/// @}
} // namespace microcanopen




