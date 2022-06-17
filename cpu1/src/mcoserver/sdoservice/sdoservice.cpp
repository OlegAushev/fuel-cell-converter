/**
 * @file
 * @ingroup microcanopen mco_sdo_service
 */


#include "sdoservice.h"


namespace microcanopen {


#ifdef DUALCORE
#pragma DATA_SECTION("SHARED_MCO_CAN1_RSDO_DATA")
CobSdo mcoCan1RsdoDataShared;
#pragma DATA_SECTION("SHARED_MCO_CAN2_RSDO_DATA")
CobSdo mcoCan2RsdoDataShared;
CobSdo mcoCan1RsdoDataNonShared;
CobSdo mcoCan2RsdoDataNonShared;
#else
CobSdo mcoCan1RsdoDataShared;
CobSdo mcoCan2RsdoDataShared;
CobSdo mcoCan1RsdoDataNonShared;
CobSdo mcoCan2RsdoDataNonShared;
#endif


#ifdef DUALCORE
#pragma DATA_SECTION("SHARED_MCO_CAN1_TSDO_DATA")
CobSdo mcoCan1TsdoDataShared;
#pragma DATA_SECTION("SHARED_MCO_CAN2_TSDO_DATA")
CobSdo mcoCan2TsdoDataShared;
CobSdo mcoCan1TsdoDataNonShared;
CobSdo mcoCan2TsdoDataNonShared;
#else
CobSdo mcoCan1TsdoDataShared;
CobSdo mcoCan2TsdoDataShared;
CobSdo mcoCan1TsdoDataNonShared;
CobSdo mcoCan2TsdoDataNonShared;
#endif


} // namespace microcanopen


