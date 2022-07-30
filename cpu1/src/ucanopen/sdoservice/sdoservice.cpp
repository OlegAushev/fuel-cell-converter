/**
 * @file
 * @ingroup ucanopen ucanopen_sdo_service
 */


#include "sdoservice.h"


namespace ucanopen {


#ifdef DUALCORE
CobSdo mcoCan1RsdoDataShared __attribute__((section("SHARED_UCANOPEN_CAN1_RSDO_DATA"), retain));
CobSdo mcoCan2RsdoDataShared __attribute__((section("SHARED_UCANOPEN_CAN2_RSDO_DATA"), retain));
CobSdo mcoCan1RsdoDataNonShared;
CobSdo mcoCan2RsdoDataNonShared;
#else
CobSdo mcoCan1RsdoDataShared;
CobSdo mcoCan2RsdoDataShared;
CobSdo mcoCan1RsdoDataNonShared;
CobSdo mcoCan2RsdoDataNonShared;
#endif


#ifdef DUALCORE
CobSdo mcoCan1TsdoDataShared __attribute__((section("SHARED_UCANOPEN_CAN1_TSDO_DATA"), retain));
CobSdo mcoCan2TsdoDataShared __attribute__((section("SHARED_UCANOPEN_CAN2_TSDO_DATA"), retain));
CobSdo mcoCan1TsdoDataNonShared;
CobSdo mcoCan2TsdoDataNonShared;
#else
CobSdo mcoCan1TsdoDataShared;
CobSdo mcoCan2TsdoDataShared;
CobSdo mcoCan1TsdoDataNonShared;
CobSdo mcoCan2TsdoDataNonShared;
#endif


} // namespace ucanopen


