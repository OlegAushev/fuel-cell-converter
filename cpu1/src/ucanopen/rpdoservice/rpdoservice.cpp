/**
 * @file
 * @ingroup ucanopen ucanopen_rpdo_service
 */


#include "rpdoservice.h"


namespace ucanopen {


/// Common IPC data storage for all MCO servers.
#ifdef DUALCORE
ProcessedRpdoData rpdoProcessedDataShared __attribute__((section("SHARED_UCANOPEN_RPDO_DATA"), retain));
ProcessedRpdoData rpdoProcessedDataNonShared;
#else
ProcessedRpdoData rpdoProcessedDataShared;
ProcessedRpdoData rpdoProcessedDataNonShared;
#endif


} // namespace ucanopen


