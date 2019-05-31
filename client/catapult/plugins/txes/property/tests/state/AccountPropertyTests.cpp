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

#include "src/state/AccountProperty.h"
#include "src/state/PropertyUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountPropertyTests

	namespace {
		constexpr auto Add = model::PropertyModificationType::Add;
		constexpr size_t Custom_Property_Size = 53;
		constexpr auto Custom_Property_Type = static_cast<model::PropertyType>(0x78);

		using CustomProperty = std::array<uint8_t, Custom_Property_Size>;
		using CustomPropertyModifications = std::vector<model::PropertyModification<CustomProperty>>;
		using RawPropertyValue = AccountProperty::RawPropertyValue;
		using RawPropertyValues = std::vector<RawPropertyValue>;

		struct TestContext {
		public:
			explicit TestContext(const std::vector<model::PropertyModificationType>& modificationTypes) {
				for (auto modificationType : modificationTypes) {
					Modifications.push_back({ modificationType, test::GenerateRandomArray<Custom_Property_Size>() });
					Values.push_back(Modifications.back().Value);
				}
			}

		public:
			CustomPropertyModifications Modifications;
			std::vector<CustomProperty> Values;
		};

		RawPropertyValues ExtractRawPropertyValues(const CustomPropertyModifications& modifications) {
			RawPropertyValues values;
			for (const auto& modification : modifications)
				values.push_back(ToVector(modification.Value));

			return values;
		}

		AccountProperty CreateWithOperationType(OperationType operationType, const CustomPropertyModifications& modifications) {
			auto propertyType = OperationType::Allow == operationType
					? Custom_Property_Type
					: Custom_Property_Type | model::PropertyType::Block;
			AccountProperty property(propertyType, Custom_Property_Size);
			for (const auto& modification : modifications) {
				if (OperationType::Allow == operationType)
					property.allow({ modification.ModificationType, ToVector(modification.Value) });
				else
					property.block({ modification.ModificationType, ToVector(modification.Value) });
			}

			return property;
		}

		// region AllowTraits / BlockTraits

		struct AllowTraits {
			static AccountProperty Create(const CustomPropertyModifications& modifications) {
				return CreateWithOperationType(DefaultOperationType(), modifications);
			}

			static AccountProperty CreateWithOppositeOperationType(const CustomPropertyModifications& modifications) {
				return CreateWithOperationType(OppositeOperationType(), modifications);
			}

			static OperationType DefaultOperationType() {
				return OperationType::Allow;
			}

			static OperationType OppositeOperationType() {
				return OperationType::Block;
			}

			static bool CanAdd(const AccountProperty& property, const RawPropertyValue& value) {
				return property.canAllow({ model::PropertyModificationType::Add, value });
			}

			static bool CanRemove(const AccountProperty& property, const RawPropertyValue& value) {
				return property.canAllow({ model::PropertyModificationType::Del, value });
			}

			static void Add(AccountProperty& property, const RawPropertyValue& value) {
				property.allow({ model::PropertyModificationType::Add, value });
			}

			static void Remove(AccountProperty& property, const RawPropertyValue& value) {
				property.allow({ model::PropertyModificationType::Del, value });
			}
		};

		struct BlockTraits {
			static AccountProperty Create(const CustomPropertyModifications& modifications) {
				return CreateWithOperationType(DefaultOperationType(), modifications);
			}

			static AccountProperty CreateWithOppositeOperationType(const CustomPropertyModifications& modifications) {
				return CreateWithOperationType(OppositeOperationType(), modifications);
			}

			static OperationType DefaultOperationType() {
				return OperationType::Block;
			}

			static OperationType OppositeOperationType() {
				return OperationType::Allow;
			}

			static bool CanAdd(const AccountProperty& property, const RawPropertyValue& value) {
				return property.canBlock({ model::PropertyModificationType::Add, value });
			}

			static bool CanRemove(const AccountProperty& property, const RawPropertyValue& value) {
				return property.canBlock({ model::PropertyModificationType::Del, value });
			}

			static void Add(AccountProperty& property, const RawPropertyValue& value) {
				property.block({ model::PropertyModificationType::Add, value });
			}

			static void Remove(AccountProperty& property, const RawPropertyValue& value) {
				property.block({ model::PropertyModificationType::Del, value });
			}
		};

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Allow) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AllowTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Block) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

		// endregion

		void AssertEqualValues(const RawPropertyValues& expectedValues, const std::set<RawPropertyValue>& values) {
			ASSERT_EQ(expectedValues.size(), values.size());

			auto valuesCopy(values);
			auto i = 0u;
			for (const auto& expectedValue : expectedValues) {
				auto iter = valuesCopy.find(expectedValue);
				ASSERT_NE(valuesCopy.cend(), iter) << "at index " << i;

				EXPECT_EQ(expectedValue, *iter) << "at index " << i;
				valuesCopy.erase(iter);
				++i;
			}

			EXPECT_TRUE(valuesCopy.empty());
		}
	}

	TEST(TEST_CLASS, CanCreateAccountProperty) {
		// Act:
		AccountProperty property(Custom_Property_Type, Custom_Property_Size);

		// Assert:
		EXPECT_EQ(Custom_Property_Type, property.descriptor().propertyType());
		EXPECT_EQ(OperationType::Block, property.descriptor().operationType());
		EXPECT_EQ(model::PropertyType(Custom_Property_Type | model::PropertyType::Block), property.descriptor().raw());
		EXPECT_TRUE(property.values().empty());
	}

	TEST(TEST_CLASS, PropertyValueSizeReturnsExpectedSize) {
		// Arrange:
		AccountProperty property(Custom_Property_Type, Custom_Property_Size);

		// Act + Assert:
		EXPECT_EQ(Custom_Property_Size, property.propertyValueSize());
	}

	TRAITS_BASED_TEST(ContainsReturnsTrueWhenValueIsKnown) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);
		auto values = ExtractRawPropertyValues(context.Modifications);

		// Assert:
		for (const auto& value : values)
			EXPECT_TRUE(property.contains(value));
	}

	TRAITS_BASED_TEST(ContainsReturnsFalseWhenValueIsUnknown) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);

		// Assert:
		for (auto i = 0u; i < 10; ++i)
			EXPECT_FALSE(property.contains(test::GenerateRandomVector(Custom_Property_Size)));
	}

	TRAITS_BASED_TEST(CanAddValueWhenOperationIsPermissible) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_TRUE(TTraits::CanAdd(property, test::GenerateRandomVector(Custom_Property_Size)));
	}

	TRAITS_BASED_TEST(CannotAddValueWhenValueSizeIsInvalid) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_THROW(TTraits::CanAdd(property, test::GenerateRandomVector(Custom_Property_Size - 1)), catapult_invalid_argument);
		EXPECT_THROW(TTraits::CanAdd(property, test::GenerateRandomVector(Custom_Property_Size + 1)), catapult_invalid_argument);
	}

	TRAITS_BASED_TEST(CannotAddValueWhenValueIsKnown) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_FALSE(TTraits::CanAdd(property, ToVector(context.Values[1])));
	}

	TRAITS_BASED_TEST(CanAddValueWhenOperationTypeConflictsPropertyTypeButValuesAreEmpty) {
		// Arrange:
		TestContext context({});
		auto property = TTraits::CreateWithOppositeOperationType(context.Modifications);

		// Act + Assert:
		EXPECT_TRUE(TTraits::CanAdd(property, test::GenerateRandomVector(Custom_Property_Size)));
	}

	TRAITS_BASED_TEST(CannotAddValueWhenOperationTypeConflictsPropertyTypeAndValuesAreNotEmpty) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::CreateWithOppositeOperationType(context.Modifications);

		// Act + Assert:
		EXPECT_FALSE(TTraits::CanAdd(property, test::GenerateRandomVector(Custom_Property_Size)));
	}

	TRAITS_BASED_TEST(CanRemoveValueWhenOperationIsPermissible) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_TRUE(TTraits::CanRemove(property, ToVector(context.Values[1])));
	}

	TRAITS_BASED_TEST(CannotRemoveValueWhenValueSizeIsInvalid) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_THROW(TTraits::CanRemove(property, test::GenerateRandomVector(Custom_Property_Size - 1)), catapult_invalid_argument);
		EXPECT_THROW(TTraits::CanRemove(property, test::GenerateRandomVector(Custom_Property_Size + 1)), catapult_invalid_argument);
	}

	TRAITS_BASED_TEST(CannotRemoveValueWhenValueIsUnknown) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_FALSE(TTraits::CanRemove(property, test::GenerateRandomVector(Custom_Property_Size)));
	}

	TRAITS_BASED_TEST(CannotRemoveValueWhenOperationTypeConflictsPropertyTypeAndValuesAreNotEmpty) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::CreateWithOppositeOperationType(context.Modifications);

		// Act + Assert:
		EXPECT_FALSE(TTraits::CanRemove(property, ToVector(context.Values[1])));
	}

	TRAITS_BASED_TEST(AddAddsValueToSet) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);
		auto value = test::GenerateRandomVector(Custom_Property_Size);

		// Act:
		TTraits::Add(property, value);

		// Assert:
		auto expectedValues = ExtractRawPropertyValues(context.Modifications);
		expectedValues.push_back(value);
		AssertEqualValues(expectedValues, property.values());
	}

	TRAITS_BASED_TEST(CanAddRemovedValueToSetAgain) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);
		auto value = ToVector(context.Values[1]);
		TTraits::Remove(property, value);

		// Sanity:
		EXPECT_EQ(2u, property.values().size());
		EXPECT_TRUE(TTraits::CanAdd(property, value));

		// Act:
		TTraits::Add(property, value);

		// Assert:
		auto expectedValues = ExtractRawPropertyValues(context.Modifications);
		AssertEqualValues(expectedValues, property.values());
	}

	TRAITS_BASED_TEST(CannotAddValueWhenValueSizeMismatches) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_THROW(TTraits::Add(property, test::GenerateRandomVector(Custom_Property_Size + 1)), catapult_invalid_argument);
		EXPECT_THROW(TTraits::Add(property, test::GenerateRandomVector(Custom_Property_Size - 1)), catapult_invalid_argument);
	}

	TRAITS_BASED_TEST(RemoveRemovesValueFromSet) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);
		auto value = ToVector(context.Values[1]);

		// Act:
		TTraits::Remove(property, value);

		// Assert:
		auto expectedValues = ExtractRawPropertyValues(context.Modifications);
		expectedValues.erase(expectedValues.cbegin() + 1);
		AssertEqualValues(expectedValues, property.values());
	}

	TRAITS_BASED_TEST(CannotRemoveValueWhenValueSizeMismatches) {
		// Arrange:
		TestContext context({ Add, Add, Add });
		auto property = TTraits::Create(context.Modifications);

		// Act + Assert:
		EXPECT_THROW(TTraits::Remove(property, test::GenerateRandomVector(Custom_Property_Size + 1)), catapult_invalid_argument);
		EXPECT_THROW(TTraits::Remove(property, test::GenerateRandomVector(Custom_Property_Size - 1)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, AddCanChangePropertyDescriptorToOperationTypeAllow) {
		// Arrange:
		auto property = BlockTraits::Create({});

		// Sanity:
		EXPECT_EQ(OperationType::Block, property.descriptor().operationType());

		// Act:
		AllowTraits::Add(property, test::GenerateRandomVector(Custom_Property_Size));

		// Assert: property has now operation type allow
		EXPECT_EQ(OperationType::Allow, property.descriptor().operationType());
		EXPECT_EQ(1u, property.values().size());
	}

	TEST(TEST_CLASS, RemoveCanChangePropertyDescriptorToOperationTypeBlock) {
		// Arrange:
		TestContext context({ Add });
		auto property = AllowTraits::Create(context.Modifications);
		auto value = ToVector(context.Values[0]);

		// Sanity:
		EXPECT_EQ(OperationType::Allow, property.descriptor().operationType());

		// Act:
		AllowTraits::Remove(property, value);

		// Assert: property has now operation type block
		EXPECT_EQ(OperationType::Block, property.descriptor().operationType());
		EXPECT_TRUE(property.values().empty());
	}
}}
