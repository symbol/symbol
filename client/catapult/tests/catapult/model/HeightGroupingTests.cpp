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

#include "catapult/model/HeightGrouping.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS HeightGroupingTests

	namespace {
		using RawHeightToRawHeightMap = std::map<uint64_t, uint64_t>;

		struct GroupedHeight_tag {};
		using GroupedHeight = utils::BaseValue<Height::ValueType, GroupedHeight_tag>;

		// region traits

		struct SingleGroupingTraits {
			static constexpr uint64_t Grouping = 1;

			static RawHeightToRawHeightMap GetHeightToGroupedHeightMap() {
				return {
					{ 1, 1 },
					{ 2, 1 },
					{ 358, 357 },
					{ 359, 358 },
					{ 360, 359 },
					{ 361, 360 },
					{ 1074, 1073 },
					{ 1095, 1094 }
				};
			}
		};

		struct DualGroupingTraits {
			static constexpr uint64_t Grouping = 2;

			static RawHeightToRawHeightMap GetHeightToGroupedHeightMap() {
				return {
					{ 1, 1 },
					{ 2, 1 },
					{ 3, 2 },
					{ 4, 2 },
					{ 5, 4 },
					{ 358, 356 },
					{ 359, 358 },
					{ 360, 358 },
					{ 361, 360 },
					{ 1074, 1072 },
					{ 1095, 1094 }
				};
			}
		};

		struct DefaultGroupingTraits {
			static constexpr uint64_t Grouping = 359;

			static RawHeightToRawHeightMap GetHeightToGroupedHeightMap() {
				return {
					{ 1, 1 },
					{ 2, 1 },
					{ 358, 1 },
					{ 359, 1 },
					{ 360, 359 },
					{ 361, 359 },
					{ 1074, 718 },
					{ 1095, 1077 }
				};
			}
		};

		struct CustomGroupingTraits {
			static constexpr uint64_t Grouping = 123;

			static RawHeightToRawHeightMap GetHeightToGroupedHeightMap() {
				return {
					{ 1, 1 },
					{ 2, 1 },
					{ 122, 1 },
					{ 123, 1 },
					{ 124, 123 },
					{ 125, 123 },
					{ 365, 246 },
					{ 400, 369 }
				};
			}
		};

		// endregion
	}

	// region CalculateGroupedHeight - zero grouping

	TEST(TEST_CLASS, CannotConvertHeightToGroupedHeight_Zero) {
		// Arrange:
		for (const auto& pair : SingleGroupingTraits::GetHeightToGroupedHeightMap()) {
			auto inputHeight = Height(pair.first);

			// Act + Assert:
			EXPECT_THROW(CalculateGroupedHeight<GroupedHeight>(inputHeight, 0), catapult_invalid_argument) << "for height " << inputHeight;
		}
	}

	// endregion

	// region CalculateGroupedHeight - non-zero grouping

