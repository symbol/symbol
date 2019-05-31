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

#include "src/state/AccountPropertiesSerializer.h"
#include "catapult/model/EntityType.h"
#include "tests/test/AccountPropertiesTestUtils.h"
#include "tests/test/core/SerializerOrderingTests.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountPropertiesSerializerTests

	namespace {
		auto CalculateExpectedSize(const AccountProperties& accountProperties) {
			auto size = Address_Decoded_Size + sizeof(uint64_t);
			for (const auto& pair : accountProperties)
				size += sizeof(uint8_t) + sizeof(uint64_t) + pair.second.values().size() * pair.second.propertyValueSize();

			return size;
		}

		void AssertEqualData(const std::vector<uint8_t>& expectedBuffer, const uint8_t* pData, const std::string& message) {
			EXPECT_EQ_MEMORY(expectedBuffer.data(), pData, expectedBuffer.size()) << message;
		}

		void AssertBuffer(const AccountProperties& accountProperties, const std::vector<uint8_t>& buffer, size_t expectedSize) {
			ASSERT_EQ(expectedSize, buffer.size());

			const auto* pData = buffer.data();
			EXPECT_EQ(accountProperties.address(), reinterpret_cast<const Address&>(*pData));
			pData += Address_Decoded_Size;

			EXPECT_EQ(accountProperties.size(), reinterpret_cast<const uint64_t&>(*pData));
			pData += sizeof(uint64_t);

			auto i = 0u;
			for (const auto& pair : accountProperties) {
				auto message = "accountProperties, property at index " + std::to_string(i);
				const auto& accountProperty = pair.second;
				EXPECT_EQ(accountProperty.descriptor().raw(), static_cast<model::PropertyType>(*pData)) << message;
				++pData;

				const auto& values = accountProperty.values();
				EXPECT_EQ(values.size(), *reinterpret_cast<const uint64_t*>(pData)) << message;
				pData += sizeof(uint64_t);

				auto j = 0u;
				for (const auto& value : values) {
					auto message2 = message + ", value at index " + std::to_string(j);
					AssertEqualData(value, pData, message2);
					pData += accountProperty.propertyValueSize();
					++j;
				}

				++i;
			}
		}

		void AssertCanSaveAccountProperties(const std::vector<size_t>& valuesSizes) {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream(buffer);
			auto accountProperties = test::CreateAccountProperties(state::OperationType::Allow, valuesSizes);

			// Act:
			AccountPropertiesSerializer::Save(accountProperties, outputStream);

			// Assert:
			AssertBuffer(accountProperties, buffer, CalculateExpectedSize(accountProperties));
		}
	}

	// region Save

	TEST(TEST_CLASS, CanSaveSingleAccountProperties_AllPropertiesEmpty) {
		// Assert:
		AssertCanSaveAccountProperties({ 0, 0, 0 });
	}

	TEST(TEST_CLASS, CanSaveSingleAccountProperties_SinglePropertyNonEmpty) {
		// Assert:
		AssertCanSaveAccountProperties({ 1, 0, 0 });
		AssertCanSaveAccountProperties({ 0, 1, 0 });
		AssertCanSaveAccountProperties({ 0, 0, 1 });
	}

	TEST(TEST_CLASS, CanSaveSingleAccountProperties_AllPropertiesNonEmpty) {
		// Assert:
		AssertCanSaveAccountProperties({ 5, 3, 6 });
		AssertCanSaveAccountProperties({ 123, 97, 24 });
	}

	namespace {
		struct AccountPropertiesSerializerOrderingTraits {
		public:
			using KeyType = Address;
			using SerializerType = AccountPropertiesSerializer;

			static auto CreateEntry() {
				return AccountProperties(test::GenerateRandomByteArray<Address>());
			}

			static void AddKeys(AccountProperties& accountProperties, const std::vector<Address>& addresses) {
				auto& accountProperty = accountProperties.property(model::PropertyType::Address);
				for (const auto& address : addresses)
					accountProperty.allow({ model::PropertyModificationType::Add, state::ToVector(address) });
			}

			static constexpr size_t GetKeyStartBufferOffset() {
				return Address_Decoded_Size + sizeof(uint64_t) + sizeof(uint8_t);
			}
		};
	}

	TEST(TEST_CLASS, SavedValuesAreOrdered) {
		// Assert:
		test::SerializerOrderingTests<AccountPropertiesSerializerOrderingTraits>::AssertSaveOrdersEntriesByKey();
	}

	namespace {
		size_t GetValueSize(model::PropertyType propertyType) {
			switch (propertyType) {
			case model::PropertyType::Address:
				return Address_Decoded_Size;
			case model::PropertyType::MosaicId:
				return sizeof(MosaicId);
			case model::PropertyType::TransactionType:
				return sizeof(model::EntityType);
			default:
				CATAPULT_THROW_INVALID_ARGUMENT_1("invalid property type", static_cast<uint16_t>(propertyType));
			}
		}

		void AssertPropertyTypes(const std::vector<model::PropertyType>& expectedPropertyTypes, const RawBuffer& buffer, size_t offset) {
			ASSERT_LE(offset + sizeof(uint64_t) + expectedPropertyTypes.size() * sizeof(uint8_t), buffer.Size);

			const auto* pData = buffer.pData + offset;
			ASSERT_EQ(expectedPropertyTypes.size(), *reinterpret_cast<const uint64_t*>(pData));
			pData += sizeof(uint64_t);
			for (auto i = 0u; i < expectedPropertyTypes.size(); ++i) {
				auto propertyDescriptor = state::PropertyDescriptor(reinterpret_cast<const model::PropertyType&>(*pData));
				EXPECT_EQ(expectedPropertyTypes[i], propertyDescriptor.raw()) << "at index " << i;
				++pData;

				auto numValues = reinterpret_cast<const uint64_t&>(*pData);
				pData += sizeof(uint64_t) + numValues * GetValueSize(propertyDescriptor.propertyType());
			}
		}

		void AssertOrderedProperties() {
			// Arrange: make address property Block and mosaic property Allow
			// - empty properties always have Block flag set
			constexpr auto Add = model::PropertyModificationType::Add;
			AccountProperties accountProperties(test::GenerateRandomByteArray<Address>());
			model::RawPropertyModification modification{ Add, test::GenerateRandomVector(sizeof(MosaicId)) };
			accountProperties.property(model::PropertyType::MosaicId).allow(modification);

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			// Act:
			AccountPropertiesSerializer::Save(accountProperties, stream);

			// Assert:
			std::vector<model::PropertyType> orderedPropertyTypes{
				model::PropertyType::Address | model::PropertyType::Block,
				model::PropertyType::MosaicId,
				model::PropertyType::TransactionType | model::PropertyType::Block
			};
			AssertPropertyTypes(orderedPropertyTypes, buffer, Address_Decoded_Size);
		}
	}

	TEST(TEST_CLASS, SavedPropertiesAreOrdered) {
		// Assert:
		AssertOrderedProperties();
	}

	// endregion

	// region Load

	namespace {
		std::vector<uint8_t> CreateBuffer(const AccountProperties& accountProperties) {
			std::vector<uint8_t> buffer(CalculateExpectedSize(accountProperties));

			auto* pData = buffer.data();
			std::memcpy(pData, accountProperties.address().data(), Address_Decoded_Size);
			pData += Address_Decoded_Size;

			reinterpret_cast<uint64_t&>(*pData) = accountProperties.size();
			pData += sizeof(uint64_t);
			for (const auto& pair : accountProperties) {
				const auto& accountProperty = pair.second;
				*pData = utils::to_underlying_type(accountProperty.descriptor().raw());
				++pData;

				*reinterpret_cast<uint64_t*>(pData) = accountProperty.values().size();
				pData += sizeof(uint64_t);
				auto propertyValueSize = accountProperty.propertyValueSize();
				for (const auto& value : accountProperty.values()) {
					std::memcpy(pData, value.data(), propertyValueSize);
					pData += propertyValueSize;
				}
			}

			return buffer;
		}

		void AssertCanLoadSingleAccountProperties(state::OperationType operationType, const std::vector<size_t>& valuesSizes) {
			// Arrange: operation type of properties in the container are alternating between Allow and Block
			auto originalAccountProperties = test::CreateAccountProperties(operationType, valuesSizes);
			auto buffer = CreateBuffer(originalAccountProperties);

			// Act:
			AccountProperties result(test::GenerateRandomByteArray<Address>());
			test::RunLoadValueTest<AccountPropertiesSerializer>(buffer, result);

			// Assert:
			test::AssertEqual(originalAccountProperties, result);
		}
	}

	TEST(TEST_CLASS, CanLoadSingleAccountPropertiesWithNoValues) {
		AssertCanLoadSingleAccountProperties(state::OperationType::Allow, { 0, 0, 0 });
		AssertCanLoadSingleAccountProperties(state::OperationType::Block, { 0, 0, 0 });
	}

	TEST(TEST_CLASS, CanLoadSingleAccountPropertiesWithSingleNonEmptyProperty) {
		AssertCanLoadSingleAccountProperties(state::OperationType::Allow, { 1, 0, 0 });
		AssertCanLoadSingleAccountProperties(state::OperationType::Block, { 0, 1, 0 });
		AssertCanLoadSingleAccountProperties(state::OperationType::Block, { 0, 0, 1 });
		AssertCanLoadSingleAccountProperties(state::OperationType::Allow, { 5, 0, 0 });
		AssertCanLoadSingleAccountProperties(state::OperationType::Block, { 0, 5, 0 });
		AssertCanLoadSingleAccountProperties(state::OperationType::Block, { 0, 0, 5 });
	}

	TEST(TEST_CLASS, CanLoadSingleAccountPropertiesWithMultipleNonEmptyProperties) {
		AssertCanLoadSingleAccountProperties(state::OperationType::Allow, { 1, 1, 1 });
		AssertCanLoadSingleAccountProperties(state::OperationType::Block, { 5, 7, 4 });
	}

	// endregion
}}
