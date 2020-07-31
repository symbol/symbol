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

#include "tests/catapult/deltaset/test/BaseSetDeltaTests.h"
#include "tests/catapult/deltaset/test/BaseSetTests.h"

namespace catapult { namespace deltaset {

	namespace {
		template<typename TMutabilityTraits>
		using OrderedTraits = test::BaseSetTraits<TMutabilityTraits, test::OrderedSetTraits<test::SetElementType<TMutabilityTraits>>>;

		using OrderedMutableTraits = OrderedTraits<test::MutableElementValueTraits>;
		using OrderedImmutableTraits = OrderedTraits<test::ImmutableElementValueTraits>;
	}

// base (mutable)
DEFINE_MUTABLE_BASE_SET_TESTS_FOR(OrderedMutable)

// base (immutable)
DEFINE_IMMUTABLE_BASE_SET_TESTS_FOR(OrderedImmutable)

// delta (mutable)
DEFINE_MUTABLE_BASE_SET_DELTA_TESTS_FOR(OrderedMutable)

// delta (immutable)
DEFINE_IMMUTABLE_BASE_SET_DELTA_TESTS_FOR(OrderedImmutable)

/* forward tests only use ordered base variants */
#define TEST_CLASS OrderedTests

#define MAKE_ORDERED_TEST(TEST_NAME, TYPE) \
	TEST(BaseOrdered##TYPE##Tests, TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BaseTraits<deltaset::Ordered##TYPE##Traits>>(); \
	}

#define ORDERED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	MAKE_ORDERED_TEST(TEST_NAME, Mutable) \
	MAKE_ORDERED_TEST(TEST_NAME, Immutable) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ORDERED_TEST(OrderedBaseCanIterateThroughSetInOrder) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithElements(3);
		auto pDelta = pBaseSet->rebase();
		pDelta->emplace("TestElement", static_cast<unsigned int>(7));
		pDelta->emplace("TestElement", static_cast<unsigned int>(4));
		pBaseSet->commit();

		auto iterableBaseSet = deltaset::MakeIterableView(*pBaseSet);
		auto iter = iterableBaseSet.begin();

		// Assert:
		EXPECT_EQ(test::TestElement("TestElement", 0), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 1), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 2), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 4), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 7), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(iterableBaseSet.end(), iter);

		// Sanity: the iterator elements are const
		test::AssertConstIterator(*pBaseSet);
	}
}}
