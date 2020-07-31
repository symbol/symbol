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
		using ReverseOrderedTraits = test::BaseSetTraits<
			TMutabilityTraits,
			test::ReverseOrderedSetTraits<test::SetElementType<TMutabilityTraits>>>;

		using ReverseOrderedMutableTraits = ReverseOrderedTraits<test::MutableElementValueTraits>;
		using ReverseOrderedImmutableTraits = ReverseOrderedTraits<test::ImmutableElementValueTraits>;
	}

// base (mutable)
DEFINE_MUTABLE_BASE_SET_TESTS_FOR(ReverseOrderedMutable)

// base (immutable)
DEFINE_IMMUTABLE_BASE_SET_TESTS_FOR(ReverseOrderedImmutable)

// delta (mutable)
DEFINE_MUTABLE_BASE_SET_DELTA_TESTS_FOR(ReverseOrderedMutable)

// delta (immutable)
DEFINE_IMMUTABLE_BASE_SET_DELTA_TESTS_FOR(ReverseOrderedImmutable)

/* reverse tests only use ordered base variants */
#define TEST_CLASS ReverseOrderedTests

#define MAKE_REVERSE_ORDERED_TEST(TEST_NAME, TYPE) \
	TEST(BaseReverseOrdered##TYPE##Tests, TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BaseTraits<deltaset::ReverseOrdered##TYPE##Traits>>(); \
	}

#define REVERSE_ORDERED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	MAKE_REVERSE_ORDERED_TEST(TEST_NAME, Mutable) \
	MAKE_REVERSE_ORDERED_TEST(TEST_NAME, Immutable) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	REVERSE_ORDERED_TEST(ReverseOrderedBaseCanIterateThroughSetInReverseOrder) {
		// Arrange:
		auto pReversedBaseSet = TTraits::CreateWithElements(3);
		auto pDelta = pReversedBaseSet->rebase();
		pDelta->emplace("TestElement", static_cast<unsigned int>(7));
		pDelta->emplace("TestElement", static_cast<unsigned int>(4));
		pReversedBaseSet->commit();

		auto iterableBaseSet = deltaset::MakeIterableView(*pReversedBaseSet);
		auto iter = iterableBaseSet.begin();

		// Assert:
		EXPECT_EQ(test::TestElement("TestElement", 7), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 4), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 2), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 1), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 0), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(iterableBaseSet.end(), iter);

		// Sanity: the iterator elements are const
		test::AssertConstIterator(*pReversedBaseSet);
	}
}}
