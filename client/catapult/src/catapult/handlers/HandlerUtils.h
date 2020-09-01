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

#pragma once
#include "HandlerTypes.h"
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/ShortHash.h"
#include <functional>

namespace catapult { namespace handlers {

	/// Creates a push handler that forwards a received entity range to \a rangeHandler
	/// given a transaction \a registry composed of supported transaction types.
	template<typename TEntity>
	auto CreatePushEntityHandler(const model::TransactionRegistry& registry, const RangeHandler<TEntity>& rangeHandler) {
		return [rangeHandler, &registry](const ionet::Packet& packet, const auto& context) {
			auto range = ionet::ExtractEntitiesFromPacket<TEntity>(packet, [&registry](const auto& entity) {
				return IsSizeValid(entity, registry);
			});
			if (range.empty()) {
				CATAPULT_LOG(warning) << "rejecting empty range: " << packet;
				return;
			}

			CATAPULT_LOG(trace) << "received valid " << packet;
			rangeHandler({ std::move(range), { context.key(), context.host() } });
		};
	}

	/// Provides a pull entities handler implementation that allows filtering by TFilterValue and short hashes.
	template<typename TFilterValue>
	struct PullEntitiesHandler {
	private:
		struct ParsedPullRequest {
			bool IsValid = false;
			TFilterValue FilterValue;
			utils::ShortHashesSet ShortHashes;
		};

	public:
		/// Creates a handler around \a entitiesRetriever that responds with packets of type \a packetType.
		template<typename TEntitiesRetriever>
		static auto Create(ionet::PacketType packetType, TEntitiesRetriever entitiesRetriever) {
			return [packetType, entitiesRetriever](const auto& packet, auto& context) {
				auto request = ProcessPullRequest(packet);
				if (!request.IsValid)
					return;

				auto entities = entitiesRetriever(request.FilterValue, request.ShortHashes);
				context.response(ionet::PacketPayloadFactory::FromEntities(packetType, entities));
			};
		}

	private:
		static ParsedPullRequest ProcessPullRequest(const ionet::Packet& packet) {
			// packet is guaranteed to have correct type because this function is only called for matching packets by ServerPacketHandlers
			auto dataSize = ionet::CalculatePacketDataSize(packet);
			if (dataSize < sizeof(TFilterValue))
				return ParsedPullRequest();

			// data is prepended with filter value
			ParsedPullRequest request;
			request.FilterValue = reinterpret_cast<const TFilterValue&>(*packet.Data());
			dataSize -= sizeof(TFilterValue);

			// followed by short hashes
			const auto* pShortHashDataStart = packet.Data() + sizeof(TFilterValue);
			auto numShortHashes = ionet::CountFixedSizeStructures<utils::ShortHash>({ pShortHashDataStart, dataSize });
			if (0 == numShortHashes && 0 != dataSize)
				return ParsedPullRequest();

			const auto* pShortHash = reinterpret_cast<const utils::ShortHash*>(pShortHashDataStart);
			request.ShortHashes.reserve(numShortHashes);
			for (auto i = 0u; i < numShortHashes; ++i, ++pShortHash)
				request.ShortHashes.insert(*pShortHash);

			request.IsValid = true;
			return request;
		}
	};
}}
