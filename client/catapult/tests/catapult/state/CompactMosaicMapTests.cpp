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

#include "catapult/state/CompactMosaicMap.h"
#include "catapult/utils/Casting.h"
#include "tests/test/nodeps/IteratorTestTraits.h"
#include "tests/TestHarness.h"
#include <unordered_map>

namespace catapult { namespace state {

#define TEST_CLASS CompactMosaicMapTests

	namespace {
		using MosaicMap = std::unordered_map<MosaicId, Amount, utils::BaseValueHasher<MosaicId>>;

		// region utils

		MosaicMap GetTenExpectedMosaics() {
			return {
				{ MosaicId(1), Amount(1) },
				{ MosaicId(102), Amount(4) },
				{ MosaicId(3), Amount(9) },
				{ MosaicId(104), Amount(16) },
				{ MosaicId(5), Amount(25) },
				{ MosaicId(106), Amount(36) },
				{ MosaicId(7), Amount(49) },
				{ MosaicId(108), Amount(64) },
				{ MosaicId(9), Amount(81) },
				{ MosaicId(110), Amount(100) }
			};
		}

		MosaicId GetMosaicId(size_t index) {
			return MosaicId(index + (0 == index % 2 ? 100 : 0));
		}

		void InsertMany(CompactMosaicMap& map, size_t count) {
			for (auto i = 1u; i <= count; ++i)
				map.insert(std::make_pair(GetMosaicId(i), Amount(i * i)));
		}

		template<typename TMap>
		bool Contains(TMap& map, MosaicId mosaicId) {
			return map.end() != map.find(mosaicId);
		}

		// endregion

		// region asserts

		template<typename TMap>
		void AssertEmpty(TMap& map, const std::string& description) {
			// Assert:
			EXPECT_EQ(map.begin(), map.end()) << description;

			// - find should return end for mosaics
			EXPECT_FALSE(Contains(map, MosaicId(1))) << description;
			EXPECT_FALSE(Contains(map, MosaicId(2))) << description;
			EXPECT_FALSE(Contains(map, MosaicId(100))) << description;
		}

		void AssertEmpty(CompactMosaicMap& map) {
			// Assert:
			EXPECT_TRUE(map.empty());
			EXPECT_EQ(0u, map.size());

			AssertEmpty(utils::as_const(map), "const");
			AssertEmpty(map, "non-const");
		}

		template<typename TActual>
		void AssertContents(const MosaicMap& expectedMosaics, TActual& actualMosaics, const std::string& description) {
			// Assert:
			EXPECT_EQ(expectedMosaics.size(), actualMosaics.size()) << description;

			for (const auto& pair : expectedMosaics) {
				std::ostringstream message;
				message << "mosaic " << pair.first << " " << description;

				auto iter = actualMosaics.find(pair.first);
				ASSERT_NE(actualMosaics.end(), iter) << message.str();
				EXPECT_EQ(pair.first, iter->first) << message.str();
				EXPECT_EQ(pair.second, iter->second) << message.str();
			}
		}

		template<typename TActual>
		void AssertIteratedContents(
				MosaicId optimizedMosaicId,
				const MosaicMap& expectedMosaics,
				TActual& actualMosaics,
				const std::string& description) {
			// Assert:
			EXPECT_NE(actualMosaics.begin(), actualMosaics.end()) << description;

			auto numIteratedMosaics = 0u;
			auto lastMosaicId = MosaicId();
			MosaicMap iteratedMosaics;
			for (const auto& pair : actualMosaics) {
				EXPECT_LT(lastMosaicId, pair.first) << "expected ordering at " << numIteratedMosaics;

				// compact map uses a custom sort that treats optimizedMosaicId as smallest value; all other mosaics are sorted normally
				if (0 != numIteratedMosaics || optimizedMosaicId != pair.first)
					lastMosaicId = pair.first;

				if (0 != numIteratedMosaics)
					EXPECT_NE(optimizedMosaicId, pair.first) << "unexpected optimizedMosaicId at " << numIteratedMosaics;

				iteratedMosaics.insert(pair);
				++numIteratedMosaics;
			}

			EXPECT_EQ(expectedMosaics.size(), numIteratedMosaics) << description;
			AssertContents(expectedMosaics, iteratedMosaics, description);
		}

