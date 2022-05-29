/**
 * @file
 * @ingroup mcu mcu_ipc
 */


#include "mcuipc.h"


#if (defined(DUALCORE) && defined(CPU1))
extern const mcu::LocalIpcSignal CPU1_PERIPHERY_CONFIGURED(31);
extern const mcu::LocalIpcSignal SYSLOG_RESET(3);
extern const mcu::LocalIpcSignal CAN_TSDO_READY(9);

extern const mcu::RemoteIpcSignal CPU2_BOOTED(31);
extern const mcu::RemoteIpcSignal CPU2_PERIPHERY_CONFIGURED(30);
extern const mcu::RemoteIpcSignal REPORT_SENT(29);
extern const mcu::RemoteIpcSignal SETUPMSG_RECEIVED(26);
extern const mcu::RemoteIpcSignal SYSLOG_POP_MESSAGE(25);
extern const mcu::RemoteIpcSignal SYSLOG_ADD_MESSAGE(24);

extern const mcu::RemoteIpcSignal CAN_FRAME_RECEIVED(0);
extern const mcu::RemoteIpcSignal CAN_RPDO1_RECEIVED(4);
extern const mcu::RemoteIpcSignal CAN_RPDO2_RECEIVED(5);
extern const mcu::RemoteIpcSignal CAN_RPDO3_RECEIVED(6);
extern const mcu::RemoteIpcSignal CAN_RPDO4_RECEIVED(7);
extern const mcu::RemoteIpcSignal CAN_RSDO_RECEIVED(8);
#endif

#if (defined(DUALCORE) && defined(CPU2))
extern const mcu::RemoteIpcSignal CPU1_PERIPHERY_CONFIGURED(31);
extern const mcu::RemoteIpcSignal SYSLOG_RESET(3);
extern const mcu::RemoteIpcSignal CAN_TSDO_READY(9);

extern const mcu::LocalIpcSignal CPU2_BOOTED(31);
extern const mcu::LocalIpcSignal CPU2_PERIPHERY_CONFIGURED(30);
extern const mcu::LocalIpcSignal REPORT_SENT(29);
extern const mcu::LocalIpcSignal SETUPMSG_RECEIVED(26);
extern const mcu::LocalIpcSignal SYSLOG_POP_MESSAGE(25);
extern const mcu::LocalIpcSignal SYSLOG_ADD_MESSAGE(24);

extern const mcu::LocalIpcSignal CAN_FRAME_RECEIVED(0);
extern const mcu::LocalIpcSignal CAN_RPDO1_RECEIVED(4);
extern const mcu::LocalIpcSignal CAN_RPDO2_RECEIVED(5);
extern const mcu::LocalIpcSignal CAN_RPDO3_RECEIVED(6);
extern const mcu::LocalIpcSignal CAN_RPDO4_RECEIVED(7);
extern const mcu::LocalIpcSignal CAN_RSDO_RECEIVED(8);

#endif

#if (!defined(DUALCORE) && defined(CPU1))
extern const mcu::LocalIpcSignal CPU1_PERIPHERY_CONFIGURED(31);
extern const mcu::LocalIpcSignal REPORT_SENT(29);
extern const mcu::LocalIpcSignal SETUPMSG_RECEIVED(26);

extern const mcu::LocalIpcSignal CAN_FRAME_RECEIVED(0);
extern const mcu::LocalIpcSignal CAN_RPDO1_RECEIVED(4);
extern const mcu::LocalIpcSignal CAN_RPDO2_RECEIVED(5);
extern const mcu::LocalIpcSignal CAN_RPDO3_RECEIVED(6);
extern const mcu::LocalIpcSignal CAN_RPDO4_RECEIVED(7);
extern const mcu::LocalIpcSignal CAN_RSDO_RECEIVED(8);
extern const mcu::LocalIpcSignal CAN_TSDO_READY(9);
#endif


