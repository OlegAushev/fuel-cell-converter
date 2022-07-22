///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <cstdio>
#include <assert.h>


#ifdef TEST_BUILD
void RUN_TESTS();
#endif


namespace emb {


/**
 * @brief
 */
class TestRunner
{
private:
	static int nAssertsTotal;
	static int nAssertsFailedInTest;
	static int nAssertsFailedTotal;
	static int nTestsTotal;
	static int nTestsFailed;
	static int nTestsPassed;

public:
	template <typename T, typename U>
	static void assertEqual(const T& t, const U& u, const char* hint)
	{
		++nAssertsTotal;
		if (!(t == u))
		{
			++nAssertsFailedInTest;
			++nAssertsFailedTotal;
			printf(hint);
			printf("\n");
		}
	}

	static void assertTrue(bool b, const char* hint)
	{
		assertEqual(b, true, hint);
	}


	template <class TestFunc>
	static void runTest(TestFunc testFunc, const char* test_name)
	{
		++nTestsTotal;
		nAssertsFailedInTest = 0;
		testFunc();
		if (nAssertsFailedInTest == 0)
		{
			++nTestsPassed;
			printf("[ PASSED ] ");
			printf(test_name);
			printf("\n");
		}
		else
		{
			++nTestsFailed;
			printf("[ FAILED ] ");
			printf(test_name);
			printf("\n");
		}
	}

	static void printResult()
	{
		printf("\n");
		printf("Asserts: %d failed, %d passed\n", nAssertsFailedTotal, nAssertsTotal - nAssertsFailedTotal);
		printf("Tests:   %d failed, %d passed\n", nTestsFailed, nTestsPassed);


		if (nTestsFailed == 0)
		{
			printf("OK\n");
		}
		else
		{
			printf("FAIL\n");
		}
	}

	static bool passed()
	{
		if (nTestsFailed == 0)
		{
			return true;
		}
		return false;
	}
};


} // namespace emb


/**
 * @brief
 */
#define EMB_RUN_TEST(func) emb::TestRunner::runTest(func, #func)


/**
 * @brief
 */
#ifdef TEST_BUILD
#define EMB_ASSERT_EQUAL(x, y) \
{ \
	const char* hint = "[  WARN  ] Assertion failed: " #x " != " #y ", file: " __FILE__ ", line: " _STR(__LINE__); \
	emb::TestRunner::assertEqual(x, y, hint); \
}
#else
#define EMB_ASSERT_EQUAL(x, y) ((void)0)
#endif


/**
 * @brief
 */
#ifdef TEST_BUILD
#define EMB_ASSERT_TRUE(x) \
{ \
	const char* hint = "[  WARN  ] Assertion failed: " #x " is false, file: " __FILE__ ", line: " _STR(__LINE__); \
	emb::TestRunner::assertTrue(x, hint); \
}
#else
#define EMB_ASSERT_TRUE(x) ((void)0)
#endif


