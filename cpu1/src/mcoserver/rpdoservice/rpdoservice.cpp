/**
 * @file
 * @ingroup microcanopen mco_rpdo_service
 */


#include "rpdoservice.h"


namespace microcanopen {


/// Common IPC data storage for all MCO servers.
#ifdef DUALCORE
#pragma DATA_SECTION("SHARED_MCO_RPDO_DATA")
ProcessedRpdoData rpdoProcessedDataShared;
ProcessedRpdoData rpdoProcessedDataNonShared;
#else
ProcessedRpdoData rpdoProcessedDataShared;
ProcessedRpdoData rpdoProcessedDataNonShared;
#endif


} // namespace microcanopen


