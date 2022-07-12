/**
 * @file
 * @ingroup syslog
 */


#include "syslog.h"


#ifdef DUALCORE
#pragma DATA_SECTION("SHARED_SYSLOG_MESSAGES")
emb::Queue<Syslog::Message, 32> Syslog::m_messages;
#else
emb::Queue<Syslog::Message, 32> Syslog::m_messages;
#endif


#ifdef DUALCORE
#pragma DATA_SECTION("SHARED_SYSLOG_MESSAGE_CPU2")
Syslog::Message Syslog::m_cpu2Message = Syslog::NO_MESSAGE;
#endif


#ifdef DUALCORE
#pragma DATA_SECTION("SHARED_SYSLOG_DATA_CPU1")
Syslog::FaultData Syslog::m_cpu1FaultData;
#pragma DATA_SECTION("SHARED_SYSLOG_DATA_CPU2")
Syslog::FaultData Syslog::m_cpu2FaultData;
#else
Syslog::FaultData Syslog::m_cpu1FaultData;
#endif


Syslog::FaultData* Syslog::m_thisCpuFaultData;


// IPC signals
mcu::IpcFlag Syslog::RESET_FAULTS_AND_WARNINGS;
mcu::IpcFlag Syslog::ADD_MESSAGE;
mcu::IpcFlag Syslog::POP_MESSAGE;

