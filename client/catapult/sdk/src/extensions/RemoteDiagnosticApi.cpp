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

#include "RemoteDiagnosticApi.h"
#include "catapult/api/RemoteRequestDispatcher.h"
#include "catapult/handlers/DiagnosticHandlers.h"
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/ionet/PacketIo.h"
#include "catapult/ionet/PacketPayloadFactory.h"

namespace catapult { namespace extensions {

	namespace {
		// region traits

		struct ConfirmTimestampedHashesTraits {
		public:
			using ResultType = state::TimestampedHashRange;
			static constexpr auto Packet_Type = ionet::PacketType::Confirm_Timestamped_Hashes;
			static constexpr auto Friendly_Name = "confirm timestamped hashes";

			static auto CreateRequestPacketPayload(state::TimestampedHashRange&& timestampedHashes) {
				return ionet::PacketPayloadFactory::FromFixedSizeRange(Packet_Type, std::move(timestampedHashes));
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractFixedSizeStructuresFromPacket<state::TimestampedHash>(packet);
				return !result.empty() || sizeof(ionet::PacketHeader) == packet.Size;
			}
		};

		struct DiagnosticCountersTraits {
		public:
			using ResultType = model::EntityRange<model::DiagnosticCounterValue>;
			static constexpr auto Packet_Type = ionet::PacketType::Diagnostic_Counters;
			static constexpr auto Friendly_Name = "diagnostic counters";

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(Packet_Type);
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractFixedSizeStructuresFromPacket<model::DiagnosticCounterValue>(packet);
				return !result.empty();
			}
		};

		struct ActiveNodeInfosTraits {
		public:
			using ResultType = model::EntityRange<ionet::PackedNodeInfo>;
			static constexpr auto Packet_Type = ionet::PacketType::Active_Node_Infos;
			static constexpr auto Friendly_Name = "active node infos";

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(Packet_Type);
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractEntitiesFromPacket<ionet::PackedNodeInfo>(packet, model::IsSizeValidT<ionet::PackedNodeInfo>);
				return !result.empty();
			}
		};

		struct UnlockedAccountsTraits {
		public:
			using ResultType = model::EntityRange<Key>;
			static constexpr auto Packet_Type = ionet::PacketType::Unlocked_Accounts;
			static constexpr auto Friendly_Name = "unlocked accounts";

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(Packet_Type);
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractFixedSizeStructuresFromPacket<Key>(packet);
				return !result.empty();
			}
		};

		template<typename TIdentifier, ionet::PacketType PacketType>
		struct InfosTraits {
		public:
			using ResultType = model::EntityRange<model::CacheEntryInfo<TIdentifier>>;
			static constexpr ionet::PacketType Packet_Type = PacketType;

			static auto CreateRequestPacketPayload(model::EntityRange<TIdentifier>&& ids) {
				return ionet::PacketPayloadFactory::FromFixedSizeRange(Packet_Type, std::move(ids));
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractEntitiesFromPacket<model::CacheEntryInfo<TIdentifier>>(
						packet,
						model::IsSizeValidT<model::CacheEntryInfo<TIdentifier>>);
				return !result.empty();
			}
		};

		struct AccountInfosTraits : public InfosTraits<Address, ionet::PacketType::Account_Infos> {
			static constexpr auto Friendly_Name = "account infos";
		};

		struct AccountRestrictionsInfosTraits : public InfosTraits<Address, ionet::PacketType::Account_Restrictions_Infos> {
			static constexpr auto Friendly_Name = "account restrictions infos";
		};

		struct NamespaceInfosTraits : public InfosTraits<NamespaceId, ionet::PacketType::Namespace_Infos> {
			static constexpr auto Friendly_Name = "namespace infos";
		};

		struct MosaicInfosTraits : public InfosTraits<MosaicId, ionet::PacketType::Mosaic_Infos> {
			static constexpr auto Friendly_Name = "mosaic infos";
		};

		struct MetadataInfosTraits : public InfosTraits<Hash256, ionet::PacketType::Metadata_Infos> {
			static constexpr auto Friendly_Name = "metadata infos";
		};

		// endregion

		class DefaultRemoteDiagnosticApi : public RemoteDiagnosticApi {
		private:
			template<typename TTraits>
			using FutureType = thread::future<typename TTraits::ResultType>;

		public:
			explicit DefaultRemoteDiagnosticApi(ionet::PacketIo& io) : m_impl(io)
			{}

		public:
			FutureType<ConfirmTimestampedHashesTraits> confirmTimestampedHashes(
					state::TimestampedHashRange&& timestampedHashes) const override {
				return m_impl.dispatch(ConfirmTimestampedHashesTraits(), std::move(timestampedHashes));
			}

			FutureType<DiagnosticCountersTraits> diagnosticCounters() const override {
				return m_impl.dispatch(DiagnosticCountersTraits());
			}

			FutureType<ActiveNodeInfosTraits> activeNodeInfos() const override {
				return m_impl.dispatch(ActiveNodeInfosTraits());
			}

			FutureType<UnlockedAccountsTraits> unlockedAccounts() const override {
				return m_impl.dispatch(UnlockedAccountsTraits());
			}

			FutureType<AccountInfosTraits> accountInfos(model::AddressRange&& addresses) const override {
				return m_impl.dispatch(AccountInfosTraits(), std::move(addresses));
			}

			FutureType<AccountRestrictionsInfosTraits> accountRestrictionsInfos(model::AddressRange&& addresses) const override {
				return m_impl.dispatch(AccountRestrictionsInfosTraits(), std::move(addresses));
			}

			FutureType<NamespaceInfosTraits> namespaceInfos(model::EntityRange<NamespaceId>&& namespaceIds) const override {
				return m_impl.dispatch(NamespaceInfosTraits(), std::move(namespaceIds));
			}

			FutureType<MosaicInfosTraits> mosaicInfos(model::EntityRange<MosaicId>&& mosaicIds) const override {
				return m_impl.dispatch(MosaicInfosTraits(), std::move(mosaicIds));
			}

			FutureType<MetadataInfosTraits> metadataInfos(model::EntityRange<Hash256>&& uniqueKeys) const override {
				return m_impl.dispatch(MetadataInfosTraits(), std::move(uniqueKeys));
			}

		private:
			mutable api::RemoteRequestDispatcher m_impl;
		};
	}

	std::unique_ptr<RemoteDiagnosticApi> CreateRemoteDiagnosticApi(ionet::PacketIo& io) {
		return std::make_unique<DefaultRemoteDiagnosticApi>(io);
	}
}}
