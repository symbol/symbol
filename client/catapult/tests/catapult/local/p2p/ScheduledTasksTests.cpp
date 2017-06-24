#include "catapult/local/p2p/ScheduledTasks.h"
#include "catapult/cache/MemoryUtCache.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/mocks/MemoryBasedStorage.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace p2p {

	// region CreateHarvestingTask

	TEST(ScheduledTasksTests, CreateHarvestingTask_CanCreateTask) {
		// Arrange:
		auto config = model::BlockChainConfiguration::Uninitialized();
		auto cache = test::CreateEmptyCatapultCache(config);
		auto pStorage = mocks::CreateMemoryBasedStorageCache(3);
		chain::UnlockedAccounts unlockedAccounts(3);

		// Act:
		auto task = CreateHarvestingTask(
				cache,
				*pStorage,
				config,
				unlockedAccounts,
				[](const auto&, const auto&) { return 0; },
				[](auto) { return chain::TransactionsInfo(); },
				[]() { return true; });

		// Assert:
		EXPECT_EQ("harvesting task", task.Name);
		EXPECT_EQ(utils::TimeSpan::FromSeconds(30), task.StartDelay);
		EXPECT_EQ(utils::TimeSpan::FromSeconds(1), task.RepeatDelay);
	}

	// endregion

	// region CreateLoggingTask

	TEST(ScheduledTasksTests, CreateLoggingTask_CanCreateTask) {
		// Act:
		auto task = CreateLoggingTask({});

		// Assert:
		EXPECT_EQ("logging task", task.Name);
		EXPECT_EQ(utils::TimeSpan::FromMinutes(1), task.StartDelay);
		EXPECT_EQ(utils::TimeSpan::FromMinutes(10), task.RepeatDelay);
	}

	// endregion

	// region CreateChainHeightDetectionTask

	namespace {
		RemoteChainHeightsRetriever CreateRetrieverWithResult(std::vector<Height>&& heights) {
			return [heights = std::move(heights)](const auto&) mutable -> thread::future<std::vector<Height>> {
				return thread::make_ready_future(std::move(heights));
			};
		}
	}

	TEST(ScheduledTasksTests, CreateChainHeightDetectionTask_CanCreateTask) {
		// Arrange:
		NetworkChainHeight chainHeight(123);
		RemoteChainHeightsRetriever retriever = CreateRetrieverWithResult(std::vector<Height>());

		// Act:
		auto task = CreateChainHeightDetectionTask(retriever, chainHeight);

		// Assert:
		EXPECT_EQ("network chain height detection", task.Name);
		EXPECT_EQ(utils::TimeSpan::FromSeconds(1), task.StartDelay);
		EXPECT_EQ(utils::TimeSpan::FromSeconds(15), task.RepeatDelay);

		// height was not modified
		EXPECT_EQ(123u, chainHeight);
	}

	TEST(ScheduledTasksTests, CreateChainHeightDetectionTask_UpdatesChainHeightIfMaxRetrievedHeightIsLarger) {
		// Arrange:
		NetworkChainHeight chainHeight(41);
		RemoteChainHeightsRetriever retriever = CreateRetrieverWithResult({ Height(12), Height(57), Height(35), Height(31) });
		auto task = CreateChainHeightDetectionTask(retriever, chainHeight);

		// Act:
		auto result = task.Callback().get();

		// Assert:
		EXPECT_EQ(thread::TaskResult::Continue, result);
		EXPECT_EQ(57u, chainHeight);
	}

	TEST(ScheduledTasksTests, CreateChainHeightDetectionTask_DoesNotUpdateChainHeightIfMaxRetrievedHeightIsSmaller) {
		// Arrange:
		NetworkChainHeight chainHeight(58);
		RemoteChainHeightsRetriever retriever = CreateRetrieverWithResult({ Height(12), Height(57), Height(35), Height(31) });
		auto task = CreateChainHeightDetectionTask(retriever, chainHeight);

		// Act:
		auto result = task.Callback().get();

		// Assert:
		EXPECT_EQ(thread::TaskResult::Continue, result);
		EXPECT_EQ(58u, chainHeight);
	}

	TEST(ScheduledTasksTests, CreateChainHeightDetectionTask_NoOperationIfEmptyHeightVectorIsReturned) {
		// Arrange:
		NetworkChainHeight chainHeight(0);
		RemoteChainHeightsRetriever retriever = CreateRetrieverWithResult({});
		auto task = CreateChainHeightDetectionTask(retriever, chainHeight);

		// Act:
		auto result = task.Callback().get();

		// Assert:
		EXPECT_EQ(thread::TaskResult::Continue, result);
		EXPECT_EQ(0u, chainHeight);
	}

	// endregion
}}}
