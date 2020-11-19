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

#include "catapult/handlers/MerkleHandlers.h"
#include "catapult/api/ChainPackets.h"
#include "tests/catapult/handlers/test/HeightRequestHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS MerkleHandlersTests

	namespace {
		constexpr auto CreateStorage = test::VariableSizedBlockChain::CreateStorage;
	}

	// region SubCacheMerkleRootsHandler

	namespace {
		using SubCacheMerkleRootsRequestPacket = api::HeightPacket<ionet::PacketType::Sub_Cache_Merkle_Roots>;

		struct SubCacheMerkleRootsHandlerTraits {
			static ionet::PacketType ResponsePacketType() {
				return SubCacheMerkleRootsRequestPacket::Packet_Type;
			}

			static auto CreateRequestPacket() {
				return ionet::CreateSharedPacket<SubCacheMerkleRootsRequestPacket>();
			}

			static void Register(ionet::ServerPacketHandlers& handlers, const io::BlockStorageCache& storage) {
				RegisterSubCacheMerkleRootsHandler(handlers, storage);
			}
		};
	}

	DEFINE_HEIGHT_REQUEST_HANDLER_TESTS(TEST_CLASS, SubCacheMerkleRootsHandler)

	namespace {
		void AssertCanRetrieveSubCacheMerkleRoots(size_t numBlocks, Height requestHeight) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto pStorage = CreateStorage(numBlocks);
			RegisterSubCacheMerkleRootsHandler(handlers, *pStorage);

			auto pPacket = ionet::CreateSharedPacket<SubCacheMerkleRootsRequestPacket>();
			pPacket->Height = requestHeight;

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert:
			const auto Num_Expected_Hashes = 3u;
			auto expectedSize = sizeof(ionet::PacketHeader) + Num_Expected_Hashes * Hash256::Size;
			test::AssertPacketHeader(handlerContext, expectedSize, SubCacheMerkleRootsRequestPacket::Packet_Type);

			auto pBlockElementFromStorage = pStorage->view().loadBlockElement(requestHeight);
			ASSERT_EQ(Num_Expected_Hashes, pBlockElementFromStorage->SubCacheMerkleRoots.size());

			auto pHashesFromPacket = reinterpret_cast<const Hash256*>(test::GetSingleBufferData(handlerContext));
			for (auto i = 0u; i < Num_Expected_Hashes; ++i)
				EXPECT_EQ(pBlockElementFromStorage->SubCacheMerkleRoots[i], pHashesFromPacket[i]) << "hash at " << i;
		}
	}

	TEST(TEST_CLASS, SubCacheMerkleRootsHandler_WritesSubCacheMerkleRootsWhenPresent) {
		AssertCanRetrieveSubCacheMerkleRoots(12, Height(7));
	}

	TEST(TEST_CLASS, SubCacheMerkleRootsHandler_CanRetrieveLastBlockSubCacheMerkleRoots) {
		AssertCanRetrieveSubCacheMerkleRoots(12, Height(12));
	}

	TEST(TEST_CLASS, SubCacheMerkleRootsHandler_WritesEmptyResponseWhenSubCacheMerkleRootsAreNotPresent) {
		// Arrange:
		ionet::ServerPacketHandlers handlers;
		auto pStorage = CreateStorage(12);
		RegisterSubCacheMerkleRootsHandler(handlers, *pStorage);

		// - remove merkle hashes from last block
		auto pBlockElementFromStorage = pStorage->view().loadBlockElement(Height(7));
		const_cast<model::BlockElement&>(*pBlockElementFromStorage).SubCacheMerkleRoots.clear();

		auto pPacket = ionet::CreateSharedPacket<SubCacheMerkleRootsRequestPacket>();
		pPacket->Height = Height(7);

		// Act:
		ionet::ServerPacketHandlerContext handlerContext;
		EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

		// Assert: only a payload header is written
		test::AssertPacketHeader(handlerContext, sizeof(ionet::PacketHeader), SubCacheMerkleRootsRequestPacket::Packet_Type);
		EXPECT_TRUE(handlerContext.response().buffers().empty());
	}

	// endregion
}}
