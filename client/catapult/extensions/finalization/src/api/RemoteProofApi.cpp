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

#include "RemoteProofApi.h"
#include "FinalizationPackets.h"
#include "catapult/api/RemoteRequestDispatcher.h"
#include "catapult/ionet/PacketEntityUtils.h"

namespace catapult { namespace api {

	namespace {
		// region traits

		struct FinalizationStatisticsTraits {
		public:
			using ResultType = model::FinalizationStatistics;
			static constexpr auto Packet_Type = ionet::PacketType::Finalization_Statistics;
			static constexpr auto Friendly_Name = "finalization statistics";

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(Packet_Type);
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				const auto* pResponse = ionet::CoercePacket<FinalizationStatisticsResponse>(&packet);
				if (!pResponse)
					return false;

				result.Point = pResponse->Point;
				result.Height = pResponse->Height;
				result.Hash = pResponse->Hash;
				return true;
			}
		};

		struct BasicProofAtTraits {
		public:
			using ResultType = std::shared_ptr<const model::FinalizationProof>;
			static constexpr auto Packet_Type = ionet::PacketType::Pull_Finalization_Proof;

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractEntityFromPacket<model::FinalizationProof>(packet, model::IsSizeValid);
				return !!result;
			}
		};

		struct ProofAtPointTraits : public BasicProofAtTraits {
		public:
			static constexpr auto Friendly_Name = "proof at point";

			static auto CreateRequestPacketPayload(FinalizationPoint point) {
				auto pPacket = ionet::CreateSharedPacket<ProofAtPointRequest>();
				pPacket->Point = point;
				return ionet::PacketPayload(pPacket);
			}
		};

		struct ProofAtHeightTraits : public BasicProofAtTraits {
		public:
			static constexpr auto Friendly_Name = "proof at height";

			static auto CreateRequestPacketPayload(Height height) {
				auto pPacket = ionet::CreateSharedPacket<ProofAtHeightRequest>();
				pPacket->Height = height;
				return ionet::PacketPayload(pPacket);
			}
		};

		// endregion

		class DefaultRemoteProofApi : public RemoteProofApi {
		private:
			template<typename TTraits>
			using FutureType = thread::future<typename TTraits::ResultType>;

		public:
			DefaultRemoteProofApi(ionet::PacketIo& io, const model::NodeIdentity& remoteIdentity)
					: RemoteProofApi(remoteIdentity)
					, m_impl(io)
			{}

		public:
			FutureType<FinalizationStatisticsTraits> finalizationStatistics() const override {
				return m_impl.dispatch(FinalizationStatisticsTraits());
			}

			FutureType<ProofAtPointTraits> proofAt(FinalizationPoint point) const override {
				return m_impl.dispatch(ProofAtPointTraits(), point);
			}

			FutureType<ProofAtHeightTraits> proofAt(Height height) const override {
				return m_impl.dispatch(ProofAtHeightTraits(), height);
			}

		private:
			mutable RemoteRequestDispatcher m_impl;
		};
	}

	std::unique_ptr<RemoteProofApi> CreateRemoteProofApi(ionet::PacketIo& io, const model::NodeIdentity& remoteIdentity) {
		return std::make_unique<DefaultRemoteProofApi>(io, remoteIdentity);
	}
}}
