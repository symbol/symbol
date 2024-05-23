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

#pragma once
#include "HandlerTypes.h"
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/ShortHash.h"
#include <functional>

namespace catapult {
namespace handlers {

    namespace detail {
        /// Parses a pull request \a packet.
        template <typename TRequest>
        TRequest ParsePullRequest(const ionet::Packet& packet)
        {
            using FilterType = typename TRequest::FilterType;
            using HashType = typename TRequest::HashType;

            // packet is guaranteed to have correct type because this function is only called for matching packets by ServerPacketHandlers
            auto dataSize = ionet::CalculatePacketDataSize(packet);
            if (dataSize < sizeof(FilterType))
                return TRequest();

            // data is prepended with filter value
            TRequest request;
            request.FilterValue = reinterpret_cast<const FilterType&>(*packet.Data());
            dataSize -= sizeof(FilterType);

            // followed by short hashes
            const auto* pHashDataStart = packet.Data() + sizeof(FilterType);
            auto numHashes = ionet::CountFixedSizeStructures<HashType>({ pHashDataStart, dataSize });
            if (0 == numHashes && 0 != dataSize)
                return TRequest();

            TRequest::SetAll(request, reinterpret_cast<const HashType*>(pHashDataStart), numHashes);
            request.IsValid = true;
            return request;
        }
    }

    /// Creates a push handler that forwards a received entity range to \a rangeHandler
    /// given a transaction \a registry composed of supported transaction types.
    template <typename TEntity>
    auto CreatePushEntityHandler(const model::TransactionRegistry& registry, const RangeHandler<TEntity>& rangeHandler)
    {
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
    template <typename TFilterValue>
    struct PullEntitiesHandler {
    private:
        struct ParsedPullRequest {
        public:
            using FilterType = TFilterValue;
            using HashType = utils::ShortHash;

        public:
            bool IsValid = false;
            TFilterValue FilterValue;
            utils::ShortHashesSet ShortHashes;

        public:
            static void SetAll(ParsedPullRequest& request, const utils::ShortHash* pShortHash, size_t count)
            {
                request.ShortHashes.reserve(count);
                for (auto i = 0u; i < count; ++i, ++pShortHash)
                    request.ShortHashes.insert(*pShortHash);
            }
        };

    public:
        /// Creates a handler around \a entitiesRetriever that responds with packets of type \a packetType.
        template <typename TEntitiesRetriever>
        static auto Create(ionet::PacketType packetType, TEntitiesRetriever entitiesRetriever)
        {
            return [packetType, entitiesRetriever](const auto& packet, auto& context) {
                auto request = detail::ParsePullRequest<ParsedPullRequest>(packet);
                if (!request.IsValid)
                    return;

                auto entities = entitiesRetriever(request.FilterValue, request.ShortHashes);
                context.response(ionet::PacketPayloadFactory::FromEntities(packetType, entities));
            };
        }
    };
}
}
