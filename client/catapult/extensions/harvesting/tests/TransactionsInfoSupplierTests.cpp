/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "harvesting/src/TransactionsInfoSupplier.h"
#include "harvesting/src/HarvestingUtFacadeFactory.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "tests/test/cache/UtTestUtils.h"
#include "tests/test/other/MockExecutionConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS TransactionsInfoSupplierTests

	namespace {
		using model::TransactionSelectionStrategy;
		using TransactionInfoPointers = std::vector<const model::TransactionInfo*>;

		// region test context

		auto CreateBlockChainConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.ImportanceGrouping = 1;
			return config;
		}

		void AssertConsistent(const TransactionsInfo& transactionsInfo, const HarvestingUtFacade& facade) {
			// Assert: transactionsInfo and facade should contain exact same transactions (and hashes)
			ASSERT_EQ(transactionsInfo.Transactions.size(), facade.size());
			ASSERT_EQ(transactionsInfo.TransactionHashes.size(), facade.size());

			for (auto i = 0u; i < facade.size(); ++i) {
				EXPECT_EQ(*transactionsInfo.Transactions[i], *facade.transactionInfos()[i].pEntity) << i;
				EXPECT_EQ(transactionsInfo.TransactionHashes[i], facade.transactionInfos()[i].EntityHash) << i;
			}
		}

		class TestContext {
		private:
			// constant returned by countRetriever and multiplied by count argument in order to implicitly check
			// countRetriever parameter is used
			static constexpr uint32_t Multiplier = 7;

			static auto EmptyHashSupplier(Height) {
				return Hash256();
			}

		public:
			explicit TestContext(TransactionSelectionStrategy strategy, uint32_t utCacheSize = 0)
					: m_catapultCache(test::CreateCatapultCacheWithMarkerAccount(Height(7)))
					, m_utFacadeFactory(m_catapultCache, CreateBlockChainConfiguration(), m_executionConfig.Config, EmptyHashSupplier)
					, m_pUtCache(test::CreateSeededMemoryUtCache(utCacheSize))
					, m_supplier(CreateTransactionsInfoSupplier(strategy, [](const auto&) { return Multiplier; }, *m_pUtCache))
			{}

		public:
			auto supply(uint32_t count) {
				// Act:
				auto pUtFacade = m_utFacadeFactory.create(Timestamp(1234));
				auto transactionsInfo = m_supplier(*pUtFacade, count * Multiplier);

				// Assert: supplied transactions should populate the ut facade
				AssertConsistent(transactionsInfo, *pUtFacade);
				return transactionsInfo;
			}

		public:
			void seedCacheForSelectionTests() {
				// add 10 transaction infos to UT cache with varying sizes and multipliers
				test::AddAll(*m_pUtCache, test::CreateTransactionInfosFromSizeMultiplierPairs({
					{ 200, 240 }, { 250, 230 },
					{ 225, 820 }, { 275, 810 },
					{ 300, 420 }, { 350, 410 },
					{ 325, 210 }, { 375, 200 },
					{ 400, 810 }, { 450, 800 }
				}));
			}

			void setValidationFailureAt(size_t triggerMin, size_t triggerMax = 0) {
				// set validation failures for all transactions starting at (unsorted) trigger index
				auto utCacheView = m_pUtCache->view();
				auto allTransactionInfos = test::ExtractTransactionInfos(utCacheView, utCacheView.size());
				if (0 == triggerMax)
					triggerMax = allTransactionInfos.size() - 1;

				for (auto i = triggerMin; i <= triggerMax; ++i) {
					const auto& entityHash = allTransactionInfos[i]->EntityHash;
					m_executionConfig.pValidator->setResult(validators::ValidationResult::Failure, entityHash, 1);
				}
			}

			auto extractUtInfos(std::initializer_list<size_t> indexes) {
				auto utCacheView = m_pUtCache->view();
				auto allTransactionInfos = test::ExtractTransactionInfos(utCacheView, utCacheView.size());

				TransactionInfoPointers selectedTransactionInfos;
				for (auto index : indexes)
					selectedTransactionInfos.push_back(allTransactionInfos[index]);

				// Sanity:
				EXPECT_EQ(indexes.size(), selectedTransactionInfos.size());
				return selectedTransactionInfos;
			}

			void assertValidatorCalls(size_t numExpectedCalls) {
				// Assert:
				const auto& validatorParamsCapture = m_executionConfig.pValidator->params();
				ASSERT_EQ(numExpectedCalls, validatorParamsCapture.size());

				// - BlockTime is only part of context explicitly forwarded by supplier
				size_t i = 0;
				for (const auto& params : validatorParamsCapture) {
					EXPECT_EQ(Timestamp(1234), params.Context.BlockTime) << "validator at " << i;
					++i;
				}
			}

		private:
			cache::CatapultCache m_catapultCache;
			test::MockExecutionConfiguration m_executionConfig;
			HarvestingUtFacadeFactory m_utFacadeFactory;
			std::unique_ptr<cache::MemoryUtCache> m_pUtCache;
			TransactionsInfoSupplier m_supplier;
		};

		void AssertTransactionsInfo(
				const TransactionsInfo& transactionsInfo,
				BlockFeeMultiplier expectedFeeMultiplier,
				const TransactionInfoPointers& expectedTransactionInfos) {
			// Assert:
			// - check multiplier
			EXPECT_EQ(expectedFeeMultiplier, transactionsInfo.FeeMultiplier);

			// - check transactions
			auto expectedCount = expectedTransactionInfos.size();
			ASSERT_EQ(expectedCount, transactionsInfo.Transactions.size());
			ASSERT_EQ(expectedCount, transactionsInfo.TransactionHashes.size());
			for (auto i = 0u; i < expectedCount; ++i) {
				const auto& expectedTransactionInfo = *expectedTransactionInfos[i];
				EXPECT_EQ(*expectedTransactionInfo.pEntity, *transactionsInfo.Transactions[i]) << "transaction at " << i;
				EXPECT_EQ(expectedTransactionInfo.EntityHash, transactionsInfo.TransactionHashes[i]) << "transaction hash at " << i;
			}

			// - check aggregate hash
			Hash256 expectedHash;
			CalculateBlockTransactionsHash(expectedTransactionInfos, expectedHash);
			EXPECT_EQ(expectedHash, transactionsInfo.TransactionsHash);
		}

		// endregion
	}

	// region basic

