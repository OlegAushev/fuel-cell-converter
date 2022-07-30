/**
 * @file
 * @ingroup ucanopen ucanopen_rpdo_service
 */


#include "rpdoservice.h"


namespace ucanopen {


/// Common IPC RPDO data storage for all servers
#ifdef DUALCORE
ProcessedRpdoData rpdoProcessedDataShared __attribute__((section("SHARED_UCANOPEN_RPDO_DATA"), retain));
ProcessedRpdoData rpdoProcessedDataNonShared;
#else
ProcessedRpdoData rpdoProcessedDataShared;
ProcessedRpdoData rpdoProcessedDataNonShared;
#endif


} // namespace ucanopen


