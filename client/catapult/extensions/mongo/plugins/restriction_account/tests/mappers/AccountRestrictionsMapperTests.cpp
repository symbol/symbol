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

#include "src/mappers/AccountRestrictionsMapper.h"
#include "plugins/txes/restriction_account/src/state/AccountRestriction.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/AccountRestrictionTestUtils.h"
#include "tests/test/AccountRestrictionsMapperTestUtils.h"
#include "tests/TestHarness.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <set>

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS AccountRestrictionsMapperTests

	namespace {
		void AssertDbView(
				const state::AccountRestrictions& restrictions,
				const bsoncxx::document::view& view,
				size_t numExpectedRestrictionValues) {
			// Assert:
			EXPECT_EQ(1u, test::GetFieldCount(view));

			auto accountRestrictionsView = view["accountRestrictions"].get_document().view();

			size_t numRestrictionValues = 0u;
			for (const auto& pair : restrictions)
				numRestrictionValues += pair.second.values().size();

			EXPECT_EQ(numExpectedRestrictionValues, numRestrictionValues);

			test::AssertEqualAccountRestrictionsData(restrictions, accountRestrictionsView);
		}
	}

	// region ToDbModel

	namespace {
		void AssertCanMapAccountRestrictions(const std::vector<size_t>& valueSizes, size_t numExpectedRestrictionValues) {
			// Arrange:
			auto restrictions = test::CreateAccountRestrictions(state::AccountRestrictionOperationType::Allow, valueSizes);

			// Act:
			auto document = ToDbModel(restrictions);
			auto view = document.view();

			// Assert:
			AssertDbView(restrictions, view, numExpectedRestrictionValues);
		}
	}

	TEST(TEST_CLASS, CanMapAccountRestrictionsWithAllRestrictionsEmpty_ModelToDbModel) {
		AssertCanMapAccountRestrictions({ 0, 0, 0, 0 }, 0);
	}

	TEST(TEST_CLASS, CanMapAccountRestrictionsWithSingleRestrictionNotEmpty_ModelToDbModel) {
		AssertCanMapAccountRestrictions({ 5, 0, 0, 0 }, 5);
		AssertCanMapAccountRestrictions({ 0, 5, 0, 0 }, 5);
		AssertCanMapAccountRestrictions({ 0, 0, 5, 0 }, 5);
		AssertCanMapAccountRestrictions({ 0, 0, 0, 5 }, 5);
	}

	TEST(TEST_CLASS, CanMapAccountRestrictionsWithNoRestrictionsEmpty_ModelToDbModel) {
		AssertCanMapAccountRestrictions({ 5, 7, 11, 3 }, 26);
	}

	// endregion
}}}
