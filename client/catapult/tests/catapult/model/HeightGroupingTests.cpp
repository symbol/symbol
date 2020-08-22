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

#include "catapult/model/HeightGrouping.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS HeightGroupingTests

	namespace {
		using RawHeightToRawHeightMap = std::map<uint64_t, uint64_t>;

		struct SingleGroupingTraits {
			static constexpr uint64_t Grouping = 1;

			static RawHeightToRawHeightMap GetHeightToImportanceHeightMap() {
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

			static RawHeightToRawHeightMap GetHeightToImportanceHeightMap() {
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

			static RawHeightToRawHeightMap GetHeightToImportanceHeightMap() {
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

			static RawHeightToRawHeightMap GetHeightToImportanceHeightMap() {
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
	}

	// region zero grouping

	TEST(TEST_CLASS, CannotConvertHeightToImportanceHeight_Zero) {
		// Arrange:
		for (const auto& pair : SingleGroupingTraits::GetHeightToImportanceHeightMap()) {
			auto inputHeight = Height(pair.first);

			// Act + Assert:
			EXPECT_THROW(ConvertToImportanceHeight(inputHeight, 0), catapult_invalid_argument) << "for height " << inputHeight;
		}
	}

	// endregion

	// region non-zero grouping

#define GROUPING_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_One) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SingleGroupingTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Dual) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DualGroupingTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Default) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DefaultGroupingTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Custom) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CustomGroupingTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	GROUPING_TRAITS_BASED_TEST(CanConvertHeightToImportanceHeight) {
		// Arrange:
		for (const auto& pair : TTraits::GetHeightToImportanceHeightMap()) {
			auto inputHeight = Height(pair.first);

			// Act:
			auto resultHeight = ConvertToImportanceHeight(inputHeight, TTraits::Grouping);

			// Assert:
			auto expectedResultHeight = ImportanceHeight(pair.second);
			EXPECT_EQ(expectedResultHeight, resultHeight) << "for height " << inputHeight;
		}
	}

	// endregion
}}
