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

#include "AccountPropertiesMapperTestUtils.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/property/src/state/AccountProperties.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

		void AssertPropertyValues(const state::AccountProperty& property, const bsoncxx::document::view& dbPropertyValues) {
			EXPECT_EQ(property.values().size(), test::GetFieldCount(dbPropertyValues));
			for (const auto& dbPropertyValue : dbPropertyValues) {
				auto valueSize = property.propertyValueSize();
				ASSERT_EQ(valueSize, dbPropertyValue.get_binary().size);

				std::vector<uint8_t> value(valueSize);
				std::memcpy(value.data(), dbPropertyValue.get_binary().bytes, valueSize);
				EXPECT_TRUE(property.contains(value));
			}
		}

		void AssertEqualAccountPropertiesData(
				const state::AccountProperties& accountProperties,
				const bsoncxx::document::view& dbAccountProperties) {
			EXPECT_EQ(accountProperties.address(), test::GetAddressValue(dbAccountProperties, "address"));

			auto dbProperties = dbAccountProperties["properties"].get_array().value;
			ASSERT_EQ(accountProperties.size(), test::GetFieldCount(dbProperties));

			for (const auto& dbProperty : dbProperties) {
				auto dbPropertyView = dbProperty.get_document().view();
				auto propertyType = model::PropertyType(test::GetUint8(dbPropertyView, "propertyType"));
				auto descriptor = state::PropertyDescriptor(propertyType);
				const auto& property = accountProperties.property(propertyType);
				EXPECT_EQ(property.descriptor().raw(), descriptor.raw());

				AssertPropertyValues(property, dbPropertyView["values"].get_array().value);
			}
		}
}}