#define GROUPING_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_One) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SingleGroupingTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Dual) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DualGroupingTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Default) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DefaultGroupingTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Custom) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CustomGroupingTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	GROUPING_TRAITS_BASED_TEST(CanConvertHeightToGroupedHeight) {
		// Arrange:
		for (const auto& pair : TTraits::GetHeightToGroupedHeightMap()) {
			auto inputHeight = Height(pair.first);

			// Act:
			auto resultHeight = CalculateGroupedHeight<GroupedHeight>(inputHeight, TTraits::Grouping);

			// Assert:
			auto expectedResultHeight = GroupedHeight(pair.second);
			EXPECT_EQ(expectedResultHeight, resultHeight) << "for height " << inputHeight;
		}
	}

	// endregion

	// region HeightGroupingFacade

	TEST(TEST_CLASS, HeightGroupingFacade_CannotCreateAroundInvalidGroupedHeight) {
		for (auto rawHeight : std::initializer_list<Height::ValueType>{ 0, 2, 99, 101, 150 })
			EXPECT_THROW(HeightGroupingFacade<GroupedHeight>(GroupedHeight(rawHeight), 100), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, HeightGroupingFacade_CannotCreateAroundZeroGrouping) {
		for (auto rawHeight : std::initializer_list<Height::ValueType>{ 1, 100, 200 })
			EXPECT_THROW(HeightGroupingFacade<GroupedHeight>(GroupedHeight(rawHeight), 0), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, HeightGroupingFacade_CanMovePreviousFromNemesis) {
		// Arrange:
		HeightGroupingFacade<GroupedHeight> facade(GroupedHeight(1), 100);

		// Act + Assert:
		EXPECT_EQ(GroupedHeight(1), facade.previous(0));
		EXPECT_EQ(GroupedHeight(1), facade.previous(1));
		EXPECT_EQ(GroupedHeight(1), facade.previous(7));
		EXPECT_EQ(GroupedHeight(1), facade.previous(10));
		EXPECT_EQ(GroupedHeight(1), facade.previous(100));
	}

	TEST(TEST_CLASS, HeightGroupingFacade_CanMoveNextFromNemesis) {
		// Arrange:
		HeightGroupingFacade<GroupedHeight> facade(GroupedHeight(1), 100);

		// Act + Assert:
		EXPECT_EQ(GroupedHeight(1), facade.next(0));
		EXPECT_EQ(GroupedHeight(100), facade.next(1));
		EXPECT_EQ(GroupedHeight(700), facade.next(7));
		EXPECT_EQ(GroupedHeight(1000), facade.next(10));
		EXPECT_EQ(GroupedHeight(10000), facade.next(100));
	}

	TEST(TEST_CLASS, HeightGroupingFacade_CanMoveNextFromNemesisWhenGroupingHeightIsOne) {
		// Arrange:
		HeightGroupingFacade<GroupedHeight> facade(GroupedHeight(1), 1);

		// Act + Assert:
		EXPECT_EQ(GroupedHeight(1), facade.next(0));
		EXPECT_EQ(GroupedHeight(2), facade.next(1));
		EXPECT_EQ(GroupedHeight(8), facade.next(7));
		EXPECT_EQ(GroupedHeight(11), facade.next(10));
		EXPECT_EQ(GroupedHeight(101), facade.next(100));
	}

	TEST(TEST_CLASS, HeightGroupingFacade_CanMovePreviousFromNonNemesis) {
		// Arrange:
		HeightGroupingFacade<GroupedHeight> facade(GroupedHeight(900), 100);

		// Act + Assert:
		EXPECT_EQ(GroupedHeight(900), facade.previous(0));
		EXPECT_EQ(GroupedHeight(800), facade.previous(1));
		EXPECT_EQ(GroupedHeight(200), facade.previous(7));
		EXPECT_EQ(GroupedHeight(100), facade.previous(8));
		EXPECT_EQ(GroupedHeight(1), facade.previous(9));
		EXPECT_EQ(GroupedHeight(1), facade.previous(10));
		EXPECT_EQ(GroupedHeight(1), facade.previous(100));
	}

	TEST(TEST_CLASS, HeightGroupingFacade_CanMoveNextFromNonNemesis) {
		// Arrange:
		HeightGroupingFacade<GroupedHeight> facade(GroupedHeight(900), 100);

		// Act + Assert:
		EXPECT_EQ(GroupedHeight(900), facade.next(0));
		EXPECT_EQ(GroupedHeight(1000), facade.next(1));
		EXPECT_EQ(GroupedHeight(1600), facade.next(7));
		EXPECT_EQ(GroupedHeight(1700), facade.next(8));
		EXPECT_EQ(GroupedHeight(1800), facade.next(9));
		EXPECT_EQ(GroupedHeight(1900), facade.next(10));
		EXPECT_EQ(GroupedHeight(10900), facade.next(100));
	}

	TEST(TEST_CLASS, HeightGroupingFacade_CanMoveNextFromNonNemesisWhenGroupingHeightIsOne) {
		// Arrange:
		HeightGroupingFacade<GroupedHeight> facade(GroupedHeight(900), 1);

		// Act + Assert:
		EXPECT_EQ(GroupedHeight(900), facade.next(0));
		EXPECT_EQ(GroupedHeight(901), facade.next(1));
		EXPECT_EQ(GroupedHeight(907), facade.next(7));
		EXPECT_EQ(GroupedHeight(908), facade.next(8));
		EXPECT_EQ(GroupedHeight(909), facade.next(9));
		EXPECT_EQ(GroupedHeight(910), facade.next(10));
		EXPECT_EQ(GroupedHeight(1000), facade.next(100));
	}

	// endregion
}}
