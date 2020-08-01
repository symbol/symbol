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

#include "catapult/ionet/PacketReader.h"
#include "catapult/model/VerifiableEntity.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS PacketReaderTests

	namespace {
		PacketReader CreatePacketReaderAroundPacket(const Packet& packet) {
			// Arrange:
			auto reader = PacketReader(packet);

			// Sanity:
			EXPECT_FALSE(reader.empty());
			EXPECT_FALSE(reader.hasError());
			return reader;
		}

		void AssertAllDataConsumed(const PacketReader& reader) {
			// Assert:
			EXPECT_TRUE(reader.empty());
			EXPECT_FALSE(reader.hasError());
		}

		void AssertHasError(const PacketReader& reader) {
			// Assert:
			EXPECT_TRUE(reader.empty());
			EXPECT_TRUE(reader.hasError());
		}
	}

	// region no data

	TEST(TEST_CLASS, CannotCreateReaderAroundPacketWithSizeTooSmall) {
		// Arrange:
		for (auto size : { static_cast<uint32_t>(0), SizeOf32<PacketHeader>() - 1 }) {
			auto pPacket = test::CreateRandomPacket(0, PacketType::Undefined);
			pPacket->Size = size;

			// Act:
			auto reader = PacketReader(*pPacket);

			// Assert:
			AssertHasError(reader);
		}
	}

	TEST(TEST_CLASS, CanCreateReaderAroundEmptyPacket) {
		// Arrange:
		auto pPacket = test::CreateRandomPacket(0, PacketType::Undefined);

		// Act:
		auto reader = PacketReader(*pPacket);

		// Assert:
		AssertAllDataConsumed(reader);
	}

	// endregion

	// region fixed size

	TEST(TEST_CLASS, CanReadSingleFixedSizeValueFromPacket) {
		// Arrange:
		auto pPacket = test::CreateRandomPacket(4, PacketType::Undefined);
		reinterpret_cast<uint32_t&>(*pPacket->Data()) = 0x98562712;

		auto reader = CreatePacketReaderAroundPacket(*pPacket);

		// Act:
		auto value = *reader.readFixed<uint32_t>();

		// Assert:
		EXPECT_EQ(0x98562712, value);
		AssertAllDataConsumed(reader);
	}

	TEST(TEST_CLASS, CanReadMultipleFixedSizeValuesFromPacket) {
		// Arrange:
		auto pPacket = test::CreateRandomPacket(12, PacketType::Undefined);
		reinterpret_cast<uint32_t&>(*pPacket->Data()) = 0x98562712;
		reinterpret_cast<uint32_t&>(*(pPacket->Data() + 4)) = 0xAB7528BC;
		reinterpret_cast<uint32_t&>(*(pPacket->Data() + 8)) = 0x00012352;

		auto reader = CreatePacketReaderAroundPacket(*pPacket);

		// Act:
		auto value1 = *reader.readFixed<uint32_t>();
		auto value2 = *reader.readFixed<uint32_t>();
		auto value3 = *reader.readFixed<uint32_t>();

		// Assert:
		EXPECT_EQ(0x98562712u, value1);
		EXPECT_EQ(0xAB7528BCu, value2);
		EXPECT_EQ(0x00012352u, value3);
		AssertAllDataConsumed(reader);
	}

	TEST(TEST_CLASS, CannotReadFixedSizeValueFromPacketWithInsufficientData) {
		// Arrange:
		auto pPacket = test::CreateRandomPacket(3, PacketType::Undefined);

		auto reader = CreatePacketReaderAroundPacket(*pPacket);

		// Act:
		const auto* pValue = reader.readFixed<uint32_t>();

		// Assert:
		EXPECT_FALSE(!!pValue);
		AssertHasError(reader);
	}

	// endregion

	// region variable size

	TEST(TEST_CLASS, CanReadSingleVariableSizeValueFromPacket) {
		// Arrange:
		auto pPacket = test::CreateRandomPacket(76, PacketType::Undefined);
		reinterpret_cast<uint32_t&>(*pPacket->Data()) = 76;

		auto reader = CreatePacketReaderAroundPacket(*pPacket);

		// Act:
		const auto& entity = *reader.readVariable<model::VerifiableEntity>();

		// Assert:
		ASSERT_EQ(76u, entity.Size);
		EXPECT_EQ(reinterpret_cast<const model::VerifiableEntity&>(*pPacket->Data()), entity);
		AssertAllDataConsumed(reader);
	}

	TEST(TEST_CLASS, CanReadMultipleVariableSizeValuesFromPacket) {
		// Arrange: align entities on 8-byte boundaries
		auto pPacket = test::CreateRandomPacket(16 + 56 + 26, PacketType::Undefined);
		reinterpret_cast<uint32_t&>(*pPacket->Data()) = 16;
		reinterpret_cast<uint32_t&>(*(pPacket->Data() + 16)) = 56;
		reinterpret_cast<uint32_t&>(*(pPacket->Data() + 16 + 56)) = 26;

		auto reader = CreatePacketReaderAroundPacket(*pPacket);

		// Act:
		const auto& entity1 = *reader.readVariable<model::VerifiableEntity>();
		const auto& entity2 = *reader.readVariable<model::VerifiableEntity>();
		const auto& entity3 = *reader.readVariable<model::VerifiableEntity>();

		// Assert:
		EXPECT_EQ(16u, entity1.Size);
		EXPECT_EQ(reinterpret_cast<const model::VerifiableEntity&>(*pPacket->Data()), entity1);
		EXPECT_EQ(56u, entity2.Size);
		EXPECT_EQ(reinterpret_cast<const model::VerifiableEntity&>(*(pPacket->Data() + 16)), entity2);
		EXPECT_EQ(26u, entity3.Size);
		EXPECT_EQ(reinterpret_cast<const model::VerifiableEntity&>(*(pPacket->Data() + 16 + 56)), entity3);
		AssertAllDataConsumed(reader);
	}

	TEST(TEST_CLASS, CannotReadVariableSizeValueFromPacketWithInsufficientDataForSize) {
		// Arrange:
		auto pPacket = test::CreateRandomPacket(3, PacketType::Undefined);

		auto reader = CreatePacketReaderAroundPacket(*pPacket);

		// Act:
		const auto* pEntity = reader.readVariable<model::VerifiableEntity>();

		// Assert:
		EXPECT_FALSE(!!pEntity);
		AssertHasError(reader);
	}

	TEST(TEST_CLASS, CannotReadVariableSizeValueFromPacketWithInsufficientDataForPayload) {
		// Arrange:
		auto pPacket = test::CreateRandomPacket(80, PacketType::Undefined);
		reinterpret_cast<uint32_t&>(*pPacket->Data()) = 80 + 1;

		auto reader = CreatePacketReaderAroundPacket(*pPacket);

		// Act:
		const auto* pEntity = reader.readVariable<model::VerifiableEntity>();

		// Assert:
		EXPECT_FALSE(!!pEntity);
		AssertHasError(reader);
	}

	// endregion

	// region mixed

	TEST(TEST_CLASS, CanReadMultipleHeterogeneousValuesFromPacket) {
		// Arrange: align entities on 8-byte boundaries
		auto pPacket = test::CreateRandomPacket(4 + 4 + 14 + 2 + 25, PacketType::Undefined);
		reinterpret_cast<uint32_t&>(*pPacket->Data()) = 0x00012234;
		reinterpret_cast<uint32_t&>(*(pPacket->Data() + 8)) = 14;
		reinterpret_cast<uint16_t&>(*(pPacket->Data() + 8 + 14)) = 0x77FF;
		reinterpret_cast<uint32_t&>(*(pPacket->Data() + 8 + 14 + 2)) = 25;

		auto reader = CreatePacketReaderAroundPacket(*pPacket);

		// Act:
		auto value1 = *reader.readFixed<uint32_t>();
		reader.readFixed<uint32_t>(); // skip padding
		const auto& entity1 = *reader.readVariable<model::VerifiableEntity>();
		auto value2 = *reader.readFixed<uint16_t>();
		const auto& entity2 = *reader.readVariable<model::VerifiableEntity>();

		// Assert:
		EXPECT_EQ(0x00012234u, value1);
		EXPECT_EQ(14u, entity1.Size);
		EXPECT_EQ(reinterpret_cast<const model::VerifiableEntity&>(*(pPacket->Data() + 8)), entity1);
		EXPECT_EQ(0x77FF, value2);
		EXPECT_EQ(25u, entity2.Size);
		EXPECT_EQ(reinterpret_cast<const model::VerifiableEntity&>(*(pPacket->Data() + 8 + 14 + 2)), entity2);
		AssertAllDataConsumed(reader);
	}

	// endregion
}}