#define STRATEGY_BASED_TEST(TEST_NAME) \
	template<TransactionSelectionStrategy Strategy> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Oldest) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionSelectionStrategy::Oldest>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MinimizeFee) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionSelectionStrategy::Minimize_Fee>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MaximizeFee) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionSelectionStrategy::Maximize_Fee>(); } \
	template<TransactionSelectionStrategy Strategy> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		void AssertEmptySupplierResults(TransactionSelectionStrategy strategy, uint32_t count, uint32_t numRequested) {
			// Arrange:
			TestContext context(strategy, count);

			// Act:
			auto transactionsInfo = context.supply(numRequested);

			// Assert:
			AssertTransactionsInfo(transactionsInfo, BlockFeeMultiplier(0), {});

			context.assertValidatorCalls(0);
		}
	}

	STRATEGY_BASED_TEST(SupplierReturnsNoTransactionInfosWhenCacheIsEmpty) {
		AssertEmptySupplierResults(Strategy, 0, 3);
	}

	STRATEGY_BASED_TEST(SupplierReturnsNoTransactionInfosWhenZeroInfosAreRequested) {
		AssertEmptySupplierResults(Strategy, 10, 0);
	}

	// endregion

	// region oldest

	TEST(TEST_CLASS, OldestStrategy_CanSelectTransactions) {
		// Arrange:
		TestContext context(TransactionSelectionStrategy::Oldest);
		context.seedCacheForSelectionTests();

		// Act:
		auto transactionsInfo = context.supply(5);

		// Assert:
		// 1. first (oldest) five transactions are chosen
		// 2. multiplier is min max multiplier of those
		//     (200, 24)+ (250, 23)+ (225, 82)+ (275, 81)+ (300, 42)+
		//     (350, 41)  (325, 21)  (375, 20)  (400, 81)  (450, 80)
		auto expectedTransactionInfos = context.extractUtInfos({ 0, 1, 2, 3, 4 });
		AssertTransactionsInfo(transactionsInfo, BlockFeeMultiplier(23), expectedTransactionInfos);

		context.assertValidatorCalls(10);
	}

	TEST(TEST_CLASS, OldestStrategy_CanSelectTransactionsWhereSomeFailValidation) {
		// Arrange: trigger four transactions to fail
		TestContext context(TransactionSelectionStrategy::Oldest);
		context.seedCacheForSelectionTests();
		context.setValidationFailureAt(1, 4);

		// Act:
		auto transactionsInfo = context.supply(3);

		// Assert:
		// 1. first three (oldest) transactions that pass validation are chosen
		// 2. multiplier is min max multiplier of those
		//     (200, 24)+ (250, 23)  (225, 82)  (275, 81)  (300, 42)
		//     (350, 41)+ (325, 21)+ (375, 20)  (400, 81)  (450, 80)
		auto expectedTransactionInfos = context.extractUtInfos({ 0, 5, 6 });
		AssertTransactionsInfo(transactionsInfo, BlockFeeMultiplier(21), expectedTransactionInfos);

		// - 3 transactions (2 success notifications each) + 4 transactions (1 failure notification each)
		//   transactions are processed until 3 transactions are processed
		context.assertValidatorCalls(3 * 2 + 4);
	}

	TEST(TEST_CLASS, OldestStrategy_CanSelectTransactionsWhereAllFailValidation) {
		// Arrange:
		TestContext context(TransactionSelectionStrategy::Oldest);
		context.seedCacheForSelectionTests();
		context.setValidationFailureAt(0);

		// Act:
		auto transactionsInfo = context.supply(5);

		// Assert:
		AssertTransactionsInfo(transactionsInfo, BlockFeeMultiplier(0), {});

		// - 10 transactions (1 failure notification each)
		context.assertValidatorCalls(10);
	}

	// endregion

	// region minimize

	TEST(TEST_CLASS, MinimizeStrategy_CanSelectTransactions) {
		// Arrange:
		TestContext context(TransactionSelectionStrategy::Minimize_Fee);
		context.seedCacheForSelectionTests();

		// Act:
		auto transactionsInfo = context.supply(6);

		// Assert:
		// 1. six transactions with smallest multipliers are chosen
		// 2. multiplier is min max multiplier of those
		//     (200, 24)+ (250, 23)+ (225, 82)  (275, 81)  (300, 42)+
		//     (350, 41)+ (325, 21)+ (375, 20)+ (400, 81)  (450, 80)
		auto expectedTransactionInfos = context.extractUtInfos({ 7, 6, 1, 0, 5, 4 });
		AssertTransactionsInfo(transactionsInfo, BlockFeeMultiplier(20), expectedTransactionInfos);

		// - 6 transactions (2 success notifications each)
		context.assertValidatorCalls(6 * 2);
	}

	TEST(TEST_CLASS, MinimizeStrategy_CanSelectTransactionsWhereSomeFailValidation) {
		// Arrange: trigger the second half of transactions to fail
		TestContext context(TransactionSelectionStrategy::Minimize_Fee);
		context.seedCacheForSelectionTests();
		context.setValidationFailureAt(5);

		// Act:
		auto transactionsInfo = context.supply(3);

		// Assert:
		// 1. three transactions with smallest multipliers are chosen (of those that pass validation)
		// 2. multiplier is min max multiplier of those
		//     (200, 24)+ (250, 23)+ (225, 82)  (275, 81)  (300, 42)+
		auto expectedTransactionInfos = context.extractUtInfos({ 1, 0, 4 });
		AssertTransactionsInfo(transactionsInfo, BlockFeeMultiplier(23), expectedTransactionInfos);

		// - 3 transactions (2 success notifications each) + 3 transactions (1 failure notification each)
		//   transactions are processed in sorted order until 3 transactions from first five are processed
		context.assertValidatorCalls(3 * 2 + 3);
	}

	TEST(TEST_CLASS, MinimizeStrategy_CanSelectTransactionsWhereAllFailValidation) {
		// Arrange:
		TestContext context(TransactionSelectionStrategy::Minimize_Fee);
		context.seedCacheForSelectionTests();
		context.setValidationFailureAt(0);

		// Act:
		auto transactionsInfo = context.supply(5);

		// Assert:
		AssertTransactionsInfo(transactionsInfo, BlockFeeMultiplier(0), {});

		// - 10 transactions (1 failure notification each)
		context.assertValidatorCalls(10);
	}

	// endregion

	// region maximize

	TEST(TEST_CLASS, MaximizeStrategy_CanSelectTransactions) {
		// Arrange:
		TestContext context(TransactionSelectionStrategy::Maximize_Fee);
		context.seedCacheForSelectionTests();

		// Act:
		auto transactionsInfo = context.supply(6);

		// Assert:
		// 1. best fee policy is chosen to maximize fees (fewer transactions than requested are selected)
		// 2. multiplier is min max multiplier of those
		//     (200, 24)  (250, 23)  (225, 82)+ (275, 81)+ (300, 42)
		//     (350, 41)  (325, 21)  (375, 20)  (400, 81)+ (450, 80)+
		auto expectedTransactionInfos = context.extractUtInfos({ 2, 3, 8, 9 });
		AssertTransactionsInfo(transactionsInfo, BlockFeeMultiplier(80), expectedTransactionInfos);

		// - 6 transactions (2 success notifications each)
		context.assertValidatorCalls(6 * 2);
	}

	TEST(TEST_CLASS, MaximizeStrategy_CanSelectTransactionsWhereSomeFailValidation) {
		// Arrange: trigger the second half of transactions to fail
		TestContext context(TransactionSelectionStrategy::Maximize_Fee);
		context.seedCacheForSelectionTests();
		context.setValidationFailureAt(5);

		// Act:
		auto transactionsInfo = context.supply(3);

		// Assert:
		// 1. best fee policy is chosen to maximize fees (fewer transactions than requested are selected)
		// 2. multiplier is min max multiplier of those
		//     (200, 24)  (250, 23)  (225, 82)+ (275, 81)+ (300, 42)
		auto expectedTransactionInfos = context.extractUtInfos({ 2, 3 });
		AssertTransactionsInfo(transactionsInfo, BlockFeeMultiplier(81), expectedTransactionInfos);

		// - 3 transactions (2 success notifications each) + 2 transactions (1 failure notification each)
		//   transactions are processed in sorted order until 3 transactions from first five are processed
		context.assertValidatorCalls(3 * 2 + 2);
	}

	TEST(TEST_CLASS, MaximizeStrategy_CanSelectTransactionsWhereAllFailValidation) {
		// Arrange:
		TestContext context(TransactionSelectionStrategy::Maximize_Fee);
		context.seedCacheForSelectionTests();
		context.setValidationFailureAt(0);

		// Act:
		auto transactionsInfo = context.supply(5);

		// Assert:
		AssertTransactionsInfo(transactionsInfo, BlockFeeMultiplier(0), {});

		// - 10 transactions (1 failure notification each)
		context.assertValidatorCalls(10);
	}

	// endregion
}}
