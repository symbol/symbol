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

#include "src/state/AccountRestrictionsSerializer.h"
#include "catapult/model/EntityType.h"
#include "tests/test/AccountRestrictionsTestUtils.h"
#include "tests/test/core/SerializerOrderingTests.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountRestrictionsSerializerTests

	namespace {
		auto CalculateExpectedSize(const AccountRestrictions& restrictions) {
			auto size = Address_Decoded_Size + sizeof(uint64_t);
			for (const auto& pair : restrictions)
				size += sizeof(uint8_t) + sizeof(uint64_t) + pair.second.values().size() * pair.second.valueSize();

			return size;
		}

		void AssertEqualData(const std::vector<uint8_t>& expectedBuffer, const uint8_t* pData, const std::string& message) {
			EXPECT_EQ_MEMORY(expectedBuffer.data(), pData, expectedBuffer.size()) << message;
		}

		void AssertBuffer(const AccountRestrictions& restrictions, const std::vector<uint8_t>& buffer, size_t expectedSize) {
			ASSERT_EQ(expectedSize, buffer.size());

			const auto* pData = buffer.data();
			EXPECT_EQ(restrictions.address(), reinterpret_cast<const Address&>(*pData));
			pData += Address_Decoded_Size;

			EXPECT_EQ(restrictions.size(), reinterpret_cast<const uint64_t&>(*pData));
			pData += sizeof(uint64_t);

			auto i = 0u;
			for (const auto& pair : restrictions) {
				auto message = "restrictions, restriction at index " + std::to_string(i);
				const auto& restriction = pair.second;
				EXPECT_EQ(restriction.descriptor().raw(), static_cast<model::AccountRestrictionType>(*pData)) << message;
				++pData;

				const auto& values = restriction.values();
				EXPECT_EQ(values.size(), *reinterpret_cast<const uint64_t*>(pData)) << message;
				pData += sizeof(uint64_t);

				auto j = 0u;
				for (const auto& value : values) {
					auto message2 = message + ", value at index " + std::to_string(j);
					AssertEqualData(value, pData, message2);
					pData += restriction.valueSize();
					++j;
				}

				++i;
			}
		}

		void AssertCanSaveAccountRestrictions(const std::vector<size_t>& valuesSizes) {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream(buffer);
			auto restrictions = test::CreateAccountRestrictions(AccountRestrictionOperationType::Allow, valuesSizes);

			// Act:
			AccountRestrictionsSerializer::Save(restrictions, outputStream);

			// Assert:
			AssertBuffer(restrictions, buffer, CalculateExpectedSize(restrictions));
		}
	}

	// region Save

	TEST(TEST_CLASS, CanSaveAccountRestrictions_AllRestrictionsEmpty) {
		// Assert:
		AssertCanSaveAccountRestrictions({ 0, 0, 0 });
	}

	TEST(TEST_CLASS, CanSaveAccountRestrictions_SingleRestrictionNotEmpty) {
		// Assert:
		AssertCanSaveAccountRestrictions({ 1, 0, 0 });
		AssertCanSaveAccountRestrictions({ 0, 1, 0 });
		AssertCanSaveAccountRestrictions({ 0, 0, 1 });
	}

	TEST(TEST_CLASS, CanSaveAccountRestrictions_NoRestrictionsEmpty) {
		// Assert:
		AssertCanSaveAccountRestrictions({ 5, 3, 6 });
		AssertCanSaveAccountRestrictions({ 123, 97, 24 });
	}

	namespace {
		struct AccountRestrictionsSerializerOrderingTraits {
		public:
			using KeyType = Address;
			using SerializerType = AccountRestrictionsSerializer;

			static auto CreateEntry() {
				return AccountRestrictions(test::GenerateRandomByteArray<Address>());
			}

			static void AddKeys(AccountRestrictions& restrictions, const std::vector<Address>& addresses) {
				auto& restriction = restrictions.restriction(model::AccountRestrictionType::Address);
				for (const auto& address : addresses)
					restriction.allow({ model::AccountRestrictionModificationType::Add, ToVector(address) });
			}

			static constexpr size_t GetKeyStartBufferOffset() {
				return Address_Decoded_Size + sizeof(uint64_t) + sizeof(uint8_t);
			}
		};
	}

	TEST(TEST_CLASS, SavedValuesAreOrdered) {
		// Assert:
		test::SerializerOrderingTests<AccountRestrictionsSerializerOrderingTraits>::AssertSaveOrdersEntriesByKey();
	}

	namespace {
		size_t GetValueSize(model::AccountRestrictionType restrictionType) {
			switch (restrictionType) {
			case model::AccountRestrictionType::Address:
				return Address_Decoded_Size;
			case model::AccountRestrictionType::MosaicId:
				return sizeof(MosaicId);
			case model::AccountRestrictionType::TransactionType:
				return sizeof(model::EntityType);
			default:
				CATAPULT_THROW_INVALID_ARGUMENT_1("invalid restriction type", static_cast<uint16_t>(restrictionType));
			}
		}

		void AssertAccountRestrictionTypes(
				const std::vector<model::AccountRestrictionType>& expectedAccountRestrictionTypes,
				const RawBuffer& buffer,
				size_t offset) {
			ASSERT_LE(offset + sizeof(uint64_t) + expectedAccountRestrictionTypes.size() * sizeof(uint8_t), buffer.Size);

			const auto* pData = buffer.pData + offset;
			ASSERT_EQ(expectedAccountRestrictionTypes.size(), *reinterpret_cast<const uint64_t*>(pData));
			pData += sizeof(uint64_t);
			for (auto i = 0u; i < expectedAccountRestrictionTypes.size(); ++i) {
				auto restrictionType = reinterpret_cast<const model::AccountRestrictionType&>(*pData);
				auto restrictionDescriptor = AccountRestrictionDescriptor(restrictionType);
				EXPECT_EQ(expectedAccountRestrictionTypes[i], restrictionDescriptor.raw()) << "at index " << i;
				++pData;

				auto numValues = reinterpret_cast<const uint64_t&>(*pData);
				pData += sizeof(uint64_t) + numValues * GetValueSize(restrictionDescriptor.restrictionType());
			}
		}

		void AssertOrderedRestrictions() {
			// Arrange: make address restriction Block and mosaic restriction Allow
			// - empty restrictions always have Block flag set
			constexpr auto Add = model::AccountRestrictionModificationType::Add;
			AccountRestrictions restrictions(test::GenerateRandomByteArray<Address>());
			model::RawAccountRestrictionModification modification{ Add, test::GenerateRandomVector(sizeof(MosaicId)) };
			restrictions.restriction(model::AccountRestrictionType::MosaicId).allow(modification);

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			// Act:
			AccountRestrictionsSerializer::Save(restrictions, stream);

			// Assert:
			std::vector<model::AccountRestrictionType> orderedAccountRestrictionTypes{
				model::AccountRestrictionType::Address | model::AccountRestrictionType::Block,
				model::AccountRestrictionType::MosaicId,
				model::AccountRestrictionType::TransactionType | model::AccountRestrictionType::Block
			};
			AssertAccountRestrictionTypes(orderedAccountRestrictionTypes, buffer, Address_Decoded_Size);
		}
	}

	TEST(TEST_CLASS, SavedRestrictionsAreOrdered) {
		// Assert:
		AssertOrderedRestrictions();
	}

	// endregion

	// region Roundtrip

	namespace {
		void AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType operationType, const std::vector<size_t>& valuesSizes) {
			// Arrange: operation type of restrictions in the container are alternating between Allow and Block
			auto originalRestrictions = test::CreateAccountRestrictions(operationType, valuesSizes);

			// Act:
			auto result = test::RunRoundtripBufferTest<AccountRestrictionsSerializer>(originalRestrictions);

			// Assert:
			test::AssertEqual(originalRestrictions, result);
		}
	}

	TEST(TEST_CLASS, CanRoundtripAccountRestrictions_AllRestrictionsEmpty) {
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Allow, { 0, 0, 0 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 0, 0, 0 });
	}

	TEST(TEST_CLASS, CanRoundtripAccountRestrictions_SingleRestrictionNotEmpty) {
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Allow, { 1, 0, 0 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 0, 1, 0 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 0, 0, 1 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Allow, { 5, 0, 0 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 0, 5, 0 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 0, 0, 5 });
	}

	TEST(TEST_CLASS, CanRoundtripAccountRestrictions_NoRestrictionsEmpty) {
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Allow, { 1, 1, 1 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 5, 7, 4 });
	}

	// endregion
}}
