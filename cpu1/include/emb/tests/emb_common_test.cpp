///
#include "emb_test.h"


class SingletonTest : public emb::c28x::Singleton<SingletonTest>
{
public:
	SingletonTest() : Singleton(this) {}
};


void EmbTest::CommonTest()
{
	SingletonTest singletonObject;
	SingletonTest* pSingletonObject = &singletonObject;
	EMB_ASSERT_EQUAL(SingletonTest::instance(), pSingletonObject);
}


