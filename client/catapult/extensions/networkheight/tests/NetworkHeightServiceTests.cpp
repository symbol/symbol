/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "networkheight/src/NetworkHeightService.h"
#include "networkheight/src/NetworkChainHeight.h"
#include "networkheight/src/NetworkHeightConfiguration.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace networkheight {

#define TEST_CLASS NetworkHeightServiceTests

	namespace {
		constexpr auto Service_Name = "networkChainHeight";
		constexpr auto Task_Name = "network chain height detection";

		struct NetworkHeightServiceTraits {
			static auto CreateRegistrar() {
				auto networkHeightConfig = NetworkHeightConfiguration::Uninitialized();
				networkHeightConfig.MaxNodes = 4;
				return CreateNetworkHeightServiceRegistrar(networkHeightConfig);
			}
		};

		class TestContext : public test::ServiceLocatorTestContext<NetworkHeightServiceTraits> {
		public:
			TestContext() : TestContext(std::vector<Height>{})
			{}

			explicit TestContext(std::vector<Height>&& heights) : NumPeers(0) {
				// set up hooks
				testState().state().hooks().setRemoteChainHeightsRetriever([this, heights = std::move(heights)](auto numPeers) mutable {
					NumPeers = numPeers;
					return thread::make_ready_future(std::move(heights));
				});
			}

		public:
			size_t NumPeers;
		};

		std::shared_ptr<NetworkChainHeight> GetNetworkChainHeight(const extensions::ServiceLocator& locator) {
			return locator.service<NetworkChainHeight>(Service_Name);
		}
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(NetworkHeight, Post_Remote_Peers)

	// region network chain height

	TEST(TEST_CLASS, NetworkChainHeightServiceIsRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(1u, context.locator().numServices());
		EXPECT_EQ(0u, context.locator().counters().size());

		auto pNetworkChainHeight = GetNetworkChainHeight(context.locator());
		ASSERT_TRUE(!!pNetworkChainHeight);
		EXPECT_EQ(0u, pNetworkChainHeight->load());
	}

	// endregion

	// region network height config

	TEST(TEST_CLASS, NetworkHeightServiceRespectsMaxNodes) {
		// Arrange:
		TestContext context({ Height(1), Height(2), Height(3), Height(4) });
		test::RunTaskTest(context, 1, Task_Name, [&context](const auto& task) mutable {
			// Act:
			task.Callback().get();

			// Assert:
			EXPECT_EQ(4u, context.NumPeers);
		});
	}

	// endregion

	// region chainSyncedPredicate

	namespace {
		void AssertChainSyncedPredicate(uint32_t localChainHeight, uint32_t remoteChainHeight, bool expectedResult) {
			// Arrange:
			TestContext context;
			context.boot();

			// - set local chain height
			mocks::SeedStorageWithFixedSizeBlocks(context.testState().state().storage(), localChainHeight);

			// - set network chain height
			auto pNetworkChainHeight = GetNetworkChainHeight(context.locator());
			ASSERT_TRUE(!!pNetworkChainHeight);
			*pNetworkChainHeight = remoteChainHeight;

			auto predicate = context.testState().state().hooks().chainSyncedPredicate();

			// Act:
			auto result = predicate();

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, ChainSyncedPredicateHookReturnsTrueWhenLocalChainHeightIsWithinAcceptedRange) {
		AssertChainSyncedPredicate(100, 23, true);
		AssertChainSyncedPredicate(24, 23, true);
		AssertChainSyncedPredicate(23, 23, true);
		AssertChainSyncedPredicate(23, 24, true);
		AssertChainSyncedPredicate(23, 25, true);
		AssertChainSyncedPredicate(23, 26, true);
	}

	TEST(TEST_CLASS, ChainSyncedPredicateHookReturnsFalseWhenLocalChainHeightIsOutsideOfAcceptedRange) {
		AssertChainSyncedPredicate(23, 27, false);
		AssertChainSyncedPredicate(23, 28, false);
		AssertChainSyncedPredicate(23, 100, false);
		AssertChainSyncedPredicate(23, 1000, false);
	}

	// endregion

	// region network chain height detection

	namespace {
		template<typename TAction>
		void RunTaskTest(TestContext& context, const std::string& taskName, TAction&& action) {
			// Act:
			test::RunTaskTest(context, 1, taskName, [&context, action = std::move(action)](const auto& task) mutable {
				auto pNetworkChainHeight = GetNetworkChainHeight(context.locator());
				ASSERT_TRUE(!!pNetworkChainHeight);
				action(*pNetworkChainHeight, task);
			});
		}

		void AssertMedian(std::vector<Height>&& heights, uint64_t expectedMedian) {
			// Arrange:
			TestContext context(std::move(heights));
			RunTaskTest(context, Task_Name, [expectedMedian](auto& networkChainHeight, const auto& task) {
				networkChainHeight = 0;

				// Act:
				task.Callback().get();

				// Assert:
				EXPECT_EQ(expectedMedian, networkChainHeight);
			});
		}
	}

	TEST(TEST_CLASS, TasksAreRegistered) {
		test::AssertRegisteredTasks(TestContext(), { Task_Name });
	}

	TEST(TEST_CLASS, MedianIsCalculatedAsExpected) {
		// Assert: odd number of elements
		AssertMedian(std::vector<Height>{ Height(1) }, 1);
		AssertMedian(std::vector<Height>{ Height(4) }, 4);
		AssertMedian(std::vector<Height>{ Height(9), Height(4), Height(16), }, 9);
		AssertMedian(std::vector<Height>{ Height(17), Height(9), Height(18), Height(4), Height(16), }, 16);

		// - even number of elements
		AssertMedian(std::vector<Height>{ Height(2), Height(2) }, 2);
		AssertMedian(std::vector<Height>{ Height(5), Height(1) }, 3);
		AssertMedian(std::vector<Height>{ Height(1), Height(17), Height(7), Height(5) }, 6);
		AssertMedian(std::vector<Height>{ Height(11), Height(5), Height(13), Height(21), Height(1), Height(4) }, 8);
	}

	TEST(TEST_CLASS, NetworkChainHeightDetectionUpdatesChainHeightWhenMedianOfRetrievedHeightsIsLarger) {
		// Arrange: median is 33
		TestContext context({ Height(12), Height(57), Height(35), Height(31) });
		RunTaskTest(context, Task_Name, [](auto& networkChainHeight, const auto& task) {
			networkChainHeight = 32;

			// Act:
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);
			EXPECT_EQ(33u, networkChainHeight);
		});
	}

	TEST(TEST_CLASS, NetworkChainHeightDetectionDoesNotUpdateChainHeightWhenMedianOfRetrievedHeightsIsSmaller) {
		// Arrange:
		TestContext context({ Height(12), Height(57), Height(35), Height(31) });
		RunTaskTest(context, Task_Name, [](auto& networkChainHeight, const auto& task) {
			networkChainHeight = 34;

			// Act:
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);
			EXPECT_EQ(34u, networkChainHeight);
		});
	}

	TEST(TEST_CLASS, NetworkChainHeightDetectionIsNoOpWhenEmptyHeightVectorIsReturnedAndLocalHeightIsZero) {
		// Arrange:
		TestContext context;
		RunTaskTest(context, Task_Name, [](const auto& networkChainHeight, const auto& task) {
			// Act:
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);
			EXPECT_EQ(0u, networkChainHeight);
		});
	}

	TEST(TEST_CLASS, NetworkChainHeightDetectionIsNoOpWhenEmptyHeightVectorIsReturnedAndLocalHeightIsNonzero) {
		// Arrange:
		TestContext context;
		RunTaskTest(context, Task_Name, [](auto& networkChainHeight, const auto& task) {
			networkChainHeight = 41;

			// Act:
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);
			EXPECT_EQ(41u, networkChainHeight);
		});
	}

	// endregion
}}
