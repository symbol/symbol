#pragma once
#include "catapult/thread/IoServiceThreadPool.h"
#include "tests/TestHarness.h"
#include <memory>
#include <stdint.h>

namespace catapult { namespace thread { class IoServiceThreadPool; } }

namespace catapult { namespace test {

	/// Creates an auto started threadpool with a default number of threads.
	std::unique_ptr<thread::IoServiceThreadPool> CreateStartedIoServiceThreadPool();

	/// Creates an auto started threadpool with \a numThreads threads.
	std::unique_ptr<thread::IoServiceThreadPool> CreateStartedIoServiceThreadPool(uint32_t numThreads);
}}