		void AssertContents(CompactMosaicMap& map, MosaicId optimizedMosaicId, const MosaicMap& expectedMosaics) {
			// Assert:
			EXPECT_FALSE(map.empty());

			// - check that all mosaics are accessible via find
			AssertContents(expectedMosaics, utils::as_const(map), "via find (const)");
			AssertContents(expectedMosaics, map, "via find (non-const)");

			// - check that all mosaics are accessible via iteration
			AssertIteratedContents(optimizedMosaicId, expectedMosaics, utils::as_const(map), "via iteration (const)");
			AssertIteratedContents(optimizedMosaicId, expectedMosaics, map, "via iteration (non-const)");
		}

		void AssertContents(CompactMosaicMap& map, const MosaicMap& expectedMosaics) {
			AssertContents(map, MosaicId(), expectedMosaics);
		}

		// endregion
	}

	// region constructor

	TEST(TEST_CLASS, MapIsInitiallyEmpty) {
		// Act:
		CompactMosaicMap map;

		// Assert:
		AssertEmpty(map);
	}

	// endregion

	// region insert

	TEST(TEST_CLASS, CanInsertMosaic) {
		// Arrange:
		CompactMosaicMap map;

		// Act:
		map.insert(std::make_pair(MosaicId(123), Amount(245)));

		// Assert:
		AssertContents(map, { { MosaicId(123), Amount(245) } });
	}

	TEST(TEST_CLASS, CanInsertMosaicWithSmallerValue) {
		// Arrange:
		CompactMosaicMap map;

		// Act: add mosaic ids of decreasing value
		map.insert(std::make_pair(MosaicId(123), Amount(245)));
		map.insert(std::make_pair(MosaicId(100), Amount(333)));

		// Assert:
		AssertContents(map, {
			{ MosaicId(100), Amount(333) },
			{ MosaicId(123), Amount(245) }
		});
	}

	TEST(TEST_CLASS, CanInsertMultipleMosaics_ValueAndPartialArray) {
		// Arrange:
		CompactMosaicMap map;

		// Act:
		InsertMany(map, 2);

		// Assert:
		AssertContents(map, {
			{ MosaicId(1), Amount(1) },
			{ MosaicId(102), Amount(4) }
		});
	}

	TEST(TEST_CLASS, CanInsertMultipleMosaics_ValueAndFullArray) {
		// Arrange:
		CompactMosaicMap map;

		// Act:
		InsertMany(map, 6);

		// Assert:
		AssertContents(map, {
			{ MosaicId(1), Amount(1) },
			{ MosaicId(102), Amount(4) },
			{ MosaicId(3), Amount(9) },
			{ MosaicId(104), Amount(16) },
			{ MosaicId(5), Amount(25) },
			{ MosaicId(106), Amount(36) }
		});
	}

	TEST(TEST_CLASS, CanInsertMultipleMosaics_ValueAndFullArrayAndMap) {
		// Arrange:
		CompactMosaicMap map;

		// Act:
		InsertMany(map, 7);

		// Assert:
		AssertContents(map, {
			{ MosaicId(1), Amount(1) },
			{ MosaicId(102), Amount(4) },
			{ MosaicId(3), Amount(9) },
			{ MosaicId(104), Amount(16) },
			{ MosaicId(5), Amount(25) },
			{ MosaicId(106), Amount(36) },
			{ MosaicId(7), Amount(49) }
		});
	}

	TEST(TEST_CLASS, CanInsertMultipleMosaics_Many) {
		// Arrange:
		CompactMosaicMap map;

		// Act:
		InsertMany(map, 10);

		// Assert:
		AssertContents(map, GetTenExpectedMosaics());
	}

	TEST(TEST_CLASS, CanInsertMosaicWithSmallerValue_Many) {
		// Arrange:
		CompactMosaicMap map;
		map.optimize(MosaicId(2));
		InsertMany(map, 10);

		// Act:
		map.insert(std::make_pair(MosaicId(2), Amount(333)));

		// Assert:
		auto expectedMosaics = GetTenExpectedMosaics();
		expectedMosaics.emplace(MosaicId(2), Amount(333));
		AssertContents(map, MosaicId(2), expectedMosaics);

		// Sanity:
		EXPECT_EQ(MosaicId(2), map.begin()->first);
	}

	TEST(TEST_CLASS, CanInsertMosaicWithZeroBalance) {
		// Arrange:
		CompactMosaicMap map;

		// Act:
		map.insert(std::make_pair(MosaicId(123), Amount(0)));

		// Assert:
		AssertContents(map, { { MosaicId(123), Amount(0) } });
	}

