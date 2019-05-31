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

#include "catapult/cache_tx/MemoryUtCacheUtils.h"
#include "tests/test/cache/UtTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS MemoryUtCacheUtilsTests

	// region traits

	namespace {
		bool CompareNaturalOrder(const model::TransactionInfo* pLhs, const model::TransactionInfo* pRhs) {
			// test::CreateSeededMemoryUtCache seeds with transactions sorted by deadline
			return pLhs->pEntity->Deadline < pRhs->pEntity->Deadline;
		}

		bool CompareReverseOrder(const model::TransactionInfo* pLhs, const model::TransactionInfo* pRhs) {
			return pLhs->pEntity->Deadline > pRhs->pEntity->Deadline;
		}

		struct GetFirstOrdinalTraits {
			static auto GetFirst(const MemoryUtCacheView& utCacheView, uint32_t count) {
				return GetFirstTransactionInfoPointers(utCacheView, count);
			}
		};

		struct GetFirstFilteredTraits {
			static auto GetFirst(const MemoryUtCacheView& utCacheView, uint32_t count) {
				return GetFirstTransactionInfoPointers(utCacheView, count, [](const auto&) { return true; });
			}
		};

		struct GetFirstSortedFilteredTraits {
			static auto GetFirst(const MemoryUtCacheView& utCacheView, uint32_t count) {
				return GetFirstTransactionInfoPointers(utCacheView, count, CompareNaturalOrder, [](const auto&) { return true; });
			}
		};
	}

