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

#include "AccountRestrictionTestUtils.h"
#include "src/state/AccountRestrictions.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		void InsertRandomValues(
				state::AccountRestriction& restriction,
				state::AccountRestrictionOperationType operationType,
				size_t count) {
			constexpr auto Add = model::AccountRestrictionModificationType::Add;
			while (restriction.values().size() < count) {
				model::RawAccountRestrictionModification modification{ Add, test::GenerateRandomVector(restriction.valueSize()) };
				if (state::AccountRestrictionOperationType::Allow == operationType)
					restriction.allow(modification);
				else
					restriction.block(modification);
			}
		}
	}

	std::vector<model::AccountRestrictionType> CollectAccountRestrictionTypes() {
		std::vector<model::AccountRestrictionType> restrictionTypes;
		state::AccountRestrictions restrictions(test::GenerateRandomByteArray<Address>());
		for (auto& pair : restrictions)
			restrictionTypes.push_back(pair.first);

		return restrictionTypes;
	}

	state::AccountRestrictions CreateAccountRestrictions(
			state::AccountRestrictionOperationType operationType,
			const std::vector<size_t>& valuesSizes) {
		state::AccountRestrictions restrictions(test::GenerateRandomByteArray<Address>());
		if (valuesSizes.size() != restrictions.size())
			CATAPULT_THROW_INVALID_ARGUMENT_2("values size mismatch", valuesSizes.size(), restrictions.size());

		auto i = 0u;
		auto restrictionTypes = CollectAccountRestrictionTypes();
		for (auto restrictionType : restrictionTypes) {
			auto& restriction = restrictions.restriction(restrictionType);
			InsertRandomValues(restriction, operationType, valuesSizes[i++]);
		}

		// Sanity:
		i = 0;
		for (auto restrictionType : restrictionTypes) {
			EXPECT_EQ(valuesSizes[i], restrictions.restriction(restrictionType).values().size());
			++i;
		}

		return restrictions;
	}

	void AssertEqual(const state::AccountRestrictions& expected, const state::AccountRestrictions& actual) {
		EXPECT_EQ(expected.address(), actual.address());
		EXPECT_EQ(expected.size(), actual.size());

		for (const auto& pair : expected) {
			const auto& expectedAccountRestriction = pair.second;
			auto restrictionType = expectedAccountRestriction.descriptor().directionalRestrictionType();
			const auto& restriction = actual.restriction(restrictionType);

			EXPECT_EQ(expectedAccountRestriction.descriptor().raw(), restriction.descriptor().raw());
			EXPECT_EQ(expectedAccountRestriction.valueSize(), restriction.valueSize());
			EXPECT_EQ(expectedAccountRestriction.values(), restriction.values());
		}
	}
}}
