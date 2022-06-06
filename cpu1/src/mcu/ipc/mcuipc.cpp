/**
 * @file
 * @ingroup mcu mcu_ipc
 */


#include "mcuipc.h"


#if (defined(DUALCORE) && defined(CPU1))
extern const mcu::LocalIpcSignal CPU1_PERIPHERY_CONFIGURED(31);

extern const mcu::RemoteIpcSignal CPU2_BOOTED(31);
extern const mcu::RemoteIpcSignal CPU2_PERIPHERY_CONFIGURED(30);
#endif

#if (defined(DUALCORE) && defined(CPU2))
extern const mcu::RemoteIpcSignal CPU1_PERIPHERY_CONFIGURED(31);

extern const mcu::LocalIpcSignal CPU2_BOOTED(31);
extern const mcu::LocalIpcSignal CPU2_PERIPHERY_CONFIGURED(30);
#endif

#if (!defined(DUALCORE) && defined(CPU1))
extern const mcu::LocalIpcSignal CPU1_PERIPHERY_CONFIGURED(31);
#endif


