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

#include "catapult/utils/ContainerHelpers.h"
#include "tests/TestHarness.h"
#include <unordered_map>

namespace catapult { namespace utils {

#define TEST_CLASS ContainerHelpersTests

	namespace {
		std::unordered_map<uint32_t, uint32_t> CreateMap(size_t count) {
			std::unordered_map<uint32_t, uint32_t> map;
			for (auto i = 0u; i < count; ++i)
				map.emplace(i, i * i);

			return map;
		}

		void AssertMapContents(const std::unordered_map<uint32_t, uint32_t>& map, std::vector<uint32_t> expectedElements) {
			ASSERT_EQ(expectedElements.size(), map.size());
			for (auto i : expectedElements) {
				auto message = "element with key " + std::to_string(i);

				auto iter = map.find(i);
				ASSERT_NE(map.cend(), iter) << message;
				EXPECT_EQ(i, iter->first) << message;
				EXPECT_EQ(i * i, iter->second) << message;
			}
		}
	}

	TEST(TEST_CLASS, NoElementsAreRemovedWhenPredicateAlwaysReturnsFalse) {
		// Arrange:
		auto map = CreateMap(5);

		// Act:
		utils::map_erase_if(map, [](const auto&) { return false; });

		// Assert:
		AssertMapContents(map, { 0u, 1u, 2u, 3u, 4u });
	}

	TEST(TEST_CLASS, ElementsAreRemovedOnlyWhenPredicateReturnsTrue) {
		// Arrange:
		auto map = CreateMap(5);

		// Act: remove even elements
		utils::map_erase_if(map, [](const auto& pair) { return 0 == pair.first % 2; });

		// Assert:
		AssertMapContents(map, { 1u, 3u });
	}

	TEST(TEST_CLASS, AllElementsAreRemovedWhenPredicateAlwaysReturnsTrue) {
		// Arrange:
		auto map = CreateMap(5);

		// Act: remove all elements
		utils::map_erase_if(map, [](const auto&) { return true; });

		// Assert:
		AssertMapContents(map, {});
	}
}}
