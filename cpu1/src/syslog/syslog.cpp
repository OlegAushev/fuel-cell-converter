/**
 * @file
 * @ingroup syslog
 */


#include "syslog.h"


#ifdef DUALCORE
#pragma DATA_SECTION("SYSLOGMESSAGES")
emb::Queue<SyslogMsg::SyslogMsg, 32> Syslog::m_messages;
#else
emb::Queue<SyslogMsg::SyslogMsg, 32> Syslog::m_messages;
#endif

#ifdef DUALCORE
#pragma DATA_SECTION("SYSLOGMESSAGECPU2")
SyslogMsg::SyslogMsg Syslog::m_cpu2Message = SyslogMsg::NO_MESSAGE;
#endif

#ifdef DUALCORE
#pragma DATA_SECTION("SYSLOGDATACPU1")
Syslog::FaultData Syslog::m_cpu1FaultData;
#pragma DATA_SECTION("SYSLOGDATACPU2")
Syslog::FaultData Syslog::m_cpu2FaultData;
#else
Syslog::FaultData Syslog::m_cpu1FaultData;
#endif

Syslog::FaultData* Syslog::m_thisCpuFaultData;


