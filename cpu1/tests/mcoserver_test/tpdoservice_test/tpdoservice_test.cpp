///
#include "tpdoservice_test.h"


namespace ucanopen {

void TpdoServiceTest::MessageProcessingTest()
{
	EMB_ASSERT_EQUAL(sizeof(CobTpdo1), 4);
	EMB_ASSERT_EQUAL(sizeof(CobTpdo2), 4);
	EMB_ASSERT_EQUAL(sizeof(CobTpdo3), 4);
	EMB_ASSERT_EQUAL(sizeof(CobTpdo4), 4);
/*
	CobTpdo1 msgTpdo1;
	uint64_t rawMsgTpdo1 = 0;
	memcpy(&msgTpdo1, &rawMsgTpdo1, 4);
	msgTpdo1.statusRun = 1;
	msgTpdo1.statusWarning = 1;
	msgTpdo1.speed = 0xABCD;
	msgTpdo1.voltageDC = 0xEF;
	memcpy(&rawMsgTpdo1, &msgTpdo1, 4);

	EMB_ASSERT_EQUAL(rawMsgTpdo1, 0x00EF0000ABCD0005);

	uint16_t msgTpdo1Bytes[8];
	splitIntoC28xBytes<uint64_t>(msgTpdo1Bytes, rawMsgTpdo1);

	EMB_ASSERT_EQUAL(msgTpdo1Bytes[0], 0x05);
	EMB_ASSERT_EQUAL(msgTpdo1Bytes[1], 0x00);
	EMB_ASSERT_EQUAL(msgTpdo1Bytes[2], 0xCD);
	EMB_ASSERT_EQUAL(msgTpdo1Bytes[3], 0xAB);
	EMB_ASSERT_EQUAL(msgTpdo1Bytes[4], 0x00);
	EMB_ASSERT_EQUAL(msgTpdo1Bytes[5], 0x00);
	EMB_ASSERT_EQUAL(msgTpdo1Bytes[6], 0xEF);
	EMB_ASSERT_EQUAL(msgTpdo1Bytes[7], 0x00);

	rawMsgTpdo1 = 0;
	memcpy(&msgTpdo1, &rawMsgTpdo1, 4);
	mergeC28xBytes<uint64_t>(rawMsgTpdo1, msgTpdo1Bytes);

	EMB_ASSERT_EQUAL(rawMsgTpdo1, 0x00EF0000ABCD0005);

	memcpy(&msgTpdo1, &rawMsgTpdo1, 4);

	EMB_ASSERT_EQUAL(msgTpdo1.statusRun, 1);
	EMB_ASSERT_EQUAL(msgTpdo1.statusWarning, 1);
	EMB_ASSERT_EQUAL(msgTpdo1.speed, 0xABCD);
	EMB_ASSERT_EQUAL(msgTpdo1.voltageDC, 0xEF);
*/
}

} // namespace ucanopen
