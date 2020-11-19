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

#include "catapult/ionet/PacketPayload.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS PacketPayloadTests

	namespace {
		constexpr auto Test_Packet_Type = static_cast<PacketType>(987);

		auto CreatePacketPointer(uint32_t payloadSize) {
			return test::CreateRandomPacket(payloadSize, Test_Packet_Type);
		}

		template<typename TDataContainer>
		auto CreatePacketPointerWithData(const TDataContainer& buffer) {
			auto bufferSize = static_cast<uint32_t>(buffer.size());
			auto pPacket = CreatePacketPointer(bufferSize);
			std::memcpy(pPacket->Data(), buffer.data(), bufferSize);
			return pPacket;
		}
	}

	// region basic

	TEST(TEST_CLASS, CanCreateUnsetPacketPayload) {
		// Act:
		PacketPayload payload;

		// Assert:
		EXPECT_TRUE(payload.unset());
		EXPECT_EQ(0u, payload.header().Size);
		EXPECT_EQ(PacketType::Undefined, payload.header().Type);
		EXPECT_TRUE(payload.buffers().empty());
	}

	TEST(TEST_CLASS, CanCreatePacketPayloadWithNoDataBuffers) {
		// Act:
		auto payload = PacketPayload(PacketType::Push_Block);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Push_Block);
		EXPECT_TRUE(payload.buffers().empty());
	}

	// endregion

	// region from packet

	TEST(TEST_CLASS, CannotCreatePacketPayloadFromPacketWithInvalidData) {
		// Arrange:
		auto pPacket = CreatePacketPointer(0);
		--pPacket->Size;

		// Act + Assert:
		EXPECT_THROW(PacketPayload(std::move(pPacket)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanCreatePacketPayloadFromPacketWithNoData) {
		// Arrange:
		auto pPacket = CreatePacketPointer(0);

		// Act:
		auto payload = PacketPayload(std::move(pPacket));
		pPacket.reset();

		// Sanity:
		EXPECT_FALSE(!!pPacket);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), Test_Packet_Type);
		EXPECT_TRUE(payload.buffers().empty());
	}

	TEST(TEST_CLASS, CanCreatePacketPayloadFromPacketWithData) {
		// Arrange:
		constexpr auto Data_Size = 123u;
		auto dataBuffer = test::GenerateRandomArray<Data_Size>();
		auto pPacket = CreatePacketPointerWithData(dataBuffer);

		// Act:
		auto payload = PacketPayload(std::move(pPacket));
		pPacket.reset();

		// Sanity:
		EXPECT_FALSE(!!pPacket);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + Data_Size, Test_Packet_Type);
		ASSERT_EQ(1u, payload.buffers().size());

		const auto& payloadBuffer = payload.buffers()[0];
		ASSERT_EQ(Data_Size, payloadBuffer.Size);
		EXPECT_EQ_MEMORY(dataBuffer.data(), payloadBuffer.pData, Data_Size);
	}

	// endregion

	// region Merge

	TEST(TEST_CLASS, CannotMergePacketWithInvalidDataAndPayload) {
		// Arrange:
		auto pPacket = CreatePacketPointer(0);
		--pPacket->Size;

		// Act + Assert:
		EXPECT_THROW(PacketPayload::Merge(std::move(pPacket), PacketPayload()), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanMergePacketAndUnsetPayload) {
		// Arrange:
		constexpr auto Data_Size = 222u;
		auto dataBuffer = test::GenerateRandomArray<Data_Size>();
		auto pPacket = CreatePacketPointerWithData(dataBuffer);

		// Act:
		auto payload = PacketPayload::Merge(std::move(pPacket), PacketPayload());

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + Data_Size, Test_Packet_Type);
		ASSERT_EQ(1u, payload.buffers().size());

		// - data from packet
		const auto& payloadBuffer = payload.buffers()[0];
		ASSERT_EQ(Data_Size, payloadBuffer.Size);
		EXPECT_EQ_MEMORY(dataBuffer.data(), payloadBuffer.pData, Data_Size);
	}

	TEST(TEST_CLASS, CanMergePacketAndPayloadWithHeaderOnly) {
		// Arrange:
		constexpr auto Data_Size = 222u;
		auto dataBuffer = test::GenerateRandomArray<Data_Size>();
		auto pPacket1 = CreatePacketPointerWithData(dataBuffer);

		auto pPacket2 = CreateSharedPacket<Packet>(0);
		pPacket2->Type = static_cast<PacketType>(987);

		// Act:
		auto payload = PacketPayload::Merge(std::move(pPacket1), PacketPayload(std::move(pPacket2)));

		// Assert:
		test::AssertPacketHeader(payload, 2 * sizeof(PacketHeader) + Data_Size, Test_Packet_Type);
		ASSERT_EQ(2u, payload.buffers().size());

		// - data from packet 1
		const auto* pPayloadBuffer = &payload.buffers()[0];
		ASSERT_EQ(Data_Size, pPayloadBuffer->Size);
		EXPECT_EQ_MEMORY(dataBuffer.data(), pPayloadBuffer->pData, Data_Size);

		// - header from packet 2
		pPayloadBuffer = &payload.buffers()[1];
		const auto& childPacketHeader = reinterpret_cast<const PacketHeader&>(*pPayloadBuffer->pData);
		ASSERT_EQ(sizeof(PacketHeader), pPayloadBuffer->Size);
		EXPECT_EQ(sizeof(PacketHeader), childPacketHeader.Size);
		EXPECT_EQ(static_cast<PacketType>(987), childPacketHeader.Type);
	}

	TEST(TEST_CLASS, CanMergePacketAndPayloadWithHeaderAndSingleDataBuffer) {
		// Arrange:
		constexpr auto Data1_Size = 222u;
		auto dataBuffer1 = test::GenerateRandomArray<Data1_Size>();
		auto pPacket1 = CreatePacketPointerWithData(dataBuffer1);

		constexpr auto Data2_Size = 123u;
		auto dataBuffer2 = test::GenerateRandomArray<Data2_Size>();
		auto pPacket2 = CreatePacketPointerWithData(dataBuffer2);
		pPacket2->Type = static_cast<PacketType>(987);

		// Act:
		auto payload = PacketPayload::Merge(std::move(pPacket1), PacketPayload(std::move(pPacket2)));

		// Assert:
		test::AssertPacketHeader(payload, 2 * sizeof(PacketHeader) + Data1_Size + Data2_Size, Test_Packet_Type);
		ASSERT_EQ(3u, payload.buffers().size());

		// - data from packet 1
		const auto* pPayloadBuffer = &payload.buffers()[0];
		ASSERT_EQ(Data1_Size, pPayloadBuffer->Size);
		EXPECT_EQ_MEMORY(dataBuffer1.data(), pPayloadBuffer->pData, Data1_Size);

		// - header from packet 2
		pPayloadBuffer = &payload.buffers()[1];
		const auto& childPacketHeader = reinterpret_cast<const PacketHeader&>(*pPayloadBuffer->pData);
		ASSERT_EQ(sizeof(PacketHeader), pPayloadBuffer->Size);
		EXPECT_EQ(sizeof(PacketHeader) + Data2_Size, childPacketHeader.Size);
		EXPECT_EQ(static_cast<PacketType>(987), childPacketHeader.Type);

		// - data from packet 2
		pPayloadBuffer = &payload.buffers()[2];
		ASSERT_EQ(Data2_Size, pPayloadBuffer->Size);
		EXPECT_EQ_MEMORY(dataBuffer2.data(), pPayloadBuffer->pData, Data2_Size);
	}

	TEST(TEST_CLASS, CanMergePacketAndPayloadWithHeaderAndMultipleDataBuffers) {
		// Arrange:
		constexpr auto Data1_Size = 222u;
		auto dataBuffer1 = test::GenerateRandomArray<Data1_Size>();
		auto pPacket1 = CreatePacketPointerWithData(dataBuffer1);

		constexpr auto Data2_Size = 164u + 212 + 132;
		auto entities = std::vector<std::shared_ptr<model::VerifiableEntity>>{
			test::CreateRandomEntityWithSize<>(164),
			test::CreateRandomEntityWithSize<>(212),
			test::CreateRandomEntityWithSize<>(132)
		};

		// Act:
		auto payload = PacketPayload::Merge(
				std::move(pPacket1),
				PacketPayloadFactory::FromEntities(static_cast<PacketType>(987), entities));

		// Assert:
		test::AssertPacketHeader(payload, 2 * sizeof(PacketHeader) + Data1_Size + Data2_Size, Test_Packet_Type);
		ASSERT_EQ(5u, payload.buffers().size());

		// - data from packet 1
		const auto* pPayloadBuffer = &payload.buffers()[0];
		ASSERT_EQ(Data1_Size, pPayloadBuffer->Size);
		EXPECT_EQ_MEMORY(dataBuffer1.data(), pPayloadBuffer->pData, Data1_Size);

		// - header from packet 2
		pPayloadBuffer = &payload.buffers()[1];
		const auto& childPacketHeader = reinterpret_cast<const PacketHeader&>(*pPayloadBuffer->pData);
		ASSERT_EQ(sizeof(PacketHeader), pPayloadBuffer->Size);
		EXPECT_EQ(sizeof(PacketHeader) + Data2_Size, childPacketHeader.Size);
		EXPECT_EQ(static_cast<PacketType>(987), childPacketHeader.Type);

		// - data (entities) from packet 2
		for (auto i = 0u; i < entities.size(); ++i) {
			pPayloadBuffer = &payload.buffers()[2 + i];
			ASSERT_EQ(entities[i]->Size, pPayloadBuffer->Size) << i;
			EXPECT_EQ_MEMORY(entities[i].get(), pPayloadBuffer->pData, entities[i]->Size) << i;
		}
	}

	// endregion
}}
