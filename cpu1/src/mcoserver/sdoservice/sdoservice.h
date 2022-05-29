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
#include "../objectdictionary/objectdictionary.h"
#include "emb/emb_common.h"
#include "emb/emb_algorithm.h"
#include "mcu/ipc/mcuipc.h"
#include "syslog/syslog.h"


namespace microcanopen {
/// @addtogroup mco_sdo_service
/// @{


/**
 * @brief SDO-service class.
 */
class SdoService
{
	friend class SdoServiceTest;
private:
	SdoService(const SdoService& other);			// no copy constructor
	SdoService& operator=(const SdoService& other);		// no copy assignment operator
public:
	/**
	 * @brief Saves RSDO messages and generates IPC signal. Used by McoServer as callback.
	 * @param rawMsg - RSDO message raw data
	 * @return (none)
	 */
	static void processRsdo(uint64_t rawMsg)
	{
#if (!defined(DUALCORE) || defined(CPU2))
		if (!mcu::isIpcSignalSet(CAN_RSDO_RECEIVED))
		{
			rsdoData.cob = CobSdo(rawMsg);
			mcu::sendIpcSignal(CAN_RSDO_RECEIVED);
		}
		else
		{
			Syslog::setWarning(Warning::CAN_BUS_WARNING);
			Syslog::addMessage(SyslogMsg::SDO_REQUEST_LOST);
		}
#else
		while (true) {} // CPU1 in DUALCORE build must not get here
#endif
	}

	/**
	 * @brief Data-storage for IPC.
	 */
	struct SdoData
	{
		CobSdo cob;
	};
	static SdoData rsdoData;
	static SdoData tsdoData;

#ifdef CPU1
	/**
	 * @brief Checks if there is new RSDO message and processes it.
	 * @param (none)
	 * @return (none)
	 */
	void processRequest()
	{
		if (!mcu::isIpcSignalSet(CAN_RSDO_RECEIVED))
		{
			return;
		}
		processSdoRequest(rsdoData.cob, tsdoData.cob);
		mcu::acknowledgeIpcSignal(CAN_RSDO_RECEIVED);
	}

private:
	/**
	 * @brief Processes RSDO and creates TSDO.
	 * @param rsdo - reference to CobSdo structure where SDO request is located
	 * @param tsdo - reference to CobSdo structure where SDO response should be placed
	 * @return (none)
	 */
	void processSdoRequest(const CobSdo& rsdo, CobSdo& tsdo)
	{
		ODAccessStatus status = OD_ACCESS_NO_ACCESS;

		const ODEntry* odEntry = emb::binary_find(OBJECT_DICTIONARY, OBJECT_DICTIONARY_END,
				ODEntryKeyAux(rsdo.index, rsdo.subindex));

		if (odEntry == OBJECT_DICTIONARY_END)
		{
			return; // OD-entry not found;
		}

		if (rsdo.cs == SDO_CCS_READ)
		{
			if (odEntry->value.dataPtr != OD_NO_DIRECT_ACCESS)
			{
				tsdo.data.u32 = 0;
				memcpy(&tsdo.data.u32, odEntry->value.dataPtr, ODEntryDataSizes[odEntry->value.dataType]);
				status = OD_ACCESS_SUCCESS;
			}
			else
			{
				status = odEntry->value.readAccessFunc(tsdo.data);
			}
		}
		else if (rsdo.cs == SDO_CCS_WRITE)
		{
			if (odEntry->value.dataPtr != OD_NO_DIRECT_ACCESS)
			{
				memcpy(odEntry->value.dataPtr, &rsdo.data.u32, ODEntryDataSizes[odEntry->value.dataType]);
				status = OD_ACCESS_SUCCESS;
			}
			else
			{
				status = odEntry->value.writeAccessFunc(rsdo.data);
			}
		}
		else
		{
			return;
		}

		switch (status)
		{
		case OD_ACCESS_SUCCESS:
			tsdo.index = rsdo.index;
			tsdo.subindex = rsdo.subindex;
			if (rsdo.cs == SDO_CCS_READ)
			{
				tsdo.cs = SDO_SCS_READ;	// read/upload response
				tsdo.expeditedTransfer = 1;
				tsdo.dataSizeIndicated = 1;
				tsdo.dataEmptyBytes = 0;
			}
			else if (rsdo.cs == SDO_CCS_WRITE)
			{
				tsdo.cs = SDO_SCS_WRITE;	// write/download response
				tsdo.expeditedTransfer = 0;
				tsdo.dataSizeIndicated = 0;
				tsdo.dataEmptyBytes = 0;
			}
			else
			{
				return;
			}
			mcu::sendIpcSignal(CAN_TSDO_READY);
			break;

		case OD_ACCESS_FAIL:
			return;

		case OD_ACCESS_NO_ACCESS:
			return;
		}
	}
#endif

/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART BEGIN ====================== */
/* ========================================================================== */
public:
	/**
	 * @ingroup mco_app_spec
	 * @brief Configures service.
	 * @param clock
	 * @param drive
	 * @param syslog
	 * @param setupManager
	 */
	SdoService()
	{
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
/* ========================================================================== */
/* =================== APPLICATION-SPECIFIC PART END ======================== */
/* ========================================================================== */
};


/// @}
} // namespace microcanopen




