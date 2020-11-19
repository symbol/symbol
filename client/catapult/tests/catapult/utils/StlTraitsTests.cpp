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

#include "catapult/utils/traits/StlTraits.h"
#include "tests/TestHarness.h"
#include <set>
#include <unordered_set>

namespace catapult { namespace utils {

#define TEST_CLASS StlTraitsTests

	// region is_map

	namespace {
		template<typename T>
		void AssertIsMap(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_TRUE(traits::is_map<T>::value) << lineNumber;
			EXPECT_TRUE(traits::is_map<const T>::value) << lineNumber;

			EXPECT_TRUE(traits::is_map_v<T>) << lineNumber;
			EXPECT_TRUE(traits::is_map_v<const T>) << lineNumber;
		}

		template<typename T>
		void AssertIsNotMap(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_FALSE(traits::is_map<T>::value) << lineNumber;
			EXPECT_FALSE(traits::is_map<const T>::value) << lineNumber;

			EXPECT_FALSE(traits::is_map_v<T>) << lineNumber;
			EXPECT_FALSE(traits::is_map_v<const T>) << lineNumber;
		}

		struct CustomIntHasher {};
	}

	TEST(TEST_CLASS, IsMapReturnsTrueOnlyForOrderedAndUnorderedMaps) {
		// Assert: non maps
		AssertIsNotMap<int>(__LINE__);
		AssertIsNotMap<std::set<int>>(__LINE__);
		AssertIsNotMap<std::unordered_set<int>>(__LINE__);

		// - map related types
		AssertIsNotMap<std::shared_ptr<std::map<int, int>>>(__LINE__);
		AssertIsNotMap<const std::map<int, int>&>(__LINE__);

		// - ordered maps
		AssertIsMap<std::map<int, int>>(__LINE__);
		AssertIsMap<std::map<std::string, double>>(__LINE__);
		AssertIsMap<std::map<int, int, std::greater<int>>>(__LINE__);

		// - unordered maps
		AssertIsMap<std::unordered_map<int, int>>(__LINE__);
		AssertIsMap<std::unordered_map<std::string, double>>(__LINE__);
		AssertIsMap<std::unordered_map<int, int, CustomIntHasher>>(__LINE__);
	}

	// endregion

	// region is_ordered

	namespace {
		template<typename T>
		void AssertIsOrdered(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_TRUE(traits::is_ordered<T>::value) << lineNumber;
			EXPECT_TRUE(traits::is_ordered<const T>::value) << lineNumber;

			EXPECT_TRUE(traits::is_ordered_v<T>) << lineNumber;
			EXPECT_TRUE(traits::is_ordered_v<const T>) << lineNumber;
		}

		template<typename T>
		void AssertIsNotOrdered(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_FALSE(traits::is_ordered<T>::value) << lineNumber;
			EXPECT_FALSE(traits::is_ordered<const T>::value) << lineNumber;

			EXPECT_FALSE(traits::is_ordered_v<T>) << lineNumber;
			EXPECT_FALSE(traits::is_ordered_v<const T>) << lineNumber;
		}
	}

	TEST(TEST_CLASS, IsOrderedReturnsTrueOnlyForOrderedSetsAndOrderedMaps) {
		// Assert: neither maps nor sets
		AssertIsNotOrdered<int>(__LINE__);

		// - set and map related types
		AssertIsNotOrdered<std::shared_ptr<std::set<int>>>(__LINE__);
		AssertIsNotOrdered<const std::set<int>&>(__LINE__);
		AssertIsNotOrdered<std::shared_ptr<std::map<int, int>>>(__LINE__);
		AssertIsNotOrdered<const std::map<int, int>&>(__LINE__);

		// - ordered sets
		AssertIsOrdered<std::set<int>>(__LINE__);
		AssertIsOrdered<std::set<std::string>>(__LINE__);
		AssertIsOrdered<std::set<int, CustomIntHasher>>(__LINE__);

		// - unordered sets
		AssertIsNotOrdered<std::unordered_set<int, int>>(__LINE__);
		AssertIsNotOrdered<std::unordered_set<std::string>>(__LINE__);
		AssertIsNotOrdered<std::unordered_set<int, CustomIntHasher>>(__LINE__);

		// - ordered maps
		AssertIsOrdered<std::map<int, int>>(__LINE__);
		AssertIsOrdered<std::map<std::string, double>>(__LINE__);
		AssertIsOrdered<std::map<int, int, std::greater<int>>>(__LINE__);

		// - unordered maps
		AssertIsNotOrdered<std::unordered_map<int, int>>(__LINE__);
		AssertIsNotOrdered<std::unordered_map<std::string, double>>(__LINE__);
		AssertIsNotOrdered<std::unordered_map<int, int, CustomIntHasher>>(__LINE__);
	}

	// endregion
}}
