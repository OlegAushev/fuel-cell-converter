/**
 * @file
 * @ingroup ucanopen ucanopen_sdo_service
 */


#include "sdoservice.h"


namespace ucanopen {


#ifdef DUALCORE
CobSdo can1RsdoDataShared __attribute__((section("SHARED_UCANOPEN_CAN1_RSDO_DATA"), retain));
CobSdo can2RsdoDataShared __attribute__((section("SHARED_UCANOPEN_CAN2_RSDO_DATA"), retain));
CobSdo can1RsdoDataNonShared;
CobSdo can2RsdoDataNonShared;
#else
CobSdo can1RsdoDataShared;
CobSdo can2RsdoDataShared;
CobSdo can1RsdoDataNonShared;
CobSdo can2RsdoDataNonShared;
#endif


#ifdef DUALCORE
CobSdo can1TsdoDataShared __attribute__((section("SHARED_UCANOPEN_CAN1_TSDO_DATA"), retain));
CobSdo can2TsdoDataShared __attribute__((section("SHARED_UCANOPEN_CAN2_TSDO_DATA"), retain));
CobSdo can1TsdoDataNonShared;
CobSdo can2TsdoDataNonShared;
#else
CobSdo can1TsdoDataShared;
CobSdo can2TsdoDataShared;
CobSdo can1TsdoDataNonShared;
CobSdo can2TsdoDataNonShared;
#endif


} // namespace ucanopen