	TEST(TEST_CLASS, CannotInsertReservedMosaic) {
		// Arrange:
		CompactMosaicMap map;

		// Act:
		EXPECT_THROW(map.insert(std::make_pair(MosaicId(0), Amount(245))), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotInsertExistingMosaic) {
		// Arrange:
		CompactMosaicMap map;
		map.insert(std::make_pair(MosaicId(123), Amount(245)));

		// Act:
		EXPECT_THROW(map.insert(std::make_pair(MosaicId(123), Amount(245))), catapult_invalid_argument);
	}

	// endregion

	// region insert + optimize

	namespace {
		MosaicMap GetSevenExpectedMosaicsForOptimizeTests() {
			return {
				{ MosaicId(29), Amount(876) },
				{ MosaicId(85), Amount(111) },
				{ MosaicId(100), Amount(333) },
				{ MosaicId(101), Amount(10) },
				{ MosaicId(102), Amount(20) },
				{ MosaicId(104), Amount(40) },
				{ MosaicId(303), Amount(30) }
			};
		}

		void InsertSevenForOptimizeTests(CompactMosaicMap& map) {
			map.insert(std::make_pair(MosaicId(100), Amount(333)));
			map.insert(std::make_pair(MosaicId(29), Amount(876)));
			map.insert(std::make_pair(MosaicId(85), Amount(111)));
			map.insert(std::make_pair(MosaicId(104), Amount(40)));
			map.insert(std::make_pair(MosaicId(303), Amount(30)));
			map.insert(std::make_pair(MosaicId(102), Amount(20)));
			map.insert(std::make_pair(MosaicId(101), Amount(10)));
		}
	}

	TEST(TEST_CLASS, CanInsertOptimizedMosaic) {
		// Arrange:
		CompactMosaicMap map;
		map.optimize(MosaicId(85));

		// Act: insert after optimize
		InsertSevenForOptimizeTests(map);

		// Assert: optimized mosaic id should be treated as smallest value
		AssertContents(map, MosaicId(85), GetSevenExpectedMosaicsForOptimizeTests());
	}

	namespace {
		void AssertCanInsertAndThenOptimize(MosaicId optimizedMosaicId) {
			// Arrange:
			CompactMosaicMap map;
			InsertSevenForOptimizeTests(map);

			// Sanity:
			EXPECT_EQ(MosaicId(29), map.begin()->first);

			// Act: optimize after insert
			map.optimize(optimizedMosaicId);

			// Assert: optimized mosaic id should be treated as smallest value
			AssertContents(map, optimizedMosaicId, GetSevenExpectedMosaicsForOptimizeTests());

			// Sanity:
			EXPECT_EQ(optimizedMosaicId, map.begin()->first);
		}
	}

	TEST(TEST_CLASS, CanInsertAndThenOptimizeMosaicFromArray) {
		AssertCanInsertAndThenOptimize(MosaicId(101));
	}

	TEST(TEST_CLASS, CanInsertAndThenOptimizeMosaicFromMap) {
		AssertCanInsertAndThenOptimize(MosaicId(303));
	}

	TEST(TEST_CLASS, CanReoptimizeWithSameMosaic) {
		// Arrange:
		CompactMosaicMap map;

		// Act: optimize, insert, optimize (note that optimize must be called with same id)
		map.optimize(MosaicId(85));
		map.optimize(MosaicId(85));
		InsertSevenForOptimizeTests(map);
		map.optimize(MosaicId(85));
		map.optimize(MosaicId(85));

		// Assert: optimized mosaic id should be treated as smallest value
		AssertContents(map, MosaicId(85), GetSevenExpectedMosaicsForOptimizeTests());
	}

	TEST(TEST_CLASS, CanDeoptimizeWhenUnoptimized) {
		// Arrange:
		CompactMosaicMap map;

		// Act: first optimize is noop
		map.optimize(MosaicId(0));
		map.optimize(MosaicId(85));
		InsertSevenForOptimizeTests(map);

		// Assert: optimized mosaic id should be treated as smallest value
		AssertContents(map, MosaicId(85), GetSevenExpectedMosaicsForOptimizeTests());
	}

	TEST(TEST_CLASS, CanReoptimizeWithDifferentMosaic) {
		// Arrange: insert and optimize
		CompactMosaicMap map;
		InsertSevenForOptimizeTests(map);
		map.optimize(MosaicId(102));

		// Sanity:
		EXPECT_EQ(MosaicId(102), map.begin()->first);

		// Act: change optimization
		map.optimize(MosaicId(85));

		// Assert: optimized mosaic id should be treated as smallest value
		AssertContents(map, MosaicId(85), GetSevenExpectedMosaicsForOptimizeTests());

		// Sanity:
		EXPECT_EQ(MosaicId(85), map.begin()->first);
	}

	TEST(TEST_CLASS, CanDeoptimizeWhenOptimized) {
		// Arrange: insert and optimize
		CompactMosaicMap map;
		InsertSevenForOptimizeTests(map);
		map.optimize(MosaicId(102));

		// Sanity:
		EXPECT_EQ(MosaicId(102), map.begin()->first);

		// Act: deoptimize
		map.optimize(MosaicId(0));

		// Assert:
		AssertContents(map, GetSevenExpectedMosaicsForOptimizeTests());

		// Sanity:
		EXPECT_EQ(MosaicId(29), map.begin()->first);
	}

	// endregion

	// region erase

	TEST(TEST_CLASS, CanEraseMosaic_Value) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 5);

