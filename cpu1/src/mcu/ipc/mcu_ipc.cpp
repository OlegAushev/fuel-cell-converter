/**
 * @file
 * @ingroup mcu mcu_ipc
 */


#include "mcu_ipc.h"


#if (defined(DUALCORE) && defined(CPU1))
extern const mcu::LocalIpcFlag CPU1_PERIPHERY_CONFIGURED(31);

extern const mcu::RemoteIpcFlag CPU2_BOOTED(31);
extern const mcu::RemoteIpcFlag CPU2_PERIPHERY_CONFIGURED(30);
#endif


#if (defined(DUALCORE) && defined(CPU2))
extern const mcu::RemoteIpcFlag CPU1_PERIPHERY_CONFIGURED(31);

extern const mcu::LocalIpcFlag CPU2_BOOTED(31);
extern const mcu::LocalIpcFlag CPU2_PERIPHERY_CONFIGURED(30);
#endif


#if (!defined(DUALCORE) && defined(CPU1))
extern const mcu::LocalIpcFlag CPU1_PERIPHERY_CONFIGURED(31);
#endif


