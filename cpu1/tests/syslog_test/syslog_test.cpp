///
#include "syslog_test.h"


void SyslogTest::Test()
{
	EMB_ASSERT_EQUAL(Syslog::faults(), 0);
	EMB_ASSERT_EQUAL(Syslog::warnings(), 0);

	Syslog::setFault(Fault::DC_UNDERVOLTAGE);
	EMB_ASSERT_TRUE(Syslog::hasFault(Fault::DC_UNDERVOLTAGE));

	Syslog::setWarning(Warning::CAN_BUS_WARNING);
	EMB_ASSERT_TRUE(Syslog::hasWarning(Warning::CAN_BUS_WARNING));

	EMB_ASSERT_EQUAL(Syslog::faults(), 1UL << Fault::DC_UNDERVOLTAGE);
	EMB_ASSERT_EQUAL(Syslog::warnings(), 1UL << Warning::CAN_BUS_WARNING);

	Syslog::setFault(Fault::EMERGENCY_STOP);
	EMB_ASSERT_EQUAL(Syslog::faults(), (1UL << Fault::DC_UNDERVOLTAGE) | (1UL << Fault::EMERGENCY_STOP));

	Syslog::resetFault(Fault::DC_UNDERVOLTAGE);
	Syslog::resetWarning(Warning::CAN_BUS_WARNING);

	EMB_ASSERT_EQUAL(Syslog::faults(), 1UL << Fault::EMERGENCY_STOP);
	EMB_ASSERT_EQUAL(Syslog::warnings(), 0);

	Syslog::resetFault(Fault::EMERGENCY_STOP);
	EMB_ASSERT_EQUAL(Syslog::faults(), 0);

	Syslog::resetFaultsAndWarnings();
	Syslog::disableFault(Fault::CAN_BUS_ERROR);
	Syslog::setFault(Fault::CAN_BUS_ERROR);
	EMB_ASSERT_EQUAL(Syslog::faults(), 0);

	Syslog::enableFault(Fault::CAN_BUS_ERROR);
	EMB_ASSERT_EQUAL(Syslog::faults(), 0);

	Syslog::setFault(Fault::CAN_BUS_ERROR);
	EMB_ASSERT_EQUAL(Syslog::faults(), (1UL << Fault::CAN_BUS_ERROR));

	Syslog::resetFaultsAndWarnings();
	EMB_ASSERT_EQUAL(Syslog::faults(), 0);

	// test critical errors
	Syslog::setFault(Fault::DRIVER_ALL_FLT);
	Syslog::setFault(Fault::CONNECTION_LOST);
	EMB_ASSERT_TRUE(Syslog::criticalFaultDetected());

	Syslog::resetFaultsAndWarnings();
	EMB_ASSERT_EQUAL(Syslog::faults(), (1UL << Fault::DRIVER_ALL_FLT));
	EMB_ASSERT_TRUE(Syslog::criticalFaultDetected());

	Syslog::resetFault(Fault::DRIVER_ALL_FLT);
	Syslog::setFault(Fault::CASE_OVERTEMP);
	EMB_ASSERT_TRUE(!Syslog::criticalFaultDetected());
	Syslog::resetFaultsAndWarnings();
	EMB_ASSERT_EQUAL(Syslog::faults(), 0);
}


