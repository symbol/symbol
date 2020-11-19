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

#include "catapult/deltaset/BaseSetUtils.h"
#include "tests/catapult/deltaset/test/BaseSetTestsInclude.h"
#include "tests/TestHarness.h"
#include <vector>

namespace catapult { namespace deltaset {

#define TEST_CLASS BaseSetUtilsTests

	namespace {
		template<typename TMutabilityTraits>
		using OrderedTraits = test::BaseSetTraits<TMutabilityTraits, test::OrderedSetTraits<test::SetElementType<TMutabilityTraits>>>;

		using NonPointerTraits = test::BaseTraits<OrderedTraits<test::MutableElementValueTraits>>;
	}

#define SET_UTILS_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_NonPointer) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonPointerTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	SET_UTILS_TRAITS_BASED_TEST(ContainsAnyReturnsTrueWhenAtLeastOneElementExists) {
		// Arrange:
		auto pSet = TTraits::Create();
		auto pDelta = pSet->rebase();
		InsertAll(*pDelta, TTraits::CreateElements(3));
		auto elements = typename TTraits::ElementVector{
			TTraits::CreateElement("MyTestElement", 0), // does not exist in delta
			TTraits::CreateElement("TestElement", 1), // exists in delta
			TTraits::CreateElement("AnotherTestElement", 123) // does not exist in delta
		};

		// Act:
		auto found = ContainsAny(*pDelta, elements);

		// Assert:
		EXPECT_TRUE(found);
	}

	SET_UTILS_TRAITS_BASED_TEST(ContainsAnyReturnsFalseWhenNoElementExists) {
		// Arrange:
		auto pSet = TTraits::Create();
		auto elements = typename TTraits::ElementVector{
			TTraits::CreateElement("MyTestElement", 0), // does not exist in delta
			TTraits::CreateElement("Blah", 1), // does not exist in delta
			TTraits::CreateElement("AnotherTestElement", 123) // does not exist in delta
		};

		// Act:
		auto found = ContainsAny(*pSet, elements);

		// Assert:
		EXPECT_FALSE(found);
	}

	SET_UTILS_TRAITS_BASED_TEST(InsertAllInsertsAllElements) {
		// Arrange:
		auto pSet = TTraits::Create();
		auto pDelta = pSet->rebase();
		auto elements = TTraits::CreateElements(3);

		// Sanity check:
		EXPECT_EQ(0u, pDelta->size());

		// Act:
		InsertAll(*pDelta, elements);

		// Assert:
		EXPECT_EQ(3u, pDelta->size());
		TTraits::AssertContents(*pDelta, elements);
	}

	SET_UTILS_TRAITS_BASED_TEST(RemoveAllRemovesAllElements) {
		// Arrange:
		auto pSet = TTraits::Create();
		auto pDelta = pSet->rebase();
		InsertAll(*pDelta, TTraits::CreateElements(5));
		auto elements = typename TTraits::ElementVector{
			TTraits::CreateElement("TestElement", 0),
			TTraits::CreateElement("TestElement", 2),
			TTraits::CreateElement("TestElement", 4)
		};
		auto expectedElements = typename TTraits::ElementVector{
			TTraits::CreateElement("TestElement", 1),
			TTraits::CreateElement("TestElement", 3)
		};

		// Act:
		RemoveAll(*pDelta, elements);

		// Assert:
		EXPECT_EQ(2u, pDelta->size());
		TTraits::AssertContents(*pDelta, expectedElements);
	}
}}
