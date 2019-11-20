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

#include "AccountRestrictionsMapperTestUtils.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/restriction_account/src/state/AccountRestrictions.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	void AssertRestrictionValues(const state::AccountRestriction& restriction, const bsoncxx::document::view& dbRestrictionValues) {
		EXPECT_FALSE(restriction.values().empty());
		EXPECT_EQ(restriction.values().size(), test::GetFieldCount(dbRestrictionValues));
		for (const auto& dbRestrictionValue : dbRestrictionValues) {
			auto valueSize = restriction.valueSize();
			ASSERT_EQ(valueSize, dbRestrictionValue.get_binary().size);

			std::vector<uint8_t> value(valueSize);
			std::memcpy(value.data(), dbRestrictionValue.get_binary().bytes, valueSize);
			EXPECT_TRUE(restriction.contains(value));
		}
	}

	void AssertEqualAccountRestrictionsData(
			const state::AccountRestrictions& restrictions,
			const bsoncxx::document::view& dbAccountRestrictions) {
		EXPECT_EQ(restrictions.address(), test::GetAddressValue(dbAccountRestrictions, "address"));

		// note that only restrictions that are not empty should have been saved to the mongo db
		auto dbRestrictions = dbAccountRestrictions["restrictions"].get_array().value;
		auto numRestrictionsWithValues = 0u;
		for (const auto& pair : restrictions)
			numRestrictionsWithValues += pair.second.values().empty() ? 0 : 1;

		ASSERT_EQ(numRestrictionsWithValues, test::GetFieldCount(dbRestrictions));

		for (const auto& dbRestriction : dbRestrictions) {
			auto dbRestrictionView = dbRestriction.get_document().view();
			auto restrictionFlags = model::AccountRestrictionFlags(test::GetUint32(dbRestrictionView, "restrictionFlags"));
			auto descriptor = state::AccountRestrictionDescriptor(restrictionFlags);
			const auto& restriction = restrictions.restriction(restrictionFlags);
			EXPECT_EQ(restriction.descriptor().raw(), descriptor.raw());

			AssertRestrictionValues(restriction, dbRestrictionView["values"].get_array().value);
		}
	}
}}
