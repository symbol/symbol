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

#include "src/state/TypedAccountProperty.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS TypedAccountPropertyTests

	namespace {
		constexpr auto Add = model::PropertyModificationType::Add;
		constexpr auto Del = model::PropertyModificationType::Del;

		constexpr size_t Custom_Property_Size = 53;
		constexpr auto Custom_Property_Type = static_cast<model::PropertyType>(0x78);
		using CustomProperty = std::array<uint8_t, Custom_Property_Size>;

		using CustomPropertyModifications = std::vector<model::PropertyModification<CustomProperty>>;

		struct TestContext {
		public:
			explicit TestContext(const std::vector<model::PropertyModificationType>& modificationTypes) {
				for (auto modificationType : modificationTypes)
					Modifications.push_back({ modificationType, test::GenerateRandomData<Custom_Property_Size>() });
			}

		public:
			CustomPropertyModifications Modifications;
		};

		TypedAccountProperty<CustomProperty> CreateTypedPropertyWithValues(
				AccountProperty& property,
				OperationType operationType,
				const CustomPropertyModifications& modifications) {
			for (const auto& modification : modifications) {
				if (OperationType::Allow == operationType)
					property.allow({ modification.ModificationType, ToVector(modification.Value) });
				else
					property.block({ modification.ModificationType, ToVector(modification.Value) });
			}

			return TypedAccountProperty<CustomProperty>(property);
		}
	}

	TEST(TEST_CLASS, CanCreateTypedAccountPropertyAroundProperty) {
		// Arrange:
		auto rawProperty = AccountProperty(Custom_Property_Type, 1234);

		// Act:
		auto property = TypedAccountProperty<CustomProperty>(rawProperty);

		// Assert:
		EXPECT_EQ(model::PropertyType(Custom_Property_Type | model::PropertyType::Block), property.descriptor().raw());
		EXPECT_EQ(0u, property.size());
	}

	TEST(TEST_CLASS, ContainsDelegatesToUnderlyingProperty) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto rawProperty = AccountProperty(Custom_Property_Type, Custom_Property_Size);
		auto property = CreateTypedPropertyWithValues(rawProperty, OperationType::Allow, context.Modifications);

		// Act + Assert:
		EXPECT_TRUE(property.contains(context.Modifications[1].Value));
		EXPECT_FALSE(property.contains(test::GenerateRandomData<Custom_Property_Size>()));
	}

	TEST(TEST_CLASS, CanAllowDelegatesToUnderlyingProperty) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto rawProperty = AccountProperty(Custom_Property_Type, Custom_Property_Size);
		auto property = CreateTypedPropertyWithValues(rawProperty, OperationType::Allow, context.Modifications);

		// Act + Assert:
		EXPECT_TRUE(property.canAllow({ Add, test::GenerateRandomData<Custom_Property_Size>() }));
		EXPECT_FALSE(property.canAllow({ Add, context.Modifications[1].Value }));
		EXPECT_TRUE(property.canAllow({ Del, context.Modifications[1].Value }));
		EXPECT_FALSE(property.canAllow({ Del, test::GenerateRandomData<Custom_Property_Size>() }));
	}

	TEST(TEST_CLASS, CanBlockDelegatesToUnderlyingProperty) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto rawProperty = AccountProperty(Custom_Property_Type, Custom_Property_Size);
		auto property = CreateTypedPropertyWithValues(rawProperty, OperationType::Block, context.Modifications);

		// Act + Assert:
		EXPECT_TRUE(property.canBlock({ Add, test::GenerateRandomData<Custom_Property_Size>() }));
		EXPECT_FALSE(property.canBlock({ Add, context.Modifications[1].Value }));
		EXPECT_TRUE(property.canBlock({ Del, context.Modifications[1].Value }));
		EXPECT_FALSE(property.canBlock({ Del, test::GenerateRandomData<Custom_Property_Size>() }));
	}
}}
