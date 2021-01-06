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

#include "RemotePtApi.h"
#include "CosignedTransactionInfoParser.h"
#include "catapult/api/RemoteApiUtils.h"
#include "catapult/api/RemoteRequestDispatcher.h"
#include "catapult/ionet/PacketPayloadFactory.h"

namespace catapult { namespace api {

	namespace {
		// region traits

		struct TransactionInfosTraits : public RegistryDependentTraits<model::Transaction> {
		public:
			using ResultType = partialtransaction::CosignedTransactionInfos;
			static constexpr auto Packet_Type = ionet::PacketType::Pull_Partial_Transaction_Infos;
			static constexpr auto Friendly_Name = "pull partial transaction infos";

			static auto CreateRequestPacketPayload(Timestamp minDeadline, cache::ShortHashPairRange&& knownShortHashPairs) {
				ionet::PacketPayloadBuilder builder(Packet_Type);
				builder.appendValue(minDeadline);
				builder.appendRange(std::move(knownShortHashPairs));
				return builder.build();
			}

		public:
			using RegistryDependentTraits::RegistryDependentTraits;

			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ExtractCosignedTransactionInfosFromPacket(packet, *this);
				return !result.empty() || sizeof(ionet::PacketHeader) == packet.Size;
			}
		};

		// endregion

		class DefaultRemotePtApi : public RemotePtApi {
		private:
			template<typename TTraits>
			using FutureType = thread::future<typename TTraits::ResultType>;

		public:
			DefaultRemotePtApi(ionet::PacketIo& io, const model::NodeIdentity& remoteIdentity, const model::TransactionRegistry& registry)
					: RemotePtApi(remoteIdentity)
					, m_registry(registry)
					, m_impl(io)
			{}

		public:
			FutureType<TransactionInfosTraits> transactionInfos(
					Timestamp minDeadline,
					cache::ShortHashPairRange&& knownShortHashPairs) const override {
				return m_impl.dispatch(TransactionInfosTraits(m_registry), minDeadline, std::move(knownShortHashPairs));
			}

		private:
			const model::TransactionRegistry& m_registry;
			mutable RemoteRequestDispatcher m_impl;
		};
	}

	std::unique_ptr<RemotePtApi> CreateRemotePtApi(
			ionet::PacketIo& io,
			const model::NodeIdentity& remoteIdentity,
			const model::TransactionRegistry& registry) {
		return std::make_unique<DefaultRemotePtApi>(io, remoteIdentity, registry);
	}
}}
