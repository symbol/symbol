#include "ThreadPoolTestUtils.h"
#include "catapult/thread/IoServiceThreadPool.h"

namespace catapult { namespace test {

	std::unique_ptr<thread::IoServiceThreadPool> CreateStartedIoServiceThreadPool(const char* name) {
		return CreateStartedIoServiceThreadPool(GetNumDefaultPoolThreads(), name);
	}

	std::unique_ptr<thread::IoServiceThreadPool> CreateStartedIoServiceThreadPool(uint32_t numThreads, const char* name) {
		auto pPool = thread::CreateIoServiceThreadPool(numThreads, name);
		pPool->start();
		return pPool;
	}
}}
