/**
 * @file
 * @ingroup microcanopen mco_rpdo_service
 */


#include "rpdoservice.h"


namespace microcanopen {


/// Common IPC data storage for all MCO servers.
#ifdef DUALCORE
ProcessedRpdoData rpdoProcessedDataShared __attribute__((section("SHARED_MCO_RPDO_DATA"), retain));
ProcessedRpdoData rpdoProcessedDataNonShared;
#else
ProcessedRpdoData rpdoProcessedDataShared;
ProcessedRpdoData rpdoProcessedDataNonShared;
#endif


} // namespace microcanopen


