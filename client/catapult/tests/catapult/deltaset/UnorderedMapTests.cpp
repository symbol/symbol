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
		using UnorderedMapTraits = test::BaseSetTraits<
			TMutabilityTraits,
			test::UnorderedMapSetTraits<test::SetElementType<TMutabilityTraits>>>;

		using UnorderedMapMutableTraits = UnorderedMapTraits<test::MutableElementValueTraits>;
		using UnorderedMapImmutableTraits = UnorderedMapTraits<test::ImmutableElementValueTraits>;
	}

// base (mutable)
DEFINE_MUTABLE_BASE_SET_TESTS_FOR(UnorderedMapMutable)

// base (immutable)
DEFINE_IMMUTABLE_BASE_SET_TESTS_FOR(UnorderedMapImmutable)

// delta (mutable)
DEFINE_MUTABLE_BASE_SET_DELTA_TESTS_FOR(UnorderedMapMutable)

// delta (immutable)
DEFINE_IMMUTABLE_BASE_SET_DELTA_TESTS_FOR(UnorderedMapImmutable)

#define TEST_CLASS UnorderedMapTests

#define MAKE_UNORDERED_MAP_MUTABLE_TEST(TEST_NAME, TYPE) \
	TEST(DeltaUnorderedMap##TYPE##Tests, TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::DeltaTraits<deltaset::UnorderedMap##TYPE##Traits>>(); \
	}

#define UNORDERED_MAP_MUTABLE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	MAKE_UNORDERED_MAP_MUTABLE_TEST(TEST_NAME, Mutable) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	UNORDERED_MAP_MUTABLE_TEST(NonConstFindAllowsElementModification) {
		// Arrange:
		auto pSet = TTraits::CreateBase();
		auto pDelta = pSet->rebase();

		auto element = TTraits::CreateElement("TestElement", 4);
		pDelta->insert(element);
		pSet->commit();

		// Act: mutate can be called
		auto pDeltaElement = pDelta->find(TTraits::ToKey(element)).get();
		pDeltaElement->mutate();

		// Assert:
		EXPECT_FALSE(std::is_const<decltype(test::Unwrap(pDeltaElement))>());
	}
}}