		// Act:
		map.erase(MosaicId(1));

		// Assert:
		AssertContents(map, {
			{ MosaicId(102), Amount(4) },
			{ MosaicId(3), Amount(9) },
			{ MosaicId(104), Amount(16) },
			{ MosaicId(5), Amount(25) }
		});
	}

	TEST(TEST_CLASS, CanEraseMosaic_Array) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 5);

		// Act:
		map.erase(MosaicId(3));

		// Assert:
		AssertContents(map, {
			{ MosaicId(1), Amount(1) },
			{ MosaicId(102), Amount(4) },
			{ MosaicId(104), Amount(16) },
			{ MosaicId(5), Amount(25) }
		});
	}

	TEST(TEST_CLASS, CanEraseMultipleMosaics_Odd) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		for (auto i = 1u; i <= 10; i += 2)
			map.erase(GetMosaicId(i));

		// Assert:
		AssertContents(map, {
			{ MosaicId(102), Amount(4) },
			{ MosaicId(104), Amount(16) },
			{ MosaicId(106), Amount(36) },
			{ MosaicId(108), Amount(64) },
			{ MosaicId(110), Amount(100) }
		});
	}

	TEST(TEST_CLASS, CanEraseMultipleMosaics_Even) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		for (auto i = 2u; i <= 10; i += 2)
			map.erase(GetMosaicId(i));

		// Assert:
		AssertContents(map, {
			{ MosaicId(1), Amount(1) },
			{ MosaicId(3), Amount(9) },
			{ MosaicId(5), Amount(25) },
			{ MosaicId(7), Amount(49) },
			{ MosaicId(9), Amount(81) }
		});
	}

	TEST(TEST_CLASS, CanEraseAllMosaics_Forward) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		for (auto i = 1u; i <= 10; ++i)
			map.erase(GetMosaicId(i));

		// Assert:
		AssertEmpty(map);
	}

	TEST(TEST_CLASS, CanEraseAllMosaics_Reverse) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		for (auto i = 10u; i >= 1; --i)
			map.erase(GetMosaicId(i));

		// Assert:
		AssertEmpty(map);
	}

	TEST(TEST_CLASS, CanEraseMosaicNotInMap) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 5);

		// Act: erasing a non-existent mosaic has no effect
		map.erase(MosaicId(123));

		// Assert:
		AssertContents(map, {
			{ MosaicId(1), Amount(1) },
			{ MosaicId(102), Amount(4) },
			{ MosaicId(3), Amount(9) },
			{ MosaicId(104), Amount(16) },
			{ MosaicId(5), Amount(25) }
		});
	}

	// endregion

	// region insert after erase

	TEST(TEST_CLASS, CanInsertAfterEraseMultipleMosaics_Odd) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		for (auto i = 1u; i <= 10; i += 2)
			map.erase(GetMosaicId(i));

		for (auto i = 1u; i <= 3; ++i)
			map.insert(std::make_pair(MosaicId(1000 + i), Amount(10 - i)));

		// Assert:
		AssertContents(map, {
			{ MosaicId(102), Amount(4) },
			{ MosaicId(104), Amount(16) },
			{ MosaicId(106), Amount(36) },
			{ MosaicId(108), Amount(64) },
			{ MosaicId(110), Amount(100) },
			{ MosaicId(1001), Amount(9) },
			{ MosaicId(1002), Amount(8) },
			{ MosaicId(1003), Amount(7) }
		});
	}

	TEST(TEST_CLASS, CanInsertAfterEraseMultipleMosaics_Even) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		for (auto i = 2u; i <= 10; i += 2)
			map.erase(GetMosaicId(i));

		for (auto i = 1u; i <= 3; ++i)
			map.insert(std::make_pair(MosaicId(1000 + i), Amount(10 - i)));

		// Assert:
		AssertContents(map, {
			{ MosaicId(1), Amount(1) },
			{ MosaicId(3), Amount(9) },
			{ MosaicId(5), Amount(25) },
			{ MosaicId(7), Amount(49) },
			{ MosaicId(9), Amount(81) },
			{ MosaicId(1001), Amount(9) },
			{ MosaicId(1002), Amount(8) },
			{ MosaicId(1003), Amount(7) }
		});
	}

	TEST(TEST_CLASS, CanInsertAfterEraseAllMosaics) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		for (auto i = 1u; i <= 10; ++i)
			map.erase(GetMosaicId(i));

		for (auto i = 1u; i <= 3; ++i)
			map.insert(std::make_pair(MosaicId(1000 + i), Amount(10 - i)));

		// Assert:
		AssertContents(map, {
			{ MosaicId(1001), Amount(9) },
			{ MosaicId(1002), Amount(8) },
			{ MosaicId(1003), Amount(7) }
		});
	}

	// endregion

	// region iteration

