///
#include "sdoservice_test.h"


namespace microcanopen {

void SdoServiceTest::MessageProcessingTest()
{
	//SetupManager::SYSTEM_CONFIG = SetupManager::DEFAULT_CONFIG;

	SdoService<mcu::CANA> sdoService(NULL);

	const ODEntry* odEntry1 = emb::binary_find(OBJECT_DICTIONARY, OBJECT_DICTIONARY_END, ODEntryKeyAux(0x1008, 0x00));
	EMB_ASSERT_TRUE(strcmp(odEntry1->value.name, "DEVICE_NAME") == 0);

	const ODEntry* odEntry2 = emb::binary_find(OBJECT_DICTIONARY, OBJECT_DICTIONARY_END, ODEntryKeyAux(0x5000, 0x00));
	EMB_ASSERT_TRUE(strcmp(odEntry2->value.name, "UPTIME") == 0);

	const ODEntry* odEntry3 = emb::binary_find(OBJECT_DICTIONARY, OBJECT_DICTIONARY_END, ODEntryKeyAux(0x2001, 0x01));
	EMB_ASSERT_TRUE(strcmp(odEntry3->value.name, "POWERDOWN") == 0);
	EMB_ASSERT_EQUAL(odEntry3->value.dataType, OD_TASK);

	CobSdo rsdo, tsdo;
	rsdo.index = 0x5FFF;
	rsdo.subindex = 0x00;
	rsdo.cs = SDO_CCS_READ;
	sdoService._processRequest(rsdo, tsdo);
	EMB_ASSERT_EQUAL(tsdo.data.u32, Syslog::SOFTWARE_VERSION);

/*
	rsdo.cs = SDO_CCS_READ;
	rsdo.index = 0x2101;
	rsdo.subindex = 0x09;
	sdoService.processSdoRequest(rsdo, tsdo);
	EMB_ASSERT_EQUAL(tsdo.data.f32, SetupManager::DEFAULT_CONFIG.ACIM_MODEL_CONFIG.iSMotorMax);

	rsdo.cs = SDO_CCS_WRITE;
	rsdo.index = 0x2101;
	rsdo.subindex = 0x0E;
	rsdo.data.f32 = 1234.56;
	sdoService.processSdoRequest(rsdo, tsdo);
	EMB_ASSERT_EQUAL(SetupManager::SYSTEM_CONFIG.ACIM_MODEL_CONFIG.speedRpmMax, 1234.56f);
*/
}

} // namespace microcanopen


