/**
 * @file
 * @ingroup microcanopen mco_sdo_service
 */


#include "sdoservice.h"


namespace microcanopen {

#ifdef DUALCORE
#pragma DATA_SECTION("SHARED_MCO_CAN1_RSDO_DATA")
CobSdo SdoService<MCO_CAN1>::m_rsdoData;
#pragma DATA_SECTION("SHARED_MCO_CAN2_RSDO_DATA")
CobSdo SdoService<MCO_CAN2>::m_rsdoData;
#else
CobSdo SdoService<MCO_CAN1>::m_rsdoData;
CobSdo SdoService<MCO_CAN2>::m_rsdoData;
#endif

#ifdef DUALCORE
#pragma DATA_SECTION("SHARED_MCO_CAN1_TSDO_DATA")
CobSdo SdoService<MCO_CAN1>::m_tsdoData;
#pragma DATA_SECTION("SHARED_MCO_CAN2_TSDO_DATA")
CobSdo SdoService<MCO_CAN2>::m_tsdoData;
#else
CobSdo SdoService<MCO_CAN1>::m_tsdoData;
CobSdo SdoService<MCO_CAN2>::m_tsdoData;
#endif


///
///
///
template <mcu::CanModule Module>
void SdoService<Module>::processRsdo(uint64_t rawMsg)
{
#if (!defined(DUALCORE) || defined(CPU2))
	if (!mcu::localIpcSignalSent(RSDO_RECEIVED->local))
	{
		m_rsdoData = CobSdo(rawMsg);
		mcu::sendIpcSignal(RSDO_RECEIVED->local);
	}
	else
	{
		Syslog::setWarning(Warning::CAN_BUS_WARNING);
		Syslog::addMessage(Syslog::SDO_REQUEST_LOST);
	}
#else
	while (true) {} // CPU1 in DUALCORE build must not get here
#endif
}
// Explicit instantiation
template void SdoService<mcu::CANA>::processRsdo(uint64_t rawMsg);
template void SdoService<mcu::CANB>::processRsdo(uint64_t rawMsg);

#ifdef CPU1
///
///
///
template <mcu::CanModule Module>
void SdoService<Module>::processRequest()
{
#ifdef DUALCORE
	if (!mcu::remoteIpcSignalSent(RSDO_RECEIVED->remote)) return;
#else
	if (!mcu::localIpcSignalSent(RSDO_RECEIVED->local)) return;
#endif
	_processRequest(m_rsdoData, m_tsdoData);
#ifdef DUALCORE
	mcu::acknowledgeRemoteIpcSignal(RSDO_RECEIVED->remote);
#else
	mcu::revokeLocalIpcSignal(RSDO_RECEIVED->local);
#endif
}
// Explicit instantiation
template void SdoService<mcu::CANA>::processRequest();
template void SdoService<mcu::CANB>::processRequest();

///
///
///
template <mcu::CanModule Module>
void SdoService<Module>::_processRequest(const CobSdo& rsdo, CobSdo& tsdo)
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
		mcu::sendIpcSignal(TSDO_READY->local);
		break;

	case OD_ACCESS_FAIL:
		return;

	case OD_ACCESS_NO_ACCESS:
		return;
	}
}
#endif


} // namespace microcanopen {


