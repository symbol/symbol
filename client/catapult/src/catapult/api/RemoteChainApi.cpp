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

#include "RemoteChainApi.h"
#include "ChainPackets.h"
#include "RemoteApiUtils.h"
#include "RemoteRequestDispatcher.h"
#include "catapult/ionet/PacketEntityUtils.h"

namespace catapult { namespace api {

	namespace {
		// region traits

		struct ChainStatisticsTraits {
		public:
			using ResultType = ChainStatistics;
			static constexpr auto Packet_Type = ionet::PacketType::Chain_Statistics;
			static constexpr auto Friendly_Name = "chain statistics";

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(Packet_Type);
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				const auto* pResponse = ionet::CoercePacket<ChainStatisticsResponse>(&packet);
				if (!pResponse)
					return false;

				result.Height = pResponse->Height;
				result.FinalizedHeight = pResponse->FinalizedHeight;
				result.Score = model::ChainScore(pResponse->ScoreHigh, pResponse->ScoreLow);
				return true;
			}
		};

		struct HashesFromTraits {
		public:
			using ResultType = model::HashRange;
			static constexpr auto Packet_Type = ionet::PacketType::Block_Hashes;
			static constexpr auto Friendly_Name = "hashes from";

			static auto CreateRequestPacketPayload(Height height, uint32_t maxHashes) {
				auto pPacket = ionet::CreateSharedPacket<BlockHashesRequest>();
				pPacket->Height = height;
				pPacket->NumHashes = maxHashes;
				return ionet::PacketPayload(pPacket);
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractFixedSizeStructuresFromPacket<Hash256>(packet);
				return !result.empty();
			}
		};

		struct BlockAtTraits : public RegistryDependentTraits<model::Block> {
		public:
			using ResultType = std::shared_ptr<const model::Block>;
			static constexpr auto Packet_Type = ionet::PacketType::Pull_Block;
			static constexpr auto Friendly_Name = "block at";

			static auto CreateRequestPacketPayload(Height height) {
				auto pPacket = ionet::CreateSharedPacket<PullBlockRequest>();
				pPacket->Height = height;
				return ionet::PacketPayload(pPacket);
			}

		public:
			using RegistryDependentTraits::RegistryDependentTraits;

			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractEntityFromPacket<model::Block>(packet, *this);
				return !!result;
			}
		};

		struct BlocksFromTraits : public RegistryDependentTraits<model::Block> {
		public:
			using ResultType = model::BlockRange;
			static constexpr auto Packet_Type = ionet::PacketType::Pull_Blocks;
			static constexpr auto Friendly_Name = "blocks from";

			static auto CreateRequestPacketPayload(Height height, const BlocksFromOptions& options) {
				auto pPacket = ionet::CreateSharedPacket<PullBlocksRequest>();
				pPacket->Height = height;
				pPacket->NumBlocks = options.NumBlocks;
				pPacket->NumResponseBytes = options.NumBytes;
				return ionet::PacketPayload(pPacket);
			}

		public:
			using RegistryDependentTraits::RegistryDependentTraits;

			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractEntitiesFromPacket<model::Block>(packet, *this);
				return !result.empty() || sizeof(ionet::PacketHeader) == packet.Size;
			}
		};

		// endregion

		class DefaultRemoteChainApi : public RemoteChainApi {
		private:
			template<typename TTraits>
			using FutureType = thread::future<typename TTraits::ResultType>;

		public:
			DefaultRemoteChainApi(
					ionet::PacketIo& io,
					const model::NodeIdentity& remoteIdentity,
					const model::TransactionRegistry* pRegistry)
					: RemoteChainApi(remoteIdentity)
					, m_pRegistry(pRegistry)
					, m_impl(io)
			{}

		public:
			FutureType<ChainStatisticsTraits> chainStatistics() const override {
				return m_impl.dispatch(ChainStatisticsTraits());
			}

			FutureType<HashesFromTraits> hashesFrom(Height height, uint32_t maxHashes) const override {
				return m_impl.dispatch(HashesFromTraits(), height, maxHashes);
			}

			FutureType<BlockAtTraits> blockLast() const override {
				return m_impl.dispatch(BlockAtTraits(*m_pRegistry), Height(0));
			}

			FutureType<BlockAtTraits> blockAt(Height height) const override {
				return m_impl.dispatch(BlockAtTraits(*m_pRegistry), height);
			}

			FutureType<BlocksFromTraits> blocksFrom(Height height, const BlocksFromOptions& options) const override {
				return m_impl.dispatch(BlocksFromTraits(*m_pRegistry), height, options);
			}

		private:
			const model::TransactionRegistry* m_pRegistry;
			mutable RemoteRequestDispatcher m_impl;
		};
	}

	std::unique_ptr<ChainApi> CreateRemoteChainApiWithoutRegistry(ionet::PacketIo& io) {
		// since the returned interface is only chain-api, the remote identity and the registry are unused
		return std::make_unique<DefaultRemoteChainApi>(io, model::NodeIdentity(), nullptr);
	}

	std::unique_ptr<RemoteChainApi> CreateRemoteChainApi(
			ionet::PacketIo& io,
			const model::NodeIdentity& remoteIdentity,
			const model::TransactionRegistry& registry) {
		return std::make_unique<DefaultRemoteChainApi>(io, remoteIdentity, &registry);
	}
}}
