#include "ThreadPoolTestUtils.h"
#include "catapult/thread/IoServiceThreadPool.h"

namespace catapult { namespace test {

	std::unique_ptr<thread::IoServiceThreadPool> CreateStartedIoServiceThreadPool() {
		return CreateStartedIoServiceThreadPool(GetNumDefaultPoolThreads());
	}

	std::unique_ptr<thread::IoServiceThreadPool> CreateStartedIoServiceThreadPool(uint32_t numThreads) {
		auto pPool = thread::CreateIoServiceThreadPool(numThreads);
		pPool->start();
		return pPool;
	}
}}
