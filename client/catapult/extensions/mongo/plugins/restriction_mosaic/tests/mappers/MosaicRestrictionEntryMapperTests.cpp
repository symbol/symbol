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

#include "src/mappers/MosaicRestrictionEntryMapper.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/MosaicRestrictionEntryMapperTestUtils.h"
#include "tests/TestHarness.h"
#include <set>

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MosaicRestrictionEntryMapperTests

	// region ToDbModel

	namespace {
		template<typename TRestrictionTraits>
		void AssertCanMapRestriction(size_t numValues) {
			// Arrange:
			auto restriction = TRestrictionTraits::CreateRestriction(numValues);
			state::MosaicRestrictionEntry restrictionEntry(restriction);

			// Act:
			auto document = ToDbModel(restrictionEntry);
			auto view = document.view();

			// Assert:
			EXPECT_EQ(1u, test::GetFieldCount(view));

			auto dbRestrictionEntry = view["mosaicRestrictionEntry"].get_document().view();
			TRestrictionTraits::AssertEqualRestriction(restrictionEntry, dbRestrictionEntry);
		}
	}

#define RESTRICTION_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::MosaicAddressRestrictionTestTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Global) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::MosaicGlobalRestrictionTestTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	RESTRICTION_TRAITS_BASED_TEST(CanMapRestriction_NoValues) {
		AssertCanMapRestriction<TTraits>(0);
	}

	RESTRICTION_TRAITS_BASED_TEST(CanMapRestriction_SingleValue) {
		AssertCanMapRestriction<TTraits>(1);
	}

	RESTRICTION_TRAITS_BASED_TEST(CanMapRestriction_MultipleValues) {
		AssertCanMapRestriction<TTraits>(5);
	}

	// endregion
}}}
