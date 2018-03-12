#include "catapult/ionet/PacketPayload.h"
#include "catapult/model/VerifiableEntity.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS PacketPayloadBuilderTests

	// region empty

	TEST(TEST_CLASS, CanBuildEmptyPayload) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		// Act:
		auto payload = builder.build();

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Chain_Info);
		EXPECT_TRUE(payload.buffers().empty());
	}

	// endregion

	// region entities

	namespace {
		using EntitiesContainer = std::vector<std::shared_ptr<model::VerifiableEntity>>;

		std::shared_ptr<model::VerifiableEntity> MakeEntityWithSize(uint32_t size) {
			auto pEntity = utils::MakeUniqueWithSize<model::VerifiableEntity>(size);
			pEntity->Size = size;

			auto headerSize = model::VerifiableEntity::Header_Size;
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pEntity.get()) + headerSize, size - headerSize });
			return std::move(pEntity);
		}
	}

	TEST(TEST_CLASS, CanBuildPayloadAroundZeroEntities) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		// Act:
		builder.appendEntities(EntitiesContainer());
		auto payload = builder.build();

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Chain_Info);
		EXPECT_TRUE(payload.buffers().empty());
	}

	// endregion

	// region entities - single

	TEST(TEST_CLASS, CanCreatePacketFromSingleEntity) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		auto pEntity = MakeEntityWithSize(124);

		// Act:
		builder.appendEntity(pEntity);
		auto payload = builder.build();

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 124u, PacketType::Chain_Info);
		ASSERT_EQ(1u, payload.buffers().size());

		// - the buffer contains the correct data and points to the original entity
		auto buffer = payload.buffers()[0];
		EXPECT_EQ(test::AsVoidPointer(pEntity.get()), buffer.pData);
		EXPECT_EQ(124u, buffer.Size);
		EXPECT_EQ(*pEntity, *reinterpret_cast<const model::VerifiableEntity*>(buffer.pData));
	}

	TEST(TEST_CLASS, CreatingPacketFromSingleEntityExtendsEntityLifetime) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		auto pEntity = MakeEntityWithSize(124);
		auto pEntityCopy = test::CopyEntity(*pEntity);

		// Act:
		builder.appendEntity(pEntity);
		auto payload = builder.build();
		pEntity.reset();

		// Sanity:
		EXPECT_FALSE(!!pEntity);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 124u, PacketType::Chain_Info);
		ASSERT_EQ(1u, payload.buffers().size());

		// - the buffer still has the original data even though the original entity was deleted
		auto buffer = payload.buffers()[0];
		EXPECT_EQ(*pEntityCopy, *reinterpret_cast<const model::VerifiableEntity*>(buffer.pData));
	}

	TEST(TEST_CLASS, CannotCreatePacketFromSingleEntityWithOverflowingSize) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		auto pEntity = MakeEntityWithSize(124);
		pEntity->Size = 0xFFFF'FFF8;

		// Act + Assert:
		EXPECT_THROW(builder.appendEntity(pEntity), catapult_runtime_error);
	}

	// endregion

	// region entities - multiple

	TEST(TEST_CLASS, CanCreatePacketFromMultipleEntities) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		auto entities = EntitiesContainer{ MakeEntityWithSize(124), MakeEntityWithSize(300), MakeEntityWithSize(198) };

		// Act:
		builder.appendEntities(entities);
		auto payload = builder.build();

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 622u, PacketType::Chain_Info);
		ASSERT_EQ(3u, payload.buffers().size());

		// - the buffers contain the correct data and point to the original entities
		for (auto i = 0u; i < payload.buffers().size(); ++i) {
			const auto& buffer = payload.buffers()[i];
			EXPECT_EQ(test::AsVoidPointer(entities[i].get()), buffer.pData);
			EXPECT_EQ(entities[i]->Size, buffer.Size);
			EXPECT_EQ(*entities[i], *reinterpret_cast<const model::VerifiableEntity*>(buffer.pData));
		}
	}

	TEST(TEST_CLASS, CreatingPacketFromMultipleEntitiesExtendsEntityLifetimes) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		auto entities = EntitiesContainer{ MakeEntityWithSize(124), MakeEntityWithSize(300), MakeEntityWithSize(198) };

		EntitiesContainer copyEntities;
		for (const auto& pEntity : entities)
			copyEntities.push_back(test::CopyEntity(*pEntity));

		// Act:
		auto payload = PacketPayload::FromEntities(PacketType::Chain_Info, entities);
		entities.clear();

		// Sanity:
		EXPECT_TRUE(entities.empty());

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 622u, PacketType::Chain_Info);
		ASSERT_EQ(3u, payload.buffers().size());

		// - the buffers still have the original data even though the original entities were deleted
		for (auto i = 0u; i < payload.buffers().size(); ++i) {
			const auto& buffer = payload.buffers()[i];
			EXPECT_EQ(*copyEntities[i], *reinterpret_cast<const model::VerifiableEntity*>(buffer.pData));
		}
	}

	TEST(TEST_CLASS, CannotCreatePacketFromMultipleEntitiesWithOverflowingSize) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		auto entities = EntitiesContainer{ MakeEntityWithSize(124), MakeEntityWithSize(300), MakeEntityWithSize(198) };

		// - make calculated total size too large (3 * 0x55555553 + 0x08 > 0xFFFFFFFF)
		for (const auto& pEntity : entities)
			pEntity->Size = 0xFFFF'FFFA / 3;

		// Act + Assert:
		EXPECT_THROW(builder.appendEntities(entities), catapult_runtime_error);
	}

	// endregion

	// region range

	TEST(TEST_CLASS, CanCreatePacketFromEmptyFixedSizeRange) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		// Act:
		builder.appendRange(model::EntityRange<uint32_t>());
		auto payload = builder.build();

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Chain_Info);
		EXPECT_TRUE(payload.buffers().empty());
	}

	namespace {
		constexpr std::array<uint8_t, 12> Entity_Range_Buffer{{
			0x00, 0x11, 0x22, 0x33,
			0xFF, 0xDD, 0xBB, 0x99,
			0x76, 0x98, 0x12, 0x34,
		}};
	}

	TEST(TEST_CLASS, CanCreatePacketFromFixedSizeRange) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		// Act:
		builder.appendRange(model::EntityRange<uint32_t>::CopyFixed(Entity_Range_Buffer.data(), 3));
		auto payload = builder.build();

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 12u, PacketType::Chain_Info);
		ASSERT_EQ(1u, payload.buffers().size());

		auto buffer = payload.buffers()[0];
		EXPECT_EQ(12u, buffer.Size);
		EXPECT_TRUE(0 == std::memcmp(Entity_Range_Buffer.data(), buffer.pData, buffer.Size));
	}

	TEST(TEST_CLASS, CanCreatePacketFromOverlaidFixedSizeRange) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		// Act:
		builder.appendRange(model::EntityRange<uint32_t>::CopyVariable(Entity_Range_Buffer.data(), Entity_Range_Buffer.size(), { 2, 6 }));
		auto payload = builder.build();

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 8u, PacketType::Chain_Info);
		ASSERT_EQ(1u, payload.buffers().size());

		auto buffer = payload.buffers()[0];
		EXPECT_EQ(8u, buffer.Size);
		EXPECT_TRUE(0 == std::memcmp(Entity_Range_Buffer.data() + 2u, buffer.pData, buffer.Size));
	}

	// endregion

	// region value

	TEST(TEST_CLASS, CanCreatePacketFromUint32) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		// Act:
		builder.appendValue<uint32_t>(0x03981204);
		auto payload = builder.build();

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 4u, PacketType::Chain_Info);
		ASSERT_EQ(1u, payload.buffers().size());

		auto buffer = payload.buffers()[0];
		EXPECT_EQ(4u, buffer.Size);
		EXPECT_EQ(0x03981204, reinterpret_cast<const uint32_t&>(*buffer.pData));
	}

	TEST(TEST_CLASS, CanCreatePacketFromArray) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		auto hash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		builder.appendValue(hash);
		auto payload = builder.build();

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + Hash256_Size, PacketType::Chain_Info);
		ASSERT_EQ(1u, payload.buffers().size());

		auto buffer = payload.buffers()[0];
		EXPECT_EQ(Hash256_Size, buffer.Size);
		EXPECT_EQ(hash, reinterpret_cast<const Hash256&>(*buffer.pData));
	}

	// endregion

	// region mixed

	TEST(TEST_CLASS, CanCreatePacketFromHeterogeneousSources) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Info);

		auto pEntity = MakeEntityWithSize(124);

		// Act:
		builder.appendValue<uint32_t>(0x03981204);
		builder.appendEntity(pEntity);
		builder.appendValue<uint32_t>(0x11111111);
		builder.appendRange(model::EntityRange<uint32_t>::CopyFixed(Entity_Range_Buffer.data(), 2));
		builder.appendValue<uint32_t>(0x00003322);
		auto payload = builder.build();

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 3u * 4 + 124 + 8, PacketType::Chain_Info);
		ASSERT_EQ(5u, payload.buffers().size());

		const auto& buffers = payload.buffers();
		EXPECT_EQ(4u, buffers[0].Size);
		EXPECT_EQ(0x03981204, reinterpret_cast<const uint32_t&>(*buffers[0].pData));

		EXPECT_EQ(test::AsVoidPointer(pEntity.get()), buffers[1].pData);
		EXPECT_EQ(124u, buffers[1].Size);
		EXPECT_EQ(*pEntity, *reinterpret_cast<const model::VerifiableEntity*>(buffers[1].pData));

		EXPECT_EQ(4u, buffers[2].Size);
		EXPECT_EQ(0x11111111, reinterpret_cast<const uint32_t&>(*buffers[2].pData));

		EXPECT_EQ(8u, buffers[3].Size);
		EXPECT_TRUE(0 == std::memcmp(Entity_Range_Buffer.data(), buffers[3].pData, buffers[3].Size));

		EXPECT_EQ(4u, buffers[4].Size);
		EXPECT_EQ(0x00003322, reinterpret_cast<const uint32_t&>(*buffers[4].pData));
	}

	// endregion
}}
