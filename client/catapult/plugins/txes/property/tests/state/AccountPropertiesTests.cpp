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

#include "src/state/AccountProperties.h"
#include "catapult/model/EntityType.h"
#include "tests/test/AccountPropertiesTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountPropertiesTests

	namespace {
		constexpr auto Add = model::PropertyModificationType::Add;

		struct AddressPropertyTraits {
			using ValueType = Address;

			static constexpr model::PropertyType PropertyType() {
				return model::PropertyType::Address;
			}
		};

		struct MosaicPropertyTraits {
			using ValueType = MosaicId;

			static constexpr model::PropertyType PropertyType() {
				return model::PropertyType::MosaicId;
			}
		};

		struct TransactionTypePropertyTraits {
			using ValueType = model::EntityType;

			static constexpr model::PropertyType PropertyType() {
				return model::PropertyType::TransactionType;
			}
		};

		std::vector<model::PropertyType> CollectPropertyTypes(const AccountProperties& properties) {
			std::vector<model::PropertyType> types;
			for (const auto& property : properties)
				types.push_back(property.first);

			return types;
		}

		void AssertIsEmpty(const std::vector<size_t>& valueSizes, bool expectedResult) {
			// Arrange:
			auto properties = test::CreateAccountProperties(state::OperationType::Allow, valueSizes);

			// Act + Assert:
			EXPECT_EQ(expectedResult, properties.isEmpty());
		}
	}

#define PROPERTY_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressPropertyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicPropertyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_TransactionType) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTypePropertyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TEST(TEST_CLASS, CanCreateAccountProperties) {
		// Arrange:
		auto address = test::GenerateRandomData<Address_Decoded_Size>();

		// Act:
		AccountProperties properties(address);

		// Assert:
		EXPECT_EQ(address, properties.address());
		EXPECT_EQ(3u, properties.size());
	}

	TEST(TEST_CLASS, CanCreateAccountPropertiesWithDifferentPropertyOperationTypes) {
		// Arrange:
		AccountProperties properties(test::GenerateRandomData<Address_Decoded_Size>());
		auto& addressProperty = properties.property(model::PropertyType::Address);
		auto& mosaicProperty = properties.property(model::PropertyType::MosaicId);

		// Act:
		addressProperty.block({ Add, test::GenerateRandomVector(Address_Decoded_Size) });
		mosaicProperty.allow({ Add, test::GenerateRandomVector(sizeof(MosaicId)) });

		// Assert:
		EXPECT_EQ(1u, addressProperty.values().size());
		EXPECT_EQ(state::OperationType::Block, addressProperty.descriptor().operationType());

		EXPECT_EQ(1u, mosaicProperty.values().size());
		EXPECT_EQ(state::OperationType::Allow, mosaicProperty.descriptor().operationType());
	}

	TEST(TEST_CLASS, IsEmptyReturnsTrueWhenNoPropertyHasValues) {
		// Assert:
		AssertIsEmpty({ 0, 0, 0 }, true);
	}

	TEST(TEST_CLASS, IsEmptyReturnsFalseWhenAtLeastOnePropertyHasAtLeastOneValue) {
		// Assert:
		AssertIsEmpty({ 1, 0, 0 }, false);
		AssertIsEmpty({ 0, 1, 0 }, false);
		AssertIsEmpty({ 0, 0, 1 }, false);
		AssertIsEmpty({ 5, 0, 0 }, false);
		AssertIsEmpty({ 0, 5, 0 }, false);
		AssertIsEmpty({ 0, 0, 5 }, false);
		AssertIsEmpty({ 20, 17, 13 }, false);
	}

	TEST(TEST_CLASS, CanIterateThroughAccountProperties) {
		// Arrange:
		AccountProperties properties(test::GenerateRandomData<Address_Decoded_Size>());

		// Act:
		auto types = CollectPropertyTypes(properties);

		// Assert:
		std::vector<model::PropertyType> expectedTypes{
			model::PropertyType::Address,
			model::PropertyType::MosaicId,
			model::PropertyType::TransactionType
		};
		EXPECT_EQ(expectedTypes, types);
	}

	PROPERTY_TRAITS_BASED_TEST(TypedPropertyReturnsCorrectTypedPropertyIfPropertyTypeIsKnown) {
		// Arrange:
		AccountProperties properties(test::GenerateRandomData<Address_Decoded_Size>());

		// Act:
		auto property = properties.property<typename TTraits::ValueType>(TTraits::PropertyType());

		// Assert:
		EXPECT_EQ(TTraits::PropertyType() | model::PropertyType::Block, property.descriptor().raw());
	}

	TEST(TEST_CLASS, TypedPropertyThrowsIfPropertyTypeIsUnknown) {
		// Arrange:
		AccountProperties properties(test::GenerateRandomData<Address_Decoded_Size>());

		// Act + Assert:
		EXPECT_THROW(properties.property<Address>(model::PropertyType::Sentinel), catapult_invalid_argument);
	}

	PROPERTY_TRAITS_BASED_TEST(PropertyReturnsCorrectPropertyIfPropertyTypeIsKnown) {
		// Arrange:
		AccountProperties properties(test::GenerateRandomData<Address_Decoded_Size>());

		// Act:
		auto& property = properties.property(TTraits::PropertyType());

		// Assert:
		EXPECT_EQ(TTraits::PropertyType(), property.descriptor().propertyType());
	}

	TEST(TEST_CLASS, PropertyThrowsIfPropertyTypeIsUnknown) {
		// Arrange:
		AccountProperties properties(test::GenerateRandomData<Address_Decoded_Size>());

		// Act + Assert:
		EXPECT_THROW(properties.property(model::PropertyType::Sentinel), catapult_invalid_argument);
	}

	PROPERTY_TRAITS_BASED_TEST(PropertyReturnsCorrectPropertyIfPropertyTypeIsKnown_Const) {
		// Arrange:
		AccountProperties properties(test::GenerateRandomData<Address_Decoded_Size>());

		// Act:
		auto& property = const_cast<const AccountProperties&>(properties).property(TTraits::PropertyType());

		// Assert:
		EXPECT_EQ(TTraits::PropertyType(), property.descriptor().propertyType());
	}

	TEST(TEST_CLASS, PropertyThrowsIfPropertyTypeIsUnknown_Const) {
		// Arrange:
		AccountProperties properties(test::GenerateRandomData<Address_Decoded_Size>());

		// Act + Assert:
		EXPECT_THROW(const_cast<const AccountProperties&>(properties).property(model::PropertyType::Sentinel), catapult_invalid_argument);
	}
}}
