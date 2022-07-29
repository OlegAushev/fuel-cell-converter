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
#include "../mco_def.h"
#include "../objectdictionary/objectdictionary.h"
#include "emb/emb_common.h"
#include "emb/emb_algorithm.h"
#include "mcu/can/mcu_can.h"
#include "mcu/ipc/mcu_ipc.h"

// APP-SPECIFIC headers
#include "syslog/syslog.h"


namespace microcanopen {
/// @addtogroup mco_sdo_service
/// @{


extern CobSdo mcoCan1RsdoDataShared;
extern CobSdo mcoCan2RsdoDataShared;
extern CobSdo mcoCan1RsdoDataNonShared;
extern CobSdo mcoCan2RsdoDataNonShared;
extern CobSdo mcoCan1TsdoDataShared;
extern CobSdo mcoCan2TsdoDataShared;
extern CobSdo mcoCan1TsdoDataNonShared;
extern CobSdo mcoCan2TsdoDataNonShared;


/**
 * @brief SDO-service class.
 */
template <mcu::CanModule Module, mcu::IpcMode Ipc, emb::MasterSlaveMode Mode>
class SdoService
{
	friend class SdoServiceTest;
private:
	mcu::IpcFlag RSDO_RECEIVED;
	mcu::IpcFlag TSDO_READY;

	/// Data-storage for IPC
	static CobSdo* s_rsdoData;

	/// Data-storage for IPC
	static CobSdo* s_tsdoData;

private:
	SdoService(const SdoService& other);			// no copy constructor
	SdoService& operator=(const SdoService& other);		// no copy assignment operator
public:
	/**
	 * @brief Configures service.
	 */
	SdoService()
	{
		EMB_STATIC_ASSERT(Ipc == mcu::IPC_MODE_DUALCORE);
		EMB_STATIC_ASSERT(Mode == emb::MODE_MASTER);
		switch (Module)
		{
		case MCO_CAN1:
			switch (Ipc)
			{
			case mcu::IPC_MODE_SINGLECORE:
				s_rsdoData = &mcoCan1RsdoDataNonShared;
				s_tsdoData = &mcoCan1TsdoDataNonShared;
				break;
			case mcu::IPC_MODE_DUALCORE:
				s_rsdoData = &mcoCan1RsdoDataShared;
				s_tsdoData = &mcoCan1TsdoDataShared;
				break;
			}
			break;
		case MCO_CAN2:
			switch (Ipc)
			{
			case mcu::IPC_MODE_SINGLECORE:
				s_rsdoData = &mcoCan2RsdoDataNonShared;
				s_tsdoData = &mcoCan2TsdoDataNonShared;
				break;
			case mcu::IPC_MODE_DUALCORE:
				s_rsdoData = &mcoCan2RsdoDataShared;
				s_tsdoData = &mcoCan2TsdoDataShared;
				break;
			}
			break;
		}
	}

	/**
	 * @ingroup mco_app_spec
	 * @brief Configures service on server that processes requests.
	 */
	SdoService(fuelcell::Converter* converter)
	{
		switch (Module)
		{
		case MCO_CAN1:
			switch (Ipc)
			{
			case mcu::IPC_MODE_SINGLECORE:
				s_rsdoData = &mcoCan1RsdoDataNonShared;
				s_tsdoData = &mcoCan1TsdoDataNonShared;
				break;
			case mcu::IPC_MODE_DUALCORE:
				s_rsdoData = &mcoCan1RsdoDataShared;
				s_tsdoData = &mcoCan1TsdoDataShared;
				break;
			}
			break;
		case MCO_CAN2:
			switch (Ipc)
			{
			case mcu::IPC_MODE_SINGLECORE:
				s_rsdoData = &mcoCan2RsdoDataNonShared;
				s_tsdoData = &mcoCan2TsdoDataNonShared;
				break;
			case mcu::IPC_MODE_DUALCORE:
				s_rsdoData = &mcoCan2RsdoDataShared;
				s_tsdoData = &mcoCan2TsdoDataShared;
				break;
			}
			break;
		}

		// APP-SPECIFIC BEGIN
		od::converter = converter;
		// APP-SPECIFIC END

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

public:
	/**
	 * @brief Configures IPC flags.
	 * @return (none)
	 */
	void initIpcFlags(const mcu::IpcFlag& flagRsdoReceived,
			const mcu::IpcFlag& flagTsdoReady)
	{
		RSDO_RECEIVED = flagRsdoReceived;
		TSDO_READY = flagTsdoReady;
	}

	/**
	 * @brief Saves RSDO messages and generates IPC signal. Used by McoServer's ISR as callback.
	 * @param rawMsg - RSDO message raw data
	 * @return (none)
	 */
	void processRsdo(uint64_t rawMsg)
	{
		EMB_STATIC_ASSERT(Mode != emb::MODE_SLAVE);

		if (!mcu::isLocalIpcFlagSet(RSDO_RECEIVED.local))
		{
			*s_rsdoData = CobSdo(rawMsg);
			mcu::setLocalIpcFlag(RSDO_RECEIVED.local);
		}
		else
		{
			Syslog::setWarning(Warning::CAN_BUS_WARNING);
			Syslog::addMessage(Syslog::SDO_REQUEST_LOST);
		}
	}

	/**
	 * @brief Returns reference to TSDO data.
	 * @param (none)
	 * @return Reference to TSDO data.
	 */
	static const CobSdo& tsdoData() { return *s_tsdoData; }

	/**
	 * @brief Checks if there is new RSDO message and processes it. Used by McoServer's run().
	 * @param (none)
	 * @return (none)
	 */
	void processRequest()
	{
		if (!mcu::isIpcFlagSet(RSDO_RECEIVED, Ipc)) return;

		_processRequest(*s_rsdoData, *s_tsdoData);

		mcu::resetIpcFlag(RSDO_RECEIVED, Ipc);
	}

private:
	/**
	 * @brief Processes RSDO and creates TSDO.
	 * @param rsdo - reference to CobSdo structure where SDO request is located
	 * @param tsdo - reference to CobSdo structure where SDO response should be placed
	 * @return (none)
	 */
	void _processRequest(const CobSdo& rsdo, CobSdo& tsdo)
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
			mcu::setLocalIpcFlag(TSDO_READY.local);
			break;

		case OD_ACCESS_FAIL:
			return;

		case OD_ACCESS_NO_ACCESS:
			return;
		}
	}
};


template <mcu::CanModule Module, mcu::IpcMode Ipc, emb::MasterSlaveMode Mode>
CobSdo* SdoService<Module, Ipc, Mode>::s_rsdoData = static_cast<CobSdo*>(NULL);
template <mcu::CanModule Module, mcu::IpcMode Ipc, emb::MasterSlaveMode Mode>
CobSdo* SdoService<Module, Ipc, Mode>::s_tsdoData = static_cast<CobSdo*>(NULL);


/// @}
} // namespace microcanopen


