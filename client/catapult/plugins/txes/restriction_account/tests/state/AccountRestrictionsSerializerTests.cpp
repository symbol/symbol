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

#include "src/state/AccountRestrictionsSerializer.h"
#include "src/state/AccountRestrictionUtils.h"
#include "catapult/model/EntityType.h"
#include "tests/test/AccountRestrictionTestUtils.h"
#include "tests/test/core/SerializerOrderingTests.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountRestrictionsSerializerTests

	namespace {
		void AssertEqualData(const std::vector<uint8_t>& expectedBuffer, const uint8_t* pData, const std::string& message) {
			EXPECT_EQ_MEMORY(expectedBuffer.data(), pData, expectedBuffer.size()) << message;
		}

		// region v1 helpers

		auto CalculateExpectedSizeV1(const AccountRestrictions& restrictions) {
			auto size = Address::Size + sizeof(uint64_t);
			for (const auto& pair : restrictions)
				size += sizeof(model::AccountRestrictionFlags) + sizeof(uint64_t) + pair.second.values().size() * pair.second.valueSize();

			return size;
		}

		void AssertBufferV1(const AccountRestrictions& restrictions, const std::vector<uint8_t>& buffer, size_t expectedSize) {
			ASSERT_EQ(expectedSize, buffer.size());

			test::BufferReader reader(buffer);
			EXPECT_EQ(restrictions.address(), reader.read<Address>());
			EXPECT_EQ(restrictions.size(), reader.read<uint64_t>());

			auto i = 0u;
			for (const auto& pair : restrictions) {
				const auto& restriction = pair.second;

				auto message = "restrictions, restriction at index " + std::to_string(i);
				EXPECT_EQ(restriction.descriptor().raw(), reader.read<model::AccountRestrictionFlags>()) << message;

				const auto& values = restriction.values();
				EXPECT_EQ(values.size(), reader.read<uint64_t>()) << message;

				auto j = 0u;
				for (const auto& value : values) {
					auto message2 = message + ", value at index " + std::to_string(j);
					AssertEqualData(value, reader.data(), message2);
					reader.advance(restriction.valueSize());
					++j;
				}

				++i;
			}
		}

		void AssertCanSaveAccountRestrictionsV1(const std::vector<size_t>& valuesSizes) {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream(buffer);
			auto restrictions = test::CreateAccountRestrictions(AccountRestrictionOperationType::Allow, valuesSizes);
			restrictions.setVersion(1);

			// Act:
			AccountRestrictionsSerializer::Save(restrictions, outputStream);

			// Assert:
			AssertBufferV1(restrictions, buffer, CalculateExpectedSizeV1(restrictions));
		}

		// endregion

		// region v2 helpers

		auto CountRestrictions(const AccountRestrictions& restrictions) {
			return static_cast<size_t>(std::count_if(restrictions.begin(), restrictions.end(), [](const auto& pair) {
				return !pair.second.values().empty();
			}));
		}

		auto CalculateExpectedSize(const AccountRestrictions& restrictions) {
			auto size = Address::Size + sizeof(uint64_t);
			for (const auto& pair : restrictions) {
				if (pair.second.values().empty())
					continue;

				size += sizeof(model::AccountRestrictionFlags) + sizeof(uint64_t) + pair.second.values().size() * pair.second.valueSize();
			}

			return size;
		}

		void AssertBuffer(const AccountRestrictions& restrictions, const std::vector<uint8_t>& buffer, size_t expectedSize) {
			ASSERT_EQ(expectedSize, buffer.size());

			test::BufferReader reader(buffer);
			EXPECT_EQ(restrictions.address(), reader.read<Address>());
			EXPECT_EQ(CountRestrictions(restrictions), reader.read<uint64_t>());

			auto i = 0u;
			for (const auto& pair : restrictions) {
				const auto& restriction = pair.second;
				if (restriction.values().empty())
					continue;

				auto message = "restrictions, restriction at index " + std::to_string(i);
				EXPECT_EQ(restriction.descriptor().raw(), reader.read<model::AccountRestrictionFlags>()) << message;

				const auto& values = restriction.values();
				EXPECT_EQ(values.size(), reader.read<uint64_t>()) << message;

				auto j = 0u;
				for (const auto& value : values) {
					auto message2 = message + ", value at index " + std::to_string(j);
					AssertEqualData(value, reader.data(), message2);
					reader.advance(restriction.valueSize());
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
			restrictions.setVersion(2);

			// Act:
			AccountRestrictionsSerializer::Save(restrictions, outputStream);

			// Assert:
			AssertBuffer(restrictions, buffer, CalculateExpectedSize(restrictions));
		}

		// endregion
	}

	// region Save

	TEST(TEST_CLASS, CanSaveAccountRestrictions_AllRestrictionsEmpty_V1) {
		AssertCanSaveAccountRestrictionsV1({ 0, 0, 0, 0 });
	}

	TEST(TEST_CLASS, CanSaveAccountRestrictions_SingleRestrictionNotEmpty_V1) {
		AssertCanSaveAccountRestrictionsV1({ 1, 0, 0, 0 });
		AssertCanSaveAccountRestrictionsV1({ 0, 1, 0, 0 });
		AssertCanSaveAccountRestrictionsV1({ 0, 0, 1, 0 });
		AssertCanSaveAccountRestrictionsV1({ 0, 0, 0, 1 });
	}

	TEST(TEST_CLASS, CanSaveAccountRestrictions_NoRestrictionsEmpty_V1) {
		AssertCanSaveAccountRestrictionsV1({ 5, 3, 6, 7 });
		AssertCanSaveAccountRestrictionsV1({ 123, 97, 24, 31 });
	}

	TEST(TEST_CLASS, CanSaveAccountRestrictions_AllRestrictionsEmpty) {
		AssertCanSaveAccountRestrictions({ 0, 0, 0, 0 });
	}

	TEST(TEST_CLASS, CanSaveAccountRestrictions_SingleRestrictionNotEmpty) {
		AssertCanSaveAccountRestrictions({ 1, 0, 0, 0 });
		AssertCanSaveAccountRestrictions({ 0, 1, 0, 0 });
		AssertCanSaveAccountRestrictions({ 0, 0, 1, 0 });
		AssertCanSaveAccountRestrictions({ 0, 0, 0, 1 });
	}

	TEST(TEST_CLASS, CanSaveAccountRestrictions_NoRestrictionsEmpty) {
		AssertCanSaveAccountRestrictions({ 5, 3, 6, 7 });
		AssertCanSaveAccountRestrictions({ 123, 97, 24, 31 });
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
				auto& restriction = restrictions.restriction(model::AccountRestrictionFlags::Address);
				for (const auto& address : addresses)
					restriction.allow({ model::AccountRestrictionModificationAction::Add, ToVector(address) });
			}

			static constexpr size_t GetKeyStartBufferOffset() {
				return Address::Size + sizeof(uint64_t) + sizeof(model::AccountRestrictionFlags);
			}
		};
	}

	TEST(TEST_CLASS, SavedValuesAreOrdered) {
		test::SerializerOrderingTests<AccountRestrictionsSerializerOrderingTraits>::AssertSaveOrdersEntriesByKey();
	}

	namespace {
		size_t GetValueSize(model::AccountRestrictionFlags restrictionFlags) {
			auto strippedRestrictionFlags = AccountRestrictionDescriptor(restrictionFlags).restrictionFlags();
			switch (strippedRestrictionFlags) {
			case model::AccountRestrictionFlags::Address:
				return Address::Size;
			case model::AccountRestrictionFlags::MosaicId:
				return sizeof(MosaicId);
			case model::AccountRestrictionFlags::TransactionType:
				return sizeof(model::EntityType);
			default:
				CATAPULT_THROW_INVALID_ARGUMENT_1("invalid restriction flags", utils::to_underlying_type(restrictionFlags));
			}
		}

		void AssertAccountRestrictionFlags(
				const std::vector<model::AccountRestrictionFlags>& expectedAccountRestrictionFlagsContainer,
				const RawBuffer& buffer,
				size_t offset) {
			ASSERT_LE(offset + sizeof(uint64_t) + expectedAccountRestrictionFlagsContainer.size() * sizeof(uint8_t), buffer.Size);

			test::BufferReader reader(buffer);
			reader.advance(offset);
			ASSERT_EQ(expectedAccountRestrictionFlagsContainer.size(), reader.read<uint64_t>());

			for (auto i = 0u; i < expectedAccountRestrictionFlagsContainer.size(); ++i) {
				auto restrictionFlags = reader.read<model::AccountRestrictionFlags>();
				auto restrictionDescriptor = AccountRestrictionDescriptor(restrictionFlags);
				EXPECT_EQ(expectedAccountRestrictionFlagsContainer[i], restrictionDescriptor.raw()) << "at index " << i;

				auto numValues = reader.read<uint64_t>();
				reader.advance(numValues * GetValueSize(restrictionDescriptor.directionalRestrictionFlags()));
			}
		}

		void AssertOrderedRestrictions() {
			// Arrange:
			constexpr auto Add = model::AccountRestrictionModificationAction::Add;
			constexpr auto Block = model::AccountRestrictionFlags::Block;
			constexpr auto Outgoing = model::AccountRestrictionFlags::Outgoing;

			// - alternate Block and Allow (notice that ordering is independent of block flag)
			AccountRestrictions restrictions(test::GenerateRandomByteArray<Address>());
			std::vector<model::AccountRestrictionFlags> allFlags{
				model::AccountRestrictionFlags::Address | Block, //                   0x8001 & ~Block == 0x0001
				model::AccountRestrictionFlags::MosaicId, //                          0x0002 & ~Block == 0x0002
				model::AccountRestrictionFlags::Address | Outgoing, //                0x4001 & ~Block == 0x4001
				model::AccountRestrictionFlags::TransactionType | Outgoing | Block // 0xC004 & ~Block == 0x4004
			};

			restrictions.restriction(allFlags[0]).block({ Add, test::GenerateRandomVector(Address::Size) });
			restrictions.restriction(allFlags[1]).allow({ Add, test::GenerateRandomVector(sizeof(MosaicId)) });
			restrictions.restriction(allFlags[2]).allow({ Add, test::GenerateRandomVector(Address::Size) });
			restrictions.restriction(allFlags[3]).block({ Add, test::GenerateRandomVector(sizeof(model::EntityType)) });

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			// Act:
			AccountRestrictionsSerializer::Save(restrictions, stream);

			// Assert:
			AssertAccountRestrictionFlags(allFlags, buffer, Address::Size);
		}
	}

	TEST(TEST_CLASS, SavedRestrictionsAreOrdered) {
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
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Allow, { 0, 0, 0, 0 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 0, 0, 0, 0 });
	}

	TEST(TEST_CLASS, CanRoundtripAccountRestrictions_SingleRestrictionNotEmpty) {
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Allow, { 1, 0, 0, 0 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 0, 1, 0, 0 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 0, 0, 1, 0 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 0, 0, 0, 1 });

		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Allow, { 5, 0, 0, 0 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 0, 5, 0, 0 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 0, 0, 5, 0 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 0, 0, 0, 5 });
	}

	TEST(TEST_CLASS, CanRoundtripAccountRestrictions_NoRestrictionsEmpty) {
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Allow, { 1, 1, 1, 1 });
		AssertCanRoundtripAccountRestrictions(AccountRestrictionOperationType::Block, { 5, 7, 4, 3 });
	}

	// endregion
}}
