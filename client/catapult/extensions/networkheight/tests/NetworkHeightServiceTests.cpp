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
			static constexpr auto CreateRegistrar = CreateNetworkHeightServiceRegistrar;
		};

		class TestContext : public test::ServiceLocatorTestContext<NetworkHeightServiceTraits> {
		public:
			TestContext() : TestContext(std::vector<Height>{})
			{}

			explicit TestContext(std::vector<Height>&& heights) {
				// set up hooks
				testState().state().hooks().setRemoteChainHeightsRetriever([heights = std::move(heights)](const auto&) mutable {
					return thread::make_ready_future(std::move(heights));
				});
			}
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

	// region chainSyncedPredicate

	namespace {
		void SetChainHeight(io::BlockStorageCache& storage, uint32_t numBlocks) {
			auto modifier = storage.modifier();
			for (auto i = 2u; i <= numBlocks; ++i) {
				model::Block block;
				block.Size = sizeof(model::Block);
				block.Height = Height(i);
				modifier.saveBlock(test::BlockToBlockElement(block));
			}
		}

		void AssertChainSyncedPredicate(uint32_t localChainHeight, uint32_t remoteChainHeight, bool expectedResult) {
			// Arrange:
			TestContext context;
			context.boot();

			// - set local chain height
			SetChainHeight(context.testState().state().storage(), localChainHeight);

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

	TEST(TEST_CLASS, ChainSyncedPredicateHookReturnsTrueIfLocalChainHeightIsWithinAcceptedRange) {
		// Assert:
		AssertChainSyncedPredicate(100, 23, true);
		AssertChainSyncedPredicate(24, 23, true);
		AssertChainSyncedPredicate(23, 23, true);
		AssertChainSyncedPredicate(23, 24, true);
		AssertChainSyncedPredicate(23, 25, true);
		AssertChainSyncedPredicate(23, 26, true);
	}

	TEST(TEST_CLASS, ChainSyncedPredicateHookReturnsFalseIfLocalChainHeightIsOutsideOfAcceptedRange) {
		// Assert:
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
	}

	TEST(TEST_CLASS, NetworkChainHeightDetectionTaskIsScheduled) {
		// Assert:
		test::AssertRegisteredTask(TestContext(), 1, Task_Name);
	}

	TEST(TEST_CLASS, NetworkChainHeightDetectionUpdatesChainHeightIfMaxRetrievedHeightIsLarger) {
		// Arrange:
		TestContext context({ Height(12), Height(57), Height(35), Height(31) });
		RunTaskTest(context, Task_Name, [](auto& networkChainHeight, const auto& task) {
			networkChainHeight = 41;

			// Act:
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);
			EXPECT_EQ(57u, networkChainHeight);
		});
	}

	TEST(TEST_CLASS, NetworkChainHeightDetectionDoesNotUpdateChainHeightIfMaxRetrievedHeightIsSmaller) {
		// Arrange:
		TestContext context({ Height(12), Height(57), Height(35), Height(31) });
		RunTaskTest(context, Task_Name, [](auto& networkChainHeight, const auto& task) {
			networkChainHeight = 58;

			// Act:
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);
			EXPECT_EQ(58u, networkChainHeight);
		});
	}

	TEST(TEST_CLASS, NetworkChainHeightDetectionIsNoOpIfEmptyHeightVectorIsReturnedAndLocalHeightIsZero) {
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

	TEST(TEST_CLASS, NetworkChainHeightDetectionIsNoOpIfEmptyHeightVectorIsReturnedAndLocalHeightIsNonZero) {
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
