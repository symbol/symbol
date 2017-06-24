#pragma once
#include "catapult/local/LocalNodeStats.h"
#include "catapult/thread/MultiServicePool.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region basic

	/// Asserts that the scheduler service can be booted with \a numTasks tasks.
	template<typename TTestContext>
	void AssertCanBootSchedulerService(uint32_t numTasks) {
		// Arrange:
		TTestContext context;
		auto& service = context.service();

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(numTasks, service.numScheduledTasks());
	}

	/// Asserts that the scheduler service can be shutdown.
	template<typename TTestContext>
	void AssertCanShutdownSchedulerService() {
		// Arrange:
		TTestContext context;
		auto& service = context.service();
		context.boot();

		// Act:
		context.pool().shutdown();

		// Assert:
		EXPECT_EQ(local::Sentinel_Stats_Value, service.numScheduledTasks());
	}

	// endregion
}}
