/**
 * @file
 * @ingroup syslog
 */


#include "syslog.h"


#ifdef DUALCORE
emb::Queue<Syslog::Message, 32> Syslog::m_messages __attribute__((section("SHARED_SYSLOG_MESSAGES"), retain));
#else
emb::Queue<Syslog::Message, 32> Syslog::m_messages;
#endif


#ifdef DUALCORE
Syslog::Message Syslog::m_cpu2Message __attribute__((section("SHARED_SYSLOG_MESSAGE_CPU2"), retain)) = Syslog::NO_MESSAGE;
#endif


#ifdef DUALCORE
Syslog::FaultData Syslog::m_cpu1FaultData __attribute__((section("SHARED_SYSLOG_DATA_CPU1"), retain));
Syslog::FaultData Syslog::m_cpu2FaultData __attribute__((section("SHARED_SYSLOG_DATA_CPU2"), retain));
#else
Syslog::FaultData Syslog::m_cpu1FaultData;
#endif


Syslog::FaultData* Syslog::m_thisCpuFaultData;


// IPC signals
mcu::IpcFlag Syslog::RESET_FAULTS_AND_WARNINGS;
mcu::IpcFlag Syslog::ADD_MESSAGE;
mcu::IpcFlag Syslog::POP_MESSAGE;

