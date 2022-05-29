///
#include "emb_profiler.h"


namespace emb {

uint64_t timeNowFuncNone_()
{
	return 0;
}

uint64_t (*DurationLogger::m_timeNowFunc)() = timeNowFuncNone_;

uint64_t (*DurationLoggerAsyncPrint::m_timeNowFunc)() = timeNowFuncNone_;
DurationLoggerAsyncPrint::DurationData DurationLoggerAsyncPrint::m_durationsUs[CAPACITY];

}


