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
			static constexpr auto PacketType() { return ionet::PacketType::Pull_Transactions; }
			static constexpr auto FriendlyName() { return "pull unconfirmed transactions"; }

			static auto CreateRequestPacketPayload(model::ShortHashRange&& knownShortHashes) {
				return ionet::PacketPayloadFactory::FromFixedSizeRange(PacketType(), std::move(knownShortHashes));
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
			explicit DefaultRemoteTransactionApi(ionet::PacketIo& io, const model::TransactionRegistry& registry)
					: m_registry(registry)
					, m_impl(io)
			{}

		public:
			FutureType<UtTraits> unconfirmedTransactions(model::ShortHashRange&& knownShortHashes) const override {
				return m_impl.dispatch(UtTraits(m_registry), std::move(knownShortHashes));
			}

		private:
			const model::TransactionRegistry& m_registry;
			mutable RemoteRequestDispatcher m_impl;
		};
	}

	std::unique_ptr<RemoteTransactionApi> CreateRemoteTransactionApi(ionet::PacketIo& io, const model::TransactionRegistry& registry) {
		return std::make_unique<DefaultRemoteTransactionApi>(io, registry);
	}
}}
