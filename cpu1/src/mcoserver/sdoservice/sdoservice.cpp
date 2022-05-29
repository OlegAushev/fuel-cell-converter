/**
 * @file
 * @ingroup microcanopen mco_sdo_service
 */


#include "sdoservice.h"


namespace microcanopen {

#ifdef DUALCORE
#pragma DATA_SECTION("CANRSDODATA")
SdoService::SdoData SdoService::rsdoData;
#else
SdoService::SdoData SdoService::rsdoData;
#endif

#ifdef DUALCORE
#pragma DATA_SECTION("CANTSDODATA")
SdoService::SdoData SdoService::tsdoData;
#else
SdoService::SdoData SdoService::tsdoData;
#endif



























} // namespace microcanopen {




