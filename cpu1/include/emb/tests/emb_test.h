///
#pragma once


#include <algorithm>

#include "emb/emb_testrunner/emb_testrunner.h"
#include "emb/emb_common.h"
#include "emb/emb_algorithm.h"
#include "emb/emb_array.h"
#include "emb/emb_queue.h"
#include "emb/emb_circularbuffer.h"
#include "emb/emb_math.h"
#include "emb/emb_filter.h"
#include "emb/emb_stack.h"
#include "emb/emb_bitset.h"


class EmbTest
{
public:
	static void CommonTest();
	static void MathTest();
	static void AlgorithmTest();
	static void ArrayTest();
	static void QueueTest();
	static void CircularBufferTest();
	static void FilterTest();
	static void StackTest();
	static void BitsetTest();
};


