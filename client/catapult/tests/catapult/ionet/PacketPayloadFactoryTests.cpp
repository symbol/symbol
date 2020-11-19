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

#include "catapult/ionet/PacketPayloadFactory.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS PacketPayloadFactoryTests

	namespace {
		constexpr auto Test_Packet_Type = static_cast<PacketType>(987);
	}

	// region FromEntity / FromEntities

	namespace {
		template<typename TEntity>
		struct EntityTraits {
			using EntityPointerType = std::shared_ptr<TEntity>;
			using EntitiesContainer = std::vector<EntityPointerType>;

			static EntityPointerType MakeEntityWithSize(uint32_t size) {
				return test::CreateRandomEntityWithSize<TEntity>(size);
			}
		};

		template<typename TEntityTraits>
		static typename TEntityTraits::EntitiesContainer MakeEntitiesWithSizes(const std::vector<uint32_t>& sizes) {
			typename TEntityTraits::EntitiesContainer entities;
			for (auto size : sizes)
				entities.push_back(TEntityTraits::MakeEntityWithSize(size));

			return entities;
		}

		using ConstEntityTraits = EntityTraits<const model::VerifiableEntity>;
		using NonConstEntityTraits = EntityTraits<model::VerifiableEntity>;
	}

#define CONST_NON_CONST_ENTITY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Const) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConstEntityTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonConst) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonConstEntityTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CONST_NON_CONST_ENTITY_TEST(CanCreatePacketFromZeroEntities) {
		// Arrange:
		typename TTraits::EntitiesContainer entities;

		// Act:
		auto payload = PacketPayloadFactory::FromEntities(Test_Packet_Type, entities);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), Test_Packet_Type);
		EXPECT_TRUE(payload.buffers().empty());
	}

	CONST_NON_CONST_ENTITY_TEST(CanCreatePacketFromSingleEntity) {
		// Arrange:
		auto pEntity = TTraits::MakeEntityWithSize(124);

		// Act:
		auto payload = PacketPayloadFactory::FromEntity(Test_Packet_Type, pEntity);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 124u, Test_Packet_Type);
		ASSERT_EQ(1u, payload.buffers().size());

		// - the buffer contains the correct data and points to the original entity
		auto buffer = payload.buffers()[0];
		EXPECT_EQ(test::AsVoidPointer(pEntity.get()), buffer.pData);
		ASSERT_EQ(124u, buffer.Size);
		EXPECT_EQ(*pEntity, *reinterpret_cast<const model::VerifiableEntity*>(buffer.pData));
	}

	CONST_NON_CONST_ENTITY_TEST(CanCreatePacketFromMultipleEntities) {
		// Arrange:
		auto entities = MakeEntitiesWithSizes<TTraits>({ 124, 300, 198 });

		// Act:
		auto payload = PacketPayloadFactory::FromEntities(Test_Packet_Type, entities);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 622u, Test_Packet_Type);
		ASSERT_EQ(3u, payload.buffers().size());

		// - the buffers contain the correct data and point to the original entities
		for (auto i = 0u; i < payload.buffers().size(); ++i) {
			const auto& buffer = payload.buffers()[i];
			EXPECT_EQ(test::AsVoidPointer(entities[i].get()), buffer.pData);
			ASSERT_EQ(entities[i]->Size, buffer.Size);
			EXPECT_EQ(*entities[i], *reinterpret_cast<const model::VerifiableEntity*>(buffer.pData));
		}
	}

	// endregion

	// region FromFixedSizeRange

	TEST(TEST_CLASS, CanCreatePacketFromEmptyFixedSizeRange) {
		// Arrange:
		model::EntityRange<uint32_t> range;

		// Act:
		auto payload = PacketPayloadFactory::FromFixedSizeRange(Test_Packet_Type, std::move(range));

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), Test_Packet_Type);
		EXPECT_TRUE(payload.buffers().empty());
	}

	namespace {
		constexpr std::array<uint8_t, 12> Entity_Range_Buffer{{
			0x00, 0x11, 0x22, 0x33,
			0xFF, 0xDD, 0xBB, 0x99,
			0x76, 0x98, 0x12, 0x34
		}};
	}

	TEST(TEST_CLASS, CanCreatePacketFromFixedSizeRange) {
		// Arrange:
		auto range = model::EntityRange<uint32_t>::CopyFixed(Entity_Range_Buffer.data(), 3);

		// Act:
		auto payload = PacketPayloadFactory::FromFixedSizeRange(Test_Packet_Type, std::move(range));

		// Sanity:
		EXPECT_TRUE(range.empty());

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 12u, Test_Packet_Type);
		ASSERT_EQ(1u, payload.buffers().size());

		auto buffer = payload.buffers()[0];
		ASSERT_EQ(12u, buffer.Size);
		EXPECT_EQ_MEMORY(Entity_Range_Buffer.data(), buffer.pData, buffer.Size);
	}

	// endregion
}}
