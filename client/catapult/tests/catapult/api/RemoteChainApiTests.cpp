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

#include "catapult/api/RemoteChainApi.h"
#include "catapult/api/ChainPackets.h"
#include "catapult/model/TransactionPlugin.h"
#include "tests/test/other/RemoteApiFactory.h"
#include "tests/test/other/RemoteApiTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace api {

	namespace {
		std::shared_ptr<ionet::Packet> CreatePacketWithBlocks(uint32_t numBlocks, Height startHeight) {
			uint32_t payloadSize = numBlocks * SizeOf32<model::BlockHeader>();
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
			test::FillWithRandomData({ pPacket->Data(), payloadSize });

			auto* pData = pPacket->Data();
			for (auto i = 0u; i < numBlocks; ++i, pData += sizeof(model::BlockHeader)) {
				auto& block = reinterpret_cast<model::Block&>(*pData);
				block.Size = sizeof(model::BlockHeader);
				block.Type = model::Entity_Type_Block;
				block.Height = startHeight + Height(i);
			}

			return pPacket;
		}

		struct ChainStatisticsTraits {
			static auto Invoke(const ChainApi& api) {
				return api.chainStatistics();
			}

			static auto CreateValidResponsePacket() {
				auto pResponsePacket = ionet::CreateSharedPacket<ChainStatisticsResponse>();
				pResponsePacket->Height = Height(625);
				pResponsePacket->FinalizedHeight = Height(256);
				pResponsePacket->ScoreHigh = 0x1234567812345678;
				pResponsePacket->ScoreLow = 0xABCDABCDABCDABCD;
				return pResponsePacket;
			}

			static auto CreateMalformedResponsePacket() {
				// just change the size because no responses are intrinsically invalid
				auto pResponsePacket = CreateValidResponsePacket();
				--pResponsePacket->Size;
				return pResponsePacket;
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				EXPECT_TRUE(ionet::IsPacketValid(packet, ChainStatisticsResponse::Packet_Type));
			}

			static void ValidateResponse(const ionet::Packet&, const ChainStatistics& chainStatistics) {
				EXPECT_EQ(Height(625), chainStatistics.Height);
				EXPECT_EQ(Height(256), chainStatistics.FinalizedHeight);

				auto scoreArray = chainStatistics.Score.toArray();
				EXPECT_EQ(0x1234567812345678u, scoreArray[0]);
				EXPECT_EQ(0xABCDABCDABCDABCDu, scoreArray[1]);
			}
		};

		struct HashesFromTraits {
			static constexpr auto Request_Height = Height(521);

			static auto Invoke(const ChainApi& api) {
				return api.hashesFrom(Request_Height, 123);
			}

			static auto CreateValidResponsePacket(uint32_t payloadSize = 3u * sizeof(Hash256)) {
				auto pResponsePacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				pResponsePacket->Type = ionet::PacketType::Block_Hashes;
				test::FillWithRandomData({ pResponsePacket->Data(), payloadSize });
				return pResponsePacket;
			}

			static auto CreateMalformedResponsePacket() {
				// the packet is malformed because it contains a partial packet (1.5 packets in all)
				return CreateValidResponsePacket(3 * sizeof(Hash256) / 2);
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				const auto* pRequest = ionet::CoercePacket<BlockHashesRequest>(&packet);
				ASSERT_TRUE(!!pRequest);
				EXPECT_EQ(Request_Height, pRequest->Height);
				EXPECT_EQ(123u, pRequest->NumHashes);
			}

			static void ValidateResponse(const ionet::Packet& response, const model::HashRange& hashes) {
				ASSERT_EQ(3u, hashes.size());

				auto iter = hashes.cbegin();
				for (auto i = 0u; i < hashes.size(); ++i) {
					auto pExpectedHash = response.Data() + i * sizeof(Hash256);
					auto pActualHash = iter->data();
					EXPECT_EQ_MEMORY(pExpectedHash, pActualHash, sizeof(Hash256)) << "comparing hashes at " << i;
					++iter;
				}
			}
		};

		struct BlockLastInvoker {
			static constexpr auto Request_Height = Height(0);

			static auto Invoke(const RemoteChainApi& api) {
				return api.blockLast();
			}
		};

		struct BlockAtInvoker {
			static constexpr auto Request_Height = Height(728);

			static auto Invoke(const RemoteChainApi& api) {
				return api.blockAt(Request_Height);
			}
		};

		template<typename TInvoker>
		struct BlockAtTraitsT : public TInvoker {
			static auto CreateValidResponsePacket(uint32_t numBlocks = 1) {
				auto pResponsePacket = CreatePacketWithBlocks(numBlocks, TInvoker::Request_Height);
				pResponsePacket->Type = ionet::PacketType::Pull_Block;
				return pResponsePacket;
			}

			static auto CreateMalformedResponsePacket() {
				// block-at api can only return a single block
				return CreateValidResponsePacket(2);
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				const auto* pRequest = ionet::CoercePacket<PullBlockRequest>(&packet);
				ASSERT_TRUE(!!pRequest);
				EXPECT_EQ(TInvoker::Request_Height, pRequest->Height);
			}

			static void ValidateResponse(const ionet::Packet& response, const std::shared_ptr<const model::Block>& pBlock) {
				ASSERT_EQ(response.Size - sizeof(ionet::Packet), pBlock->Size);
				ASSERT_EQ(sizeof(model::BlockHeader), pBlock->Size);
				EXPECT_EQ(TInvoker::Request_Height, pBlock->Height);
				EXPECT_EQ_MEMORY(response.Data(), pBlock.get(), pBlock->Size);
			}
		};

		using BlockLastTraits = BlockAtTraitsT<BlockLastInvoker>;
		using BlockAtTraits = BlockAtTraitsT<BlockAtInvoker>;

		struct BlocksFromTraits {
			static constexpr auto Request_Height = Height(823);

			static auto Invoke(const RemoteChainApi& api) {
				return api.blocksFrom(Request_Height, { 200, 1024 });
			}

			static auto CreateValidResponsePacket() {
				auto pResponsePacket = CreatePacketWithBlocks(3, Request_Height);
				pResponsePacket->Type = ionet::PacketType::Pull_Blocks;
				return pResponsePacket;
			}

			static auto CreateMalformedResponsePacket() {
				// the packet is malformed because it contains a partial block
				auto pResponsePacket = CreateValidResponsePacket();
				--pResponsePacket->Size;
				return pResponsePacket;
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				const auto* pRequest = ionet::CoercePacket<PullBlocksRequest>(&packet);
				ASSERT_TRUE(!!pRequest);
				EXPECT_EQ(Request_Height, pRequest->Height);
				EXPECT_EQ(200u, pRequest->NumBlocks);
				EXPECT_EQ(1024u, pRequest->NumResponseBytes);
			}

			static void ValidateResponse(const ionet::Packet& response, const model::BlockRange& blocks) {
				ASSERT_EQ(3u, blocks.size());

				const auto* pData = response.Data();
				auto iter = blocks.cbegin();
				for (auto i = 0u; i < blocks.size(); ++i) {
					std::string message = "comparing blocks at " + std::to_string(i);
					const auto& expectedBlock = reinterpret_cast<const model::Block&>(*pData);
					const auto& actualBlock = *iter;
					ASSERT_EQ(expectedBlock.Size, actualBlock.Size) << message;
					EXPECT_EQ(Request_Height + Height(i), actualBlock.Height) << message;
					EXPECT_EQ(expectedBlock, actualBlock) << message;
					++iter;
					pData += expectedBlock.Size;
				}
			}
		};

		struct RemoteChainApiBlocklessTraits {
			static auto Create(ionet::PacketIo& packetIo) {
				return CreateRemoteChainApiWithoutRegistry(packetIo);
			}
		};

		struct RemoteChainApiTraits {
			static auto Create(ionet::PacketIo& packetIo, const model::NodeIdentity& remoteIdentity) {
				return test::CreateLifetimeExtendedApi(CreateRemoteChainApi, packetIo, remoteIdentity, model::TransactionRegistry());
			}

			static auto Create(ionet::PacketIo& packetIo) {
				return Create(packetIo, model::NodeIdentity());
			}
		};
	}

	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteChainApiBlockless, ChainStatistics)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteChainApiBlockless, HashesFrom)

	DEFINE_REMOTE_API_TESTS(RemoteChainApi)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteChainApi, ChainStatistics)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteChainApi, HashesFrom)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteChainApi, BlockLast)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteChainApi, BlockAt)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_VALID(RemoteChainApi, BlocksFrom)
}}
