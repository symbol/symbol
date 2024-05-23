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

#include "ProofHandlers.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "finalization/src/api/FinalizationPackets.h"
#include "finalization/src/io/ProofStorageCache.h"

namespace catapult {
namespace handlers {

    namespace {
        auto CreateFinalizationStatisticsHandler(const io::ProofStorageCache& proofStorage)
        {
            return [&proofStorage](const auto& packet, auto& context) {
                using RequestType = api::FinalizationStatisticsResponse;
                if (!ionet::IsPacketValid(packet, RequestType::Packet_Type))
                    return;

                auto finalizationStatistics = proofStorage.view().statistics();

                auto pResponsePacket = ionet::CreateSharedPacket<RequestType>();
                pResponsePacket->Round = finalizationStatistics.Round;
                pResponsePacket->Height = finalizationStatistics.Height;
                pResponsePacket->Hash = finalizationStatistics.Hash;
                context.response(ionet::PacketPayload(pResponsePacket));
            };
        }
    }

    void RegisterFinalizationStatisticsHandler(ionet::ServerPacketHandlers& handlers, const io::ProofStorageCache& proofStorage)
    {
        handlers.registerHandler(ionet::PacketType::Finalization_Statistics, CreateFinalizationStatisticsHandler(proofStorage));
    }

    namespace {
        struct ProofAtEpochTraits {
            using RequestType = api::ProofAtEpochRequest;

            static auto LoadProof(const io::ProofStorageView& proofStorageView, const RequestType& request)
            {
                return FinalizationEpoch() == request.Epoch || request.Epoch > proofStorageView.statistics().Round.Epoch
                    ? nullptr
                    : proofStorageView.loadProof(request.Epoch);
            }
        };

        struct ProofAtHeightTraits {
            using RequestType = api::ProofAtHeightRequest;

            static auto LoadProof(const io::ProofStorageView& proofStorageView, const RequestType& request)
            {
                return Height() == request.Height || request.Height > proofStorageView.statistics().Height
                    ? nullptr
                    : proofStorageView.loadProof(request.Height);
            }
        };

        template <typename TTraits>
        auto CreateFinalizationProofHandler(const io::ProofStorageCache& proofStorage)
        {
            return [&proofStorage](const auto& packet, auto& context) {
                const auto* pRequest = ionet::CoercePacket<typename TTraits::RequestType>(&packet);
                if (!pRequest)
                    return;

                auto proofStorageView = proofStorage.view();
                auto pProof = TTraits::LoadProof(proofStorageView, *pRequest);

                if (!pProof) {
                    auto pResponsePacket = ionet::CreateSharedPacket<ionet::Packet>();
                    pResponsePacket->Type = ionet::PacketType::Pull_Finalization_Proof;
                    context.response(ionet::PacketPayload(pResponsePacket));
                    return;
                }

                auto payload = ionet::PacketPayloadFactory::FromEntity(ionet::PacketType::Pull_Finalization_Proof, std::move(pProof));
                context.response(std::move(payload));
            };
        }
    }

    void RegisterFinalizationProofAtEpochHandler(ionet::ServerPacketHandlers& handlers, const io::ProofStorageCache& proofStorage)
    {
        handlers.registerHandler(
            ionet::PacketType::Finalization_Proof_At_Epoch,
            CreateFinalizationProofHandler<ProofAtEpochTraits>(proofStorage));
    }

    void RegisterFinalizationProofAtHeightHandler(ionet::ServerPacketHandlers& handlers, const io::ProofStorageCache& proofStorage)
    {
        handlers.registerHandler(
            ionet::PacketType::Finalization_Proof_At_Height,
            CreateFinalizationProofHandler<ProofAtHeightTraits>(proofStorage));
    }
}
}
