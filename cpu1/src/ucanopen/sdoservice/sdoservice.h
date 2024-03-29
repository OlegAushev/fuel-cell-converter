/**
 * @defgroup ucanopen_sdo_service SDO Service
 * @ingroup ucanopen
 *
 * @file
 * @ingroup ucanopen ucanopen_sdo_service
 */


#pragma once


#include "driverlib.h"
#include "device.h"

#include <string.h>
#include <algorithm>
#include "../ucanopen_def.h"
#include "../objectdictionary/objectdictionary.h"
#include "emb/emb_common.h"
#include "emb/emb_algorithm.h"
#include "mcu/can/mcu_can.h"
#include "mcu/ipc/mcu_ipc.h"

// APP-SPECIFIC headers
#include "sys/syslog/syslog.h"


namespace ucanopen {
/// @addtogroup ucanopen_sdo_service
/// @{


extern CobSdo can1RsdoDataShared;
extern CobSdo can2RsdoDataShared;
extern CobSdo can1RsdoDataNonShared;
extern CobSdo can2RsdoDataNonShared;
extern CobSdo can1TsdoDataShared;
extern CobSdo can2TsdoDataShared;
extern CobSdo can1TsdoDataNonShared;
extern CobSdo can2TsdoDataNonShared;


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
		case UCANOPEN_CAN1:
			switch (Ipc)
			{
			case mcu::IPC_MODE_SINGLECORE:
				s_rsdoData = &can1RsdoDataNonShared;
				s_tsdoData = &can1TsdoDataNonShared;
				break;
			case mcu::IPC_MODE_DUALCORE:
				s_rsdoData = &can1RsdoDataShared;
				s_tsdoData = &can1TsdoDataShared;
				break;
			}
			break;
		case UCANOPEN_CAN2:
			switch (Ipc)
			{
			case mcu::IPC_MODE_SINGLECORE:
				s_rsdoData = &can2RsdoDataNonShared;
				s_tsdoData = &can2TsdoDataNonShared;
				break;
			case mcu::IPC_MODE_DUALCORE:
				s_rsdoData = &can2RsdoDataShared;
				s_tsdoData = &can2TsdoDataShared;
				break;
			}
			break;
		}
	}

	/**
	 * @ingroup ucanopen_app_spec
	 * @brief Configures service on server that processes requests.
	 */
	SdoService(fuelcell::Converter* converter)
	{
		switch (Module)
		{
		case UCANOPEN_CAN1:
			switch (Ipc)
			{
			case mcu::IPC_MODE_SINGLECORE:
				s_rsdoData = &can1RsdoDataNonShared;
				s_tsdoData = &can1TsdoDataNonShared;
				break;
			case mcu::IPC_MODE_DUALCORE:
				s_rsdoData = &can1RsdoDataShared;
				s_tsdoData = &can1TsdoDataShared;
				break;
			}
			break;
		case UCANOPEN_CAN2:
			switch (Ipc)
			{
			case mcu::IPC_MODE_SINGLECORE:
				s_rsdoData = &can2RsdoDataNonShared;
				s_tsdoData = &can2TsdoDataNonShared;
				break;
			case mcu::IPC_MODE_DUALCORE:
				s_rsdoData = &can2RsdoDataShared;
				s_tsdoData = &can2TsdoDataShared;
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

			if (OBJECT_DICTIONARY[i].hasReadAccess())
			{
				assert((OBJECT_DICTIONARY[i].value.readAccessFunc != OD_NO_INDIRECT_READ_ACCESS)
						|| (OBJECT_DICTIONARY[i].value.dataPtr != OD_NO_DIRECT_ACCESS));
			}
			else
			{
				assert(OBJECT_DICTIONARY[i].value.readAccessFunc == OD_NO_INDIRECT_READ_ACCESS
						&& (OBJECT_DICTIONARY[i].value.dataPtr == OD_NO_DIRECT_ACCESS));
			}

			if (OBJECT_DICTIONARY[i].hasWriteAccess())
			{
				assert(OBJECT_DICTIONARY[i].value.writeAccessFunc != OD_NO_INDIRECT_WRITE_ACCESS
						|| (OBJECT_DICTIONARY[i].value.dataPtr != OD_NO_DIRECT_ACCESS));
			}
			else
			{
				assert(OBJECT_DICTIONARY[i].value.writeAccessFunc == OD_NO_INDIRECT_WRITE_ACCESS
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
	 * @brief Saves RSDO messages and generates IPC signal. Used by Server's ISR as callback.
	 * @param rawMsg - RSDO message raw data
	 * @return (none)
	 */
	void processRsdo(uint64_t rawMsg)
	{
		assert(Mode != emb::MODE_SLAVE);

		if (!mcu::isLocalIpcFlagSet(RSDO_RECEIVED.local))
		{
			*s_rsdoData = CobSdo(rawMsg);
			mcu::setLocalIpcFlag(RSDO_RECEIVED.local);
		}
		else
		{
			Syslog::addMessage(sys::Message::SDO_REQUEST_LOST);
		}
	}

	/**
	 * @brief Returns reference to TSDO data.
	 * @param (none)
	 * @return Reference to TSDO data.
	 */
	static const CobSdo& tsdoData() { return *s_tsdoData; }

	/**
	 * @brief Checks if there is new RSDO message and processes it. Used by Server's run().
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
			if ((odEntry->value.dataPtr != OD_NO_DIRECT_ACCESS)
					&& odEntry->hasReadAccess())
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
			if ((odEntry->value.dataPtr != OD_NO_DIRECT_ACCESS)
					&& odEntry->hasWriteAccess())
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
				tsdo.cs = SDO_SCS_READ;		// read/upload response
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
} // namespace ucanopen


