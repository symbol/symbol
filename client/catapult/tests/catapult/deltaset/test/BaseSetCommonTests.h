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

#pragma once
#include "BaseSetIterationTests.h"

namespace catapult { namespace test {

	/// All tests that apply to both BaseSet and BaseSetDelta.
	template<typename TTraits>
	class BaseSetCommonTests {
	private:
		using BaseSetType = std::remove_reference_t<decltype(*TTraits::Create())>;

	public:
		// region ctor

		static void AssertCanCreateBaseSet() {
			// Act:
			auto pSet = TTraits::Create();

			// Assert:
			TTraits::AssertContents(*pSet, typename TTraits::ElementVector());
		}

		static void AssertCanCreateBaseSetWithForwardedArguments() {
			// Arrange: create an underlying set with three elements
			typename BaseSetType::SetType underlyingSet{
				TTraits::ToStorage(TTraits::CreateElement("alpha", 1)),
				TTraits::ToStorage(TTraits::CreateElement("beta", 4)),
				TTraits::ToStorage(TTraits::CreateElement("gamma", 9))
			};

			// Act: create a base set around it
			auto pSet = TTraits::Create(underlyingSet);

			// Assert:
			TTraits::AssertContents(*pSet, typename TTraits::ElementVector{
				TTraits::CreateElement("alpha", 1),
				TTraits::CreateElement("beta", 4),
				TTraits::CreateElement("gamma", 9)
			});
		}

		// endregion

		// region empty

		static void AssertEmptyReturnsTrueForEmptySet() {
			// Act:
			auto pSet = TTraits::Create();

			// Assert:
			EXPECT_TRUE(pSet->empty());
		}

		static void AssertEmptyReturnsFalseForSetWithValues() {
			// Act:
			auto pSet = TTraits::CreateWithElements(3);

			// Assert:
			EXPECT_FALSE(pSet->empty());
		}

		// endregion

		// region contains

		static void AssertContainsReturnsTrueForExistingElement() {
			// Arrange:
			auto pSet = TTraits::CreateWithElements(3);
			auto element = TTraits::CreateElement("TestElement", 2);

			// Act:
			auto found = pSet->contains(TTraits::ToKey(element));

			// Assert:
			EXPECT_TRUE(found);
		}

		static void AssertContainsReturnsFalseForNonexistentElement() {
			// Arrange:
			auto pSet = TTraits::CreateWithElements(3);
			auto element1 = TTraits::CreateElement("TestElement", 123);
			auto element2 = TTraits::CreateElement("BadElement", 2);

			// Act:
			auto found1 = pSet->contains(TTraits::ToKey(element1));
			auto found2 = pSet->contains(TTraits::ToKey(element2));

			// Assert:
			EXPECT_FALSE(found1);
			EXPECT_FALSE(found2);
		}

		// endregion

		// region find

		static void AssertCanFindExistingElement() {
			// Arrange:
			auto pSet = TTraits::CreateWithElements(3);
			auto element = TTraits::CreateElement("TestElement", 2);

			// Act:
			auto pFoundElement = pSet->find(TTraits::ToKey(element)).get();

			// Assert:
			EXPECT_EQ(3u, pSet->size());
			ASSERT_TRUE(!!pFoundElement);
			EXPECT_EQ(*TTraits::ToPointer(element), *pFoundElement);
		}

		static void AssertCanFindExistingElementMultipleTimes() {
			// Arrange:
			auto pSet = TTraits::CreateWithElements(3);
			auto element = TTraits::CreateElement("TestElement", 2);

			// Act:
			auto pFoundElement1 = pSet->find(TTraits::ToKey(element)).get();
			auto pFoundElement2 = pSet->find(TTraits::ToKey(element)).get();

			// Assert:
			EXPECT_EQ(3u, pSet->size());
			ASSERT_TRUE(!!pFoundElement1);
			ASSERT_TRUE(!!pFoundElement2);
			EXPECT_EQ(pFoundElement1, pFoundElement2);
			EXPECT_EQ(*TTraits::ToPointer(element), *pFoundElement1);
			EXPECT_EQ(*TTraits::ToPointer(element), *pFoundElement2);
		}

		static void AssertCannotFindNonexistentElement() {
			// Arrange:
			auto pSet = TTraits::CreateWithElements(3);
			auto element1 = TTraits::CreateElement("TestElement", 123);
			auto element2 = TTraits::CreateElement("BadElement", 2);

			// Act:
			auto pFoundElement1 = pSet->find(TTraits::ToKey(element1)).get();
			auto pFoundElement2 = pSet->find(TTraits::ToKey(element2)).get();

			// Assert:
			EXPECT_EQ(3u, pSet->size());
			EXPECT_FALSE(!!pFoundElement1);
			EXPECT_FALSE(!!pFoundElement2);
		}

		// endregion
	};

#define MAKE_COMMON_BASE_SET_TEST(TEST_CLASS, TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::BaseSetCommonTests<TRAITS>::Assert##TEST_NAME(); }

#define DEFINE_COMMON_BASE_SET_TESTS(TEST_CLASS, TRAITS) \
	MAKE_COMMON_BASE_SET_TEST(TEST_CLASS, TRAITS, CanCreateBaseSet) \
	MAKE_COMMON_BASE_SET_TEST(TEST_CLASS, TRAITS, CanCreateBaseSetWithForwardedArguments) \
	\
	MAKE_COMMON_BASE_SET_TEST(TEST_CLASS, TRAITS, EmptyReturnsTrueForEmptySet) \
	MAKE_COMMON_BASE_SET_TEST(TEST_CLASS, TRAITS, EmptyReturnsFalseForSetWithValues) \
	\
	MAKE_COMMON_BASE_SET_TEST(TEST_CLASS, TRAITS, ContainsReturnsTrueForExistingElement) \
	MAKE_COMMON_BASE_SET_TEST(TEST_CLASS, TRAITS, ContainsReturnsFalseForNonexistentElement) \
	\
	MAKE_COMMON_BASE_SET_TEST(TEST_CLASS, TRAITS, CanFindExistingElement) \
	MAKE_COMMON_BASE_SET_TEST(TEST_CLASS, TRAITS, CanFindExistingElementMultipleTimes) \
	MAKE_COMMON_BASE_SET_TEST(TEST_CLASS, TRAITS, CannotFindNonexistentElement)
}}
