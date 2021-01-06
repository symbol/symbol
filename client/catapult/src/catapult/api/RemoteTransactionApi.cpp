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

#include "RemoteTransactionApi.h"
#include "RemoteApiUtils.h"
#include "RemoteRequestDispatcher.h"
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/ionet/PacketPayloadFactory.h"

namespace catapult { namespace api {

	namespace {
		// region traits

		struct UtTraits : public RegistryDependentTraits<model::Transaction> {
		public:
			using ResultType = model::TransactionRange;
			static constexpr auto Packet_Type = ionet::PacketType::Pull_Transactions;
			static constexpr auto Friendly_Name = "pull unconfirmed transactions";

			static auto CreateRequestPacketPayload(
					Timestamp minDeadline,
					BlockFeeMultiplier minFeeMultiplier,
					model::ShortHashRange&& knownShortHashes) {
				ionet::PacketPayloadBuilder builder(Packet_Type);
				builder.appendValue(minDeadline);
				builder.appendValue(minFeeMultiplier);
				builder.appendRange(std::move(knownShortHashes));
				return builder.build();
			}

		public:
			using RegistryDependentTraits::RegistryDependentTraits;

			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractEntitiesFromPacket<model::Transaction>(packet, *this);
				return !result.empty() || sizeof(ionet::PacketHeader) == packet.Size;
			}
		};

		// endregion

		class DefaultRemoteTransactionApi : public RemoteTransactionApi {
		private:
			template<typename TTraits>
			using FutureType = thread::future<typename TTraits::ResultType>;

		public:
			DefaultRemoteTransactionApi(
					ionet::PacketIo& io,
					const model::NodeIdentity& remoteIdentity,
					const model::TransactionRegistry& registry)
					: RemoteTransactionApi(remoteIdentity)
					, m_registry(registry)
					, m_impl(io)
			{}

		public:
			FutureType<UtTraits> unconfirmedTransactions(
					Timestamp minDeadline,
					BlockFeeMultiplier minFeeMultiplier,
					model::ShortHashRange&& knownShortHashes) const override {
				return m_impl.dispatch(UtTraits(m_registry), minDeadline, minFeeMultiplier, std::move(knownShortHashes));
			}

		private:
			const model::TransactionRegistry& m_registry;
			mutable RemoteRequestDispatcher m_impl;
		};
	}

	std::unique_ptr<RemoteTransactionApi> CreateRemoteTransactionApi(
			ionet::PacketIo& io,
			const model::NodeIdentity& remoteIdentity,
			const model::TransactionRegistry& registry) {
		return std::make_unique<DefaultRemoteTransactionApi>(io, remoteIdentity, registry);
	}
}}
