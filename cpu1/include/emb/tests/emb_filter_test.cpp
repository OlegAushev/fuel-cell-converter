///
#include "emb_test.h"


void EmbTest::FilterTest()
{
	/* MovingAvgFilter */
	emb::MovingAvgFilter<int, 5> mvAvgFilterI16;
	for (size_t i = 1; i <= 5; ++i)
	{
		mvAvgFilterI16.push(i);
	}
	EMB_ASSERT_EQUAL(mvAvgFilterI16.output(), 3);
	mvAvgFilterI16.reset();
	EMB_ASSERT_EQUAL(mvAvgFilterI16.output(), 0);
	EMB_ASSERT_EQUAL(mvAvgFilterI16.size(), 5);
	mvAvgFilterI16.resize(3);
	EMB_ASSERT_EQUAL(mvAvgFilterI16.size(), 3);
	mvAvgFilterI16.resize(6);
	EMB_ASSERT_EQUAL(mvAvgFilterI16.size(), 5);

	emb::Array<float, 10> filterArray;
	emb::MovingAvgFilter<float, 10> mvAvgFilterF32(filterArray);
	for (size_t i = 0; i < mvAvgFilterF32.size(); ++i)
	{
		mvAvgFilterF32.push(emb::PI * (1 + (i % 2)));
	}
	EMB_ASSERT_EQUAL(mvAvgFilterF32.output(), emb::PI * 1.5f);
	mvAvgFilterF32.setOutput(emb::PI);
	EMB_ASSERT_EQUAL(mvAvgFilterF32.output(), emb::PI);

	/* MedianFilter */
	emb::MedianFilter<int, 5> medFilter;
	medFilter.push(-10);
	EMB_ASSERT_EQUAL(medFilter.output(), 0);
	medFilter.push(10);
	EMB_ASSERT_EQUAL(medFilter.output(), 0);
	medFilter.push(100);
	medFilter.push(100);
	medFilter.push(5);
	EMB_ASSERT_EQUAL(medFilter.output(), 10);
	medFilter.push(20);
	EMB_ASSERT_EQUAL(medFilter.output(), 20);
	medFilter.push(105);
	EMB_ASSERT_EQUAL(medFilter.output(), 100);
	medFilter.reset();
	EMB_ASSERT_EQUAL(medFilter.output(), 0);
	medFilter.setOutput(50);
	EMB_ASSERT_EQUAL(medFilter.output(), 50);

	/* ExponentialMedianFilter */
	emb::ExponentialMedianFilter<float, 3> expMedFilter;
	expMedFilter.setSmoothFactor(0.5);
	expMedFilter.push(16);
	EMB_ASSERT_EQUAL(expMedFilter.output(), 0);
	expMedFilter.push(8);
	EMB_ASSERT_EQUAL(expMedFilter.output(), 4);
	expMedFilter.push(32);
	EMB_ASSERT_EQUAL(expMedFilter.output(), 10);
	expMedFilter.push(8);
	EMB_ASSERT_EQUAL(expMedFilter.output(), 9);
	expMedFilter.setSmoothFactor(1);
	expMedFilter.push(19);
	EMB_ASSERT_EQUAL(expMedFilter.output(), 19);
	expMedFilter.setOutput(10);
	EMB_ASSERT_EQUAL(expMedFilter.output(), 10);
	expMedFilter.push(5);
	expMedFilter.push(0);
	EMB_ASSERT_EQUAL(expMedFilter.output(), 5);
	expMedFilter.reset();
	EMB_ASSERT_EQUAL(expMedFilter.output(), 0);
}


