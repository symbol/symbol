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

#include "harvesting/src/HarvestingUtFacadeFactory.h"
#include "tests/test/cache/UtTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/other/MockExecutionConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS HarvestingUtFacadeFactoryTests

	namespace {
		constexpr auto Default_Height = Height(17);
		constexpr auto Default_Time = Timestamp(987);

		template<typename TAction>
		void RunUtFacadeTest(TAction action) {
			// Arrange:
			auto catapultCache = test::CreateCatapultCacheWithMarkerAccount(Default_Height);

			test::MockExecutionConfiguration executionConfig;
			HarvestingUtFacadeFactory factory(catapultCache, cache::MemoryCacheOptions(1024, 1000), executionConfig.Config);

			auto pFacade = factory.create(Default_Time);
			ASSERT_TRUE(!!pFacade);

			// Act + Assert:
			action(*pFacade, executionConfig);
		}

		void AssertValidatorContexts(
				const test::MockExecutionConfiguration& executionConfig,
				const std::vector<size_t>& expectedNumDifficultyInfos) {
			// Assert:
			test::MockExecutionConfiguration::AssertValidatorContexts(
					*executionConfig.pValidator,
					expectedNumDifficultyInfos,
					Default_Height + Height(1),
					Default_Time);
		}

		void AssertObserverContexts(
				const test::MockExecutionConfiguration& executionConfig,
				size_t numObserverCalls,
				const std::unordered_set<size_t>& rollbackIndexes) {
			// Assert:
			EXPECT_EQ(numObserverCalls, executionConfig.pObserver->params().size());
			test::MockExecutionConfiguration::AssertObserverContexts(
					*executionConfig.pObserver,
					0,
					Default_Height + Height(1),
					model::ImportanceHeight(0), // a dummy state is passed by the updater because only block observers modify it
					[&rollbackIndexes](auto i) { return rollbackIndexes.cend() != rollbackIndexes.find(i); });
		}

		void AssertObserverContexts(const test::MockExecutionConfiguration& executionConfig, size_t numObserverCalls) {
			AssertObserverContexts(executionConfig, numObserverCalls, {});
		}
	}

	TEST(TEST_CLASS, FacadeIsCreatedAroundEmptyUnconfirmedCache) {
		// Act:
		RunUtFacadeTest([](const auto& facade, const auto&) {
			// Assert:
			EXPECT_EQ(0u, facade.view().size());
		});
	}

	TEST(TEST_CLASS, CanApplyTransactionsToCache_AllSuccess) {
		// Arrange:
		RunUtFacadeTest([](auto& facade, const auto& executionConfig) {
			auto transactionInfos = test::CreateTransactionInfos(4);
			auto transactionHashes = test::ExtractHashes(transactionInfos);

			// Act: apply transactions to the cache
			std::vector<bool> applyResults;
			for (const auto& transactionInfo : transactionInfos)
				applyResults.push_back(facade.apply(transactionInfo));

			// Assert:
			EXPECT_EQ(std::vector<bool>(4, true), applyResults);

			// - all transactions were added to ut cache
			const auto& utCacheView = facade.view();
			EXPECT_EQ(4u, utCacheView.size());
			test::AssertContainsAll(utCacheView, transactionHashes);

			// - check contexts (validator and observer should be called for all notifications, 2 per transaction)
			AssertValidatorContexts(executionConfig, { 0, 1, 2, 3, 4, 5, 6, 7 });
			AssertObserverContexts(executionConfig, 8);
		});
	}

	TEST(TEST_CLASS, CanApplyTransactionsToCache_AllFailure) {
		// Arrange:
		RunUtFacadeTest([](auto& facade, const auto& executionConfig) {
			auto transactionInfos = test::CreateTransactionInfos(4);
			auto transactionHashes = test::ExtractHashes(transactionInfos);

			// - mark all failures
			executionConfig.pValidator->setResult(validators::ValidationResult::Failure);

			// Act: apply transactions to the cache
			std::vector<bool> applyResults;
			for (const auto& transactionInfo : transactionInfos)
				applyResults.push_back(facade.apply(transactionInfo));

			// Assert:
			EXPECT_EQ(std::vector<bool>(4, false), applyResults);

			// - no transactions were added to ut cache
			const auto& utCacheView = facade.view();
			EXPECT_EQ(0u, utCacheView.size());

			// - check contexts (validator should be called for first notifications; observer should never be called)
			AssertValidatorContexts(executionConfig, { 0, 0, 0, 0 });
			AssertObserverContexts(executionConfig, 0);
		});
	}

	namespace {
		template<typename TAssertContexts>
		void AssertCanApplyTransactionsSomeSuccess(size_t idTrigger, TAssertContexts assertContexts) {
			// Arrange:
			RunUtFacadeTest([idTrigger, assertContexts](auto& facade, const auto& executionConfig) {
				auto transactionInfos = test::CreateTransactionInfos(4);
				auto transactionHashes = test::ExtractHashes(transactionInfos);

				// - mark some failures
				executionConfig.pValidator->setResult(validators::ValidationResult::Failure, transactionHashes[1], idTrigger);
				executionConfig.pValidator->setResult(validators::ValidationResult::Failure, transactionHashes[3], idTrigger);

				// Act: apply transactions to the cache
				std::vector<bool> applyResults;
				for (const auto& transactionInfo : transactionInfos)
					applyResults.push_back(facade.apply(transactionInfo));

				// Assert:
				EXPECT_EQ(std::vector<bool>({ true, false, true, false }), applyResults);

				// Assert: only two transactions were added to ut cache
				const auto& utCacheView = facade.view();
				EXPECT_EQ(2u, utCacheView.size());
				test::AssertContainsAll(utCacheView, { transactionHashes[0], transactionHashes[2] });

				// - check contexts
				assertContexts(executionConfig);
			});
		}
	}

	TEST(TEST_CLASS, CanApplyTransactionsToCache_SomeSuccess) {
		// Act:
		AssertCanApplyTransactionsSomeSuccess(1, [](const auto& executionConfig) {
			// Assert: since first notifications failed, no undos are necessary
			AssertValidatorContexts(executionConfig, { 0, 1, 2, 2, 3, 4 });
			AssertObserverContexts(executionConfig, 4);
		});
	}

	TEST(TEST_CLASS, CanApplyTransactionsToCache_SomeSuccessSomeUndos) {
		// Act:
		AssertCanApplyTransactionsSomeSuccess(2, [](const auto& executionConfig) {
			// Assert: since second notifications failed, undos are necessary
			AssertValidatorContexts(executionConfig, { 0, 1, 2, 3, 4, 5, 6, 7 });
			AssertObserverContexts(executionConfig, 8, { 3, 7 });
		});
	}
}}
