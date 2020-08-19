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

#include "ChainHandlers.h"
#include "HandlerUtils.h"
#include "HeightRequestProcessor.h"
#include "catapult/api/ChainPackets.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockUtils.h"

namespace catapult { namespace handlers {

	void RegisterPushBlockHandler(
			ionet::ServerPacketHandlers& handlers,
			const model::TransactionRegistry& registry,
			const BlockRangeHandler& blockRangeHandler) {
		handlers.registerHandler(ionet::PacketType::Push_Block, CreatePushEntityHandler<model::Block>(registry, blockRangeHandler));
	}

	namespace {
		auto CreatePullBlockHandler(const io::BlockStorageCache& storage) {
			return [&storage](const auto& packet, auto& context) {
				using RequestType = api::PullBlockRequest;
				auto storageView = storage.view();
				auto info = HeightRequestProcessor<RequestType>::Process(storageView, packet, context, true);
				if (!info.pRequest)
					return;

				auto pBlock = storageView.loadBlock(info.NormalizedRequestHeight);

				auto payload = ionet::PacketPayloadFactory::FromEntity(RequestType::Packet_Type, std::move(pBlock));
				context.response(std::move(payload));
			};
		}
	}

	void RegisterPullBlockHandler(ionet::ServerPacketHandlers& handlers, const io::BlockStorageCache& storage) {
		handlers.registerHandler(ionet::PacketType::Pull_Block, CreatePullBlockHandler(storage));
	}

	namespace {
		auto CreateChainStatisticsHandler(
				const io::BlockStorageCache& storage,
				const model::ChainScoreSupplier& chainScoreSupplier,
				const supplier<Height>& finalizedHeightSupplier) {
			return [&storage, chainScoreSupplier, finalizedHeightSupplier](const auto& packet, auto& context) {
				using RequestType = api::ChainStatisticsResponse;
				if (!ionet::IsPacketValid(packet, RequestType::Packet_Type))
					return;

				auto pResponsePacket = ionet::CreateSharedPacket<RequestType>();
				pResponsePacket->Height = storage.view().chainHeight();
				pResponsePacket->FinalizedHeight = finalizedHeightSupplier();

				auto scoreArray = chainScoreSupplier().toArray();
				pResponsePacket->ScoreHigh = scoreArray[0];
				pResponsePacket->ScoreLow = scoreArray[1];
				context.response(ionet::PacketPayload(pResponsePacket));
			};
		}
	}

	void RegisterChainStatisticsHandler(
			ionet::ServerPacketHandlers& handlers,
			const io::BlockStorageCache& storage,
			const model::ChainScoreSupplier& chainScoreSupplier,
			const supplier<Height>& finalizedHeightSupplier) {
		handlers.registerHandler(
				ionet::PacketType::Chain_Statistics,
				CreateChainStatisticsHandler(storage, chainScoreSupplier, finalizedHeightSupplier));
	}

	namespace {
		uint32_t ClampNumHashes(const HeightRequestInfo<api::BlockHashesRequest>& info, uint32_t maxHashes) {
			auto numHashes = std::min(maxHashes, info.pRequest->NumHashes);
			return std::min(numHashes, info.numAvailableBlocks());
		}

		auto CreateBlockHashesHandler(const io::BlockStorageCache& storage, uint32_t maxHashes) {
			return [&storage, maxHashes](const auto& packet, auto& context) {
				using RequestType = api::BlockHashesRequest;
				auto storageView = storage.view();
				auto info = HeightRequestProcessor<RequestType>::Process(storageView, packet, context, false);
				if (!info.pRequest)
					return;

				auto numHashes = ClampNumHashes(info, maxHashes);
				auto hashRange = storageView.loadHashesFrom(info.pRequest->Height, numHashes);
				auto payload = ionet::PacketPayloadFactory::FromFixedSizeRange(RequestType::Packet_Type, std::move(hashRange));
				context.response(std::move(payload));
			};
		}
	}

	void RegisterBlockHashesHandler(ionet::ServerPacketHandlers& handlers, const io::BlockStorageCache& storage, uint32_t maxHashes) {
		handlers.registerHandler(ionet::PacketType::Block_Hashes, CreateBlockHashesHandler(storage, maxHashes));
	}

	namespace {
		uint32_t ClampNumBlocks(const HeightRequestInfo<api::PullBlocksRequest>& info, const PullBlocksHandlerConfiguration& config) {
			auto numBlocks = std::min(config.MaxBlocks, info.pRequest->NumBlocks);
			return std::min(numBlocks, info.numAvailableBlocks());
		}

		uint32_t ClampNumResponseBytes(
				const HeightRequestInfo<api::PullBlocksRequest>& info,
				const PullBlocksHandlerConfiguration& config) {
			return std::min(config.MaxResponseBytes, info.pRequest->NumResponseBytes);
		}

		auto CreatePullBlocksHandler(const io::BlockStorageCache& storage, const PullBlocksHandlerConfiguration& config) {
			return [&storage, config](const auto& packet, auto& context) {
				using RequestType = api::PullBlocksRequest;
				auto storageView = storage.view();
				auto info = HeightRequestProcessor<RequestType>::Process(storageView, packet, context, false);
				if (!info.pRequest)
					return;

				auto numBlocks = ClampNumBlocks(info, config);
				auto numResponseBytes = ClampNumResponseBytes(info, config);

				uint32_t payloadSize = 0;
				std::vector<std::shared_ptr<const model::Block>> blocks;
				for (auto i = 0u; i < numBlocks; ++i) {
					// always return at least one block
					auto pBlock = storageView.loadBlock(info.pRequest->Height + Height(i));
					if (!blocks.empty() && payloadSize + pBlock->Size > numResponseBytes)
						break;

					payloadSize += pBlock->Size;
					blocks.push_back(std::move(pBlock));
				}

				auto payload = ionet::PacketPayloadFactory::FromEntities(RequestType::Packet_Type, blocks);
				context.response(std::move(payload));
			};
		}
	}

	void RegisterPullBlocksHandler(
			ionet::ServerPacketHandlers& handlers,
			const io::BlockStorageCache& storage,
			const PullBlocksHandlerConfiguration& config) {
		handlers.registerHandler(ionet::PacketType::Pull_Blocks, CreatePullBlocksHandler(storage, config));
	}
}}