#define ITERATOR_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_NonConst) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BeginEndTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Const) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BeginEndConstTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ITERATOR_BASED_TEST(CanAdvanceIteratorsPostfixOperator) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		MosaicMap allMosaics;
		for (auto iter = TTraits::begin(map); TTraits::end(map) != iter; iter++)
			allMosaics.insert(*iter);

		// Assert:
		AssertContents(GetTenExpectedMosaics(), allMosaics, "postfix");
	}

	ITERATOR_BASED_TEST(CanAdvanceIteratorsPrefixOperator) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		MosaicMap allMosaics;
		for (auto iter = TTraits::begin(map); TTraits::end(map) != iter; ++iter)
			allMosaics.insert(*iter);

		// Assert:
		AssertContents(GetTenExpectedMosaics(), allMosaics, "prefix");
	}

	ITERATOR_BASED_TEST(CannotAdvanceIteratorsPastEnd) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		EXPECT_THROW(++TTraits::end(map), catapult_out_of_range);
		EXPECT_THROW(TTraits::end(map)++, catapult_out_of_range);
	}

	ITERATOR_BASED_TEST(CannotDereferenceIteratorsAtEnd) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		EXPECT_THROW(*TTraits::end(map), catapult_out_of_range);
		EXPECT_THROW(TTraits::end(map).operator->(), catapult_out_of_range);
	}

	ITERATOR_BASED_TEST(BeginEndIteratorsBasedOnDifferentMapsAreNotEqual) {
		// Arrange:
		CompactMosaicMap map1;
		CompactMosaicMap map2;

		// Act + Assert:
		EXPECT_NE(TTraits::begin(map1), TTraits::begin(map2));
		EXPECT_NE(TTraits::end(map1), TTraits::end(map2));
	}

	// endregion

	// region modification

	TEST(TEST_CLASS, CanModifyAmountViaFind) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 3);

		// Act:
		auto iter = map.find(MosaicId(102));
		ASSERT_NE(map.end(), iter);

		iter->second = Amount(999);

		// Assert:
		AssertContents(map, {
			{ MosaicId(1), Amount(1) },
			{ MosaicId(102), Amount(999) },
			{ MosaicId(3), Amount(9) }
		});
	}

	TEST(TEST_CLASS, CanZeroAndEraseAllMosaics_Forward) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		for (auto i = 1u; i <= 10; ++i)
			map.find(GetMosaicId(i))->second = Amount();

		for (auto i = 1u; i <= 10; ++i)
			map.erase(GetMosaicId(i));

		// Assert:
		AssertEmpty(map);
	}

	TEST(TEST_CLASS, CanZeroAndEraseAllMosaics_Reverse) {
		// Arrange:
		CompactMosaicMap map;
		InsertMany(map, 10);

		// Act:
		for (auto i = 10u; i >= 1; --i)
			map.find(GetMosaicId(i))->second = Amount();

		for (auto i = 10u; i >= 1; --i)
			map.erase(GetMosaicId(i));

		// Assert:
		AssertEmpty(map);
	}

	// endregion
}}
