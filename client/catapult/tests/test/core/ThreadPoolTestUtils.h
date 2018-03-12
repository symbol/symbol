#pragma once
#include "catapult/thread/IoServiceThreadPool.h"
#include "tests/TestHarness.h"
#include <memory>
#include <stdint.h>

namespace catapult { namespace thread { class IoServiceThreadPool; } }

namespace catapult { namespace test {

	/// Creates an auto started threadpool with a default number of threads and \a name.
	std::unique_ptr<thread::IoServiceThreadPool> CreateStartedIoServiceThreadPool(const char* name = nullptr);

	/// Creates an auto started threadpool with \a numThreads threads and \a name.
	std::unique_ptr<thread::IoServiceThreadPool> CreateStartedIoServiceThreadPool(uint32_t numThreads, const char* name = nullptr);
}}
