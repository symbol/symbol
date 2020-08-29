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

#include "catapult/handlers/HandlerUtils.h"
#include "catapult/model/Block.h"
#include "catapult/model/TransactionPlugin.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS HandlerUtilsTests

	// region CreatePushEntityHandler

	namespace {
		constexpr auto Block_Packet_Size = sizeof(ionet::PacketHeader) + sizeof(model::BlockHeader);
		constexpr auto Two_Blocks_Packet_Size = sizeof(ionet::PacketHeader) + 2 * sizeof(model::BlockHeader);

		void AssertCreatePushEntityHandlerForwarding(const ionet::Packet& packet, size_t numExpectedForwards) {
			// Arrange:
			model::TransactionRegistry registry;
			model::NodeIdentity capturedSourceIdentity;
			auto counter = 0u;
			auto handler = CreatePushEntityHandler<model::Block>(registry, [&capturedSourceIdentity, &counter](const auto& range) {
				capturedSourceIdentity = range.SourceIdentity;
				++counter;
			});

			// Act:
			auto sourcePublicKey = test::GenerateRandomByteArray<Key>();
			auto sourceHost = std::string("11.22.33.44");
			handler(packet, ionet::ServerPacketHandlerContext(sourcePublicKey, sourceHost));

			// Assert:
			EXPECT_EQ(numExpectedForwards, counter);

			// - if the callback was called, context should have been forwarded along with the range
			if (numExpectedForwards > 0) {
				EXPECT_EQ(sourcePublicKey, capturedSourceIdentity.PublicKey);
				EXPECT_EQ("11.22.33.44", capturedSourceIdentity.Host);
			}
		}
	}

	TEST(TEST_CLASS, CreatePushEntityHandler_DoesNotForwardMalformedEntityToRangeHandler) {
		// Arrange:
		ionet::ByteBuffer buffer(Block_Packet_Size);
		auto& packet = test::SetPushBlockPacketInBuffer(buffer);
		--packet.Size;

		// Assert:
		AssertCreatePushEntityHandlerForwarding(packet, 0);
	}

	TEST(TEST_CLASS, CreatePushEntityHandler_ForwardsWellFormedEntityToRangeHandler) {
		// Arrange:
		ionet::ByteBuffer buffer(Block_Packet_Size);
		const auto& packet = test::SetPushBlockPacketInBuffer(buffer);

		// Assert:
		AssertCreatePushEntityHandlerForwarding(packet, 1);
	}

	TEST(TEST_CLASS, CreatePushEntityHandler_DoesNotForwardMalformedEntitiesToRangeHandler) {
		// Arrange:
		ionet::ByteBuffer buffer(Two_Blocks_Packet_Size);
		auto& packet = test::SetPushBlockPacketInBuffer(buffer);
		test::SetBlockAt(buffer, sizeof(ionet::Packet));
		test::SetBlockAt(buffer, sizeof(ionet::Packet) + sizeof(model::BlockHeader));
		--packet.Size;

		// Assert:
		AssertCreatePushEntityHandlerForwarding(packet, 0);
	}

	TEST(TEST_CLASS, CreatePushEntityHandler_ForwardsWellFormedEntitiesToRangeHandler) {
		// Arrange:
		ionet::ByteBuffer buffer(Two_Blocks_Packet_Size);
		const auto& packet = test::SetPushBlockPacketInBuffer(buffer);
		test::SetBlockAt(buffer, sizeof(ionet::Packet));
		test::SetBlockAt(buffer, sizeof(ionet::Packet) + sizeof(model::BlockHeader));

		// Assert:
		AssertCreatePushEntityHandlerForwarding(packet, 1);
	}

	// endregion

	// region PullEntitiesHandler::Create

	namespace {
		auto Min_Pull_Packet_Size = SizeOf32<ionet::PacketHeader>() + SizeOf32<Height>(); // use Height as filter in tests
		auto Test_Pull_Packet_Type = static_cast<ionet::PacketType>(123);

		struct TestPullEntity : model::SizePrefixedEntity {
			Height FilterHeight;
			utils::ShortHash ShortHash;
		};

		ionet::Packet& CoerceToPullPacket(ionet::ByteBuffer& buffer) {
			auto& packet = reinterpret_cast<ionet::Packet&>(buffer[0]);
			packet.Size = Min_Pull_Packet_Size;
			packet.Type = Test_Pull_Packet_Type;
			return packet;
		}

		template<typename TCheckContext>
		void RunCreatePullEntitiesHandlerTest(const ionet::Packet& packet, size_t numExpectedRetrieverCalls, TCheckContext checkContext) {
			// Arrange:
			auto numRetrieverCalls = 0u;
			auto handler = PullEntitiesHandler<Height>::Create(Test_Pull_Packet_Type, [&numRetrieverCalls](
					auto filterHeight,
					const auto& shortHashes) {
				++numRetrieverCalls;

				std::vector<std::shared_ptr<const model::SizePrefixedEntity>> entities;
				for (const auto& shortHash : shortHashes) {
					auto pEntity = std::make_shared<TestPullEntity>();
					pEntity->Size = sizeof(TestPullEntity);
					pEntity->FilterHeight = filterHeight;
					pEntity->ShortHash = shortHash;
					entities.push_back(pEntity);
				}

				return entities;
			});

			// Act:
			auto sourcePublicKey = test::GenerateRandomByteArray<Key>();
			auto sourceHost = std::string("11.22.33.44");
			auto context = ionet::ServerPacketHandlerContext(sourcePublicKey, sourceHost);
			handler(packet, context);

			// Assert:
			EXPECT_EQ(numExpectedRetrieverCalls, numRetrieverCalls);
			checkContext(context);
		}
	}

	TEST(TEST_CLASS, CreatePullEntitiesHandler_DoesNotRespondWhenRequestPacketIsTooSmall) {
		// Arrange:
		ionet::ByteBuffer buffer(Min_Pull_Packet_Size);
		auto& packet = CoerceToPullPacket(buffer);
		--packet.Size;

		// Act:
		RunCreatePullEntitiesHandlerTest(packet, 0, [](const auto& context) {
			// Assert:
			EXPECT_FALSE(context.hasResponse());
		});
	}

	TEST(TEST_CLASS, CreatePullEntitiesHandler_RespondsWhenRequestPacketHasNoShortHashes) {
		// Arrange:
		ionet::ByteBuffer buffer(Min_Pull_Packet_Size);
		auto& packet = CoerceToPullPacket(buffer);

		// Act:
		RunCreatePullEntitiesHandlerTest(packet, 1, [](const auto& context) {
			// Assert:
			test::AssertPacketHeader(context, sizeof(ionet::PacketHeader), Test_Pull_Packet_Type);
		});
	}

	TEST(TEST_CLASS, CreatePullEntitiesHandler_DoesNotRespondWhenRequestPacketHasCorruptShortHashes) {
		// Arrange:
		ionet::ByteBuffer buffer(Min_Pull_Packet_Size + 3 * sizeof(utils::ShortHash));
		test::FillWithRandomData(buffer);

		auto& packet = CoerceToPullPacket(buffer);
		packet.Size += 3 * SizeOf32<utils::ShortHash>() - 1;

		// Act:
		RunCreatePullEntitiesHandlerTest(packet, 0, [](const auto& context) {
			// Assert:
			EXPECT_FALSE(context.hasResponse());
		});
	}

	TEST(TEST_CLASS, CreatePullEntitiesHandler_RespondsWhenRequestPacketHasShortHashes) {
		// Arrange:
		ionet::ByteBuffer buffer(Min_Pull_Packet_Size + 3 * sizeof(utils::ShortHash));
		test::FillWithRandomData(buffer);

		auto& packet = CoerceToPullPacket(buffer);
		packet.Size += 3 * SizeOf32<utils::ShortHash>();

		// Act:
		RunCreatePullEntitiesHandlerTest(packet, 1, [&packet](const auto& context) {
			// Assert:
			test::AssertPacketHeader(context, sizeof(ionet::PacketHeader) + 3 * sizeof(TestPullEntity), Test_Pull_Packet_Type);
			ASSERT_EQ(3u, context.response().buffers().size());

			auto i = 0u;
			utils::ShortHashesSet requestShortHashes;
			utils::ShortHashesSet responseShortHashes;

			auto requestFilterHeight = reinterpret_cast<const Height&>(*packet.Data());
			const auto* pRequestShortHashes = reinterpret_cast<const utils::ShortHash*>(packet.Data() + sizeof(Height));
			for (const auto& responseBuffer : context.response().buffers()) {
				const auto& entity = reinterpret_cast<const TestPullEntity&>(*responseBuffer.pData);

				ASSERT_EQ(sizeof(TestPullEntity), entity.Size) << "entity at " << i;
				EXPECT_EQ(requestFilterHeight, entity.FilterHeight) << "entity at " << i;

				requestShortHashes.insert(pRequestShortHashes[i]);
				responseShortHashes.insert(entity.ShortHash);
				++i;
			}

			// - request hashes are parsed into unordered_set, so order is not deterministic
			EXPECT_EQ(requestShortHashes, responseShortHashes);
		});
	}

	// endregion
}}