#define GET_FIRST_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Ordinal) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<GetFirstOrdinalTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Filtered) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<GetFirstFilteredTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_SortedFiltered) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<GetFirstSortedFilteredTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region basic tests

	namespace {
		template<typename TTraits>
		void RunGetFirstTest(uint32_t count, uint32_t numRequested, uint32_t expectedCount) {
			// Arrange:
			auto pUtCache = test::CreateSeededMemoryUtCache(count);
			auto utCacheView = pUtCache->view();

			// Act:
			auto transactionInfos = TTraits::GetFirst(utCacheView, numRequested);

			// Assert:
			auto expectedTransactionInfos = test::ExtractTransactionInfos(utCacheView, expectedCount);
			ASSERT_EQ(expectedCount, transactionInfos.size());
			for (auto i = 0u; i < expectedCount; ++i)
				test::AssertEqual(*expectedTransactionInfos[i], *transactionInfos[i], "transaction at " + std::to_string(i));
		}
	}

	GET_FIRST_TRAITS_BASED_TEST(GetFirstTransactionInfoPointersReturnsNoTransactionInfosWhenCacheIsEmpty) {
		// Assert:
		RunGetFirstTest<TTraits>(0, 3, 0);
	}

	GET_FIRST_TRAITS_BASED_TEST(GetFirstTransactionInfoPointersReturnsNoTransactionInfosWhenZeroAreRequested) {
		// Assert:
		RunGetFirstTest<TTraits>(10, 0, 0);
	}

	GET_FIRST_TRAITS_BASED_TEST(GetFirstTransactionInfoPointersReturnsFirstCountTransactionInfosWhenCacheHasEnoughTransactions) {
		// Assert:
		RunGetFirstTest<TTraits>(10, 3, 3);
	}

	GET_FIRST_TRAITS_BASED_TEST(GetFirstTransactionInfoPointersReturnsAllTransactionInfosWhenCacheHasExactlyRequestedTransactions) {
		// Assert:
		RunGetFirstTest<TTraits>(10, 10, 10);
	}

	GET_FIRST_TRAITS_BASED_TEST(GetFirstTransactionInfoPointersReturnsAllTransactionInfosWhenCacheHasLessThanRequestedTransactions) {
		// Assert:
		RunGetFirstTest<TTraits>(10, 15, 10);
	}

	// endregion

	// region Filtered

	TEST(TEST_CLASS, GetFirstTransactionInfoPointersAppliesFiltering_Filtered) {
		// Arrange:
		auto pUtCache = test::CreateSeededMemoryUtCache(10);
		auto utCacheView = pUtCache->view();

		// Act: filter odd deadline txes
		auto transactionInfos = GetFirstTransactionInfoPointers(utCacheView, 3, [](const auto& transactionInfo) {
			return 0 == transactionInfo.pEntity->Deadline.unwrap() % 2;
		});

		// Assert: (1, 3, 5) should be returned; if count was applied first, wrong (1) would be returned
		auto allTransactionInfos = test::ExtractTransactionInfos(utCacheView, 10);
		ASSERT_EQ(3u, transactionInfos.size());
		for (auto i = 0u; i < transactionInfos.size(); ++i)
			test::AssertEqual(*allTransactionInfos[1 + i * 2], *transactionInfos[i], "transaction at " + std::to_string(i));
	}

	// endregion

	// region SortedFiltered

	TEST(TEST_CLASS, GetFirstTransactionInfoPointersAppliesSorting_SortedFiltered) {
		// Arrange:
		auto pUtCache = test::CreateSeededMemoryUtCache(10);
		auto utCacheView = pUtCache->view();

		// Act: apply a reverse sort
		auto transactionInfos = GetFirstTransactionInfoPointers(utCacheView, 3, CompareReverseOrder, [](const auto&) {
			return true;
		});

		// Assert: (9, 8, 7) should be returned; if count was applied first, wrong (2, 1, 0) would be returned
		auto allTransactionInfos = test::ExtractTransactionInfos(utCacheView, 10);
		ASSERT_EQ(3u, transactionInfos.size());
		for (auto i = 0u; i < transactionInfos.size(); ++i)
			test::AssertEqual(*allTransactionInfos[10 - i - 1], *transactionInfos[i], "transaction at " + std::to_string(i));
	}

	TEST(TEST_CLASS, GetFirstTransactionInfoPointersAppliesStableSorting_SortedFiltered) {
		// Arrange:
		auto pUtCache = test::CreateSeededMemoryUtCache(10);
		test::AddAll(*pUtCache, test::CreateTransactionInfos(4, [](auto i) {
			return 0 == i % 2 ? Timestamp(10) : Timestamp(9);
		}));
		auto utCacheView = pUtCache->view();

		// Act: apply a reverse sort
		auto transactionInfos = GetFirstTransactionInfoPointers(utCacheView, 4, CompareReverseOrder, [](const auto&) {
			return true;
		});

		// Assert:
		auto allTransactionInfos = test::ExtractTransactionInfos(utCacheView, 14);
		ASSERT_EQ(4u, transactionInfos.size());
		test::AssertEqual(*allTransactionInfos[9], *transactionInfos[0], "transaction at 0");
		test::AssertEqual(*allTransactionInfos[10], *transactionInfos[1], "transaction at 1");
		test::AssertEqual(*allTransactionInfos[12], *transactionInfos[2], "transaction at 2");
		test::AssertEqual(*allTransactionInfos[8], *transactionInfos[3], "transaction at 3");
	}

	TEST(TEST_CLASS, GetFirstTransactionInfoPointersAppliesFiltering_SortedFiltered) {
		// Arrange:
		auto pUtCache = test::CreateSeededMemoryUtCache(10);
		auto utCacheView = pUtCache->view();

		// Act: filter odd deadline txes
		auto transactionInfos = GetFirstTransactionInfoPointers(utCacheView, 3, CompareNaturalOrder, [](const auto& transactionInfo) {
			return 0 == transactionInfo.pEntity->Deadline.unwrap() % 2;
		});

		// Assert: (1, 3, 5) should be returned; if count was applied first, wrong (1) would be returned
		auto allTransactionInfos = test::ExtractTransactionInfos(utCacheView, 10);
		ASSERT_EQ(3u, transactionInfos.size());
		for (auto i = 0u; i < transactionInfos.size(); ++i)
			test::AssertEqual(*allTransactionInfos[1 + i * 2], *transactionInfos[i], "transaction at " + std::to_string(i));
	}

	TEST(TEST_CLASS, GetFirstTransactionInfoPointersAppliesSortingAndFiltering_SortedFiltered) {
		// Arrange:
		auto pUtCache = test::CreateSeededMemoryUtCache(10);
		auto utCacheView = pUtCache->view();

		// Act: apply a reverse sort AND filter odd deadline txes
		auto transactionInfos = GetFirstTransactionInfoPointers(utCacheView, 3, CompareReverseOrder, [](const auto& transactionInfo) {
			return 0 == transactionInfo.pEntity->Deadline.unwrap() % 2;
		});

		// Assert: (9, 7, 5) should be returned; if count was applied first, wrong (1) would be returned
		auto allTransactionInfos = test::ExtractTransactionInfos(utCacheView, 10);
		ASSERT_EQ(3u, transactionInfos.size());
		for (auto i = 0u; i < transactionInfos.size(); ++i)
			test::AssertEqual(*allTransactionInfos[9 - i * 2], *transactionInfos[i], "transaction at " + std::to_string(i));
	}

	// endregion
}}
