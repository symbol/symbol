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

#include "AccountPropertiesTestUtils.h"
#include "src/state/AccountProperties.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		void InsertRandomValues(state::AccountProperty& property, state::OperationType operationType, size_t count) {
			constexpr auto Add = model::PropertyModificationType::Add;
			while (property.values().size() < count) {
				model::RawPropertyModification modification{ Add, test::GenerateRandomVector(property.propertyValueSize()) };
				if (state::OperationType::Allow == operationType)
					property.allow(modification);
				else
					property.block(modification);
			}
		}
	}

	std::vector<model::PropertyType> CollectPropertyTypes() {
		std::vector<model::PropertyType> propertyTypes;
		state::AccountProperties accountProperties(test::GenerateRandomData<Address_Decoded_Size>());
		for (auto& pair : accountProperties)
			propertyTypes.push_back(pair.first);

		return propertyTypes;
	}

	state::AccountProperties CreateAccountProperties(state::OperationType operationType, const std::vector<size_t>& valuesSizes) {
		state::AccountProperties accountProperties(test::GenerateRandomData<Address_Decoded_Size>());
		if (valuesSizes.size() != accountProperties.size())
			CATAPULT_THROW_INVALID_ARGUMENT_2("values size mismatch", valuesSizes.size(), accountProperties.size());

		auto i = 0u;
		auto propertyTypes = CollectPropertyTypes();
		for (auto propertyType : propertyTypes) {
			auto& property = accountProperties.property(propertyType);
			InsertRandomValues(property, operationType, valuesSizes[i++]);
		}

		// Sanity:
		i = 0;
		for (auto propertyType : propertyTypes) {
			EXPECT_EQ(valuesSizes[i], accountProperties.property(propertyType).values().size());
			++i;
		}

		return accountProperties;
	}

	void AssertEqual(const state::AccountProperties& expected, const state::AccountProperties& actual) {
		EXPECT_EQ(expected.address(), actual.address());
		EXPECT_EQ(expected.size(), actual.size());

		for (const auto& pair : expected) {
			const auto& expectedAccountProperty = pair.second;
			auto propertyType = expectedAccountProperty.descriptor().propertyType();
			const auto& accountProperty = actual.property(propertyType);

			EXPECT_EQ(expectedAccountProperty.descriptor().raw(), accountProperty.descriptor().raw());
			EXPECT_EQ(expectedAccountProperty.propertyValueSize(), accountProperty.propertyValueSize());
			EXPECT_EQ(expectedAccountProperty.values(), accountProperty.values());
		}
	}
}}
