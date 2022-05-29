///
#include "rpdoservice_test.h"


namespace microcanopen {

void RpdoServiceTest::MessageProcessingTest()
{
	EMB_ASSERT_EQUAL(sizeof(CobRpdo1), 4);
	EMB_ASSERT_EQUAL(sizeof(CobRpdo2), 4);
}

} // namespace microcanopen
