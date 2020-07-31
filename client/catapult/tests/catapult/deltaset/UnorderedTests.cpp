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
		using UnorderedTraits = test::BaseSetTraits<TMutabilityTraits, test::UnorderedSetTraits<test::SetElementType<TMutabilityTraits>>>;

		using UnorderedMutableTraits = UnorderedTraits<test::MutableElementValueTraits>;
		using UnorderedImmutableTraits = UnorderedTraits<test::ImmutableElementValueTraits>;
	}

// base (mutable)
DEFINE_MUTABLE_BASE_SET_TESTS_FOR(UnorderedMutable)

// base (immutable)
DEFINE_IMMUTABLE_BASE_SET_TESTS_FOR(UnorderedImmutable)

// delta (mutable)
DEFINE_MUTABLE_BASE_SET_DELTA_TESTS_FOR(UnorderedMutable)

// delta (immutable)
DEFINE_IMMUTABLE_BASE_SET_DELTA_TESTS_FOR(UnorderedImmutable)

/* hasher tests only use unordered delta variants */
#define TEST_CLASS UnorderedTests

#define MAKE_UNORDERED_TEST(TEST_NAME, TYPE) \
	TEST(DeltaUnordered##TYPE##Tests, TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::DeltaTraits<deltaset::Unordered##TYPE##Traits>>(); \
	}

#define UNORDERED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	MAKE_UNORDERED_TEST(TEST_NAME, Mutable) \
	MAKE_UNORDERED_TEST(TEST_NAME, Immutable) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	UNORDERED_TEST(SuppliedHasherIsUsed) {
		// Arrange:
		auto pDelta = TTraits::Create();
		auto element = TTraits::CreateElement("", 0);

		// Act:
		pDelta->insert(element);

		// Assert:
		EXPECT_LE(1u, TTraits::ToPointer(element)->HasherCallCount);
	}
}}
