#include "ToolThreadUtils.h"
#include "catapult/thread/IoServiceThreadPool.h"

namespace catapult { namespace tools {

	std::shared_ptr<thread::IoServiceThreadPool> CreateStartedThreadPool(uint32_t numThreads) {
		auto pPool = thread::CreateIoServiceThreadPool(numThreads);
		pPool->start();
		return std::move(pPool);
	}
}}
