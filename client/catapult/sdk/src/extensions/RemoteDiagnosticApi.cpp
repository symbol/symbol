#include "RemoteDiagnosticApi.h"
#include "catapult/api/RemoteRequestDispatcher.h"
#include "catapult/handlers/DiagnosticHandlers.h"
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/ionet/PacketIo.h"

namespace catapult { namespace extensions {

	namespace {
		// region traits

		struct AccountInfosTraits {
		public:
			using ResultType = model::AccountInfoRange;
			static constexpr auto PacketType() { return ionet::PacketType::Account_Infos; }
			static constexpr auto FriendlyName() { return "account infos"; }

			static auto CreateRequestPacketPayload(model::AddressRange&& addresses) {
				return ionet::PacketPayload::FromFixedSizeRange(PacketType(), std::move(addresses));
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractEntitiesFromPacket<model::AccountInfo>(packet, ionet::IsSizeValid<model::AccountInfo>);
				return !result.empty();
			}
		};

		struct ConfirmTimestampedHashesTraits {
		public:
			using ResultType = state::TimestampedHashRange;
			static constexpr auto PacketType() { return ionet::PacketType::Confirm_Timestamped_Hashes; }
			static constexpr auto FriendlyName() { return "confirm timestamped hashes"; }

			static auto CreateRequestPacketPayload(state::TimestampedHashRange&& timestampedHashes) {
				return ionet::PacketPayload::FromFixedSizeRange(PacketType(), std::move(timestampedHashes));
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
			static constexpr auto PacketType() { return ionet::PacketType::Diagnostic_Counters; }
			static constexpr auto FriendlyName() { return "diagnostic counters"; }

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(PacketType());
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
			static constexpr auto PacketType() { return ionet::PacketType::Active_Node_Infos; }
			static constexpr auto FriendlyName() { return "active node infos"; }

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(PacketType());
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractEntitiesFromPacket<ionet::PackedNodeInfo>(packet, ionet::IsSizeValid<ionet::PackedNodeInfo>);
				return !result.empty();
			}
		};

		struct NamespaceInfosTraits {
		public:
			using ResultType = model::EntityRange<model::NamespaceInfo>;
			static constexpr auto PacketType() { return ionet::PacketType::Namespace_Infos; }
			static constexpr auto FriendlyName() { return "namespace infos"; }

			static auto CreateRequestPacketPayload(model::EntityRange<NamespaceId>&& namespaceIds) {
				return ionet::PacketPayload::FromFixedSizeRange(PacketType(), std::move(namespaceIds));
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractEntitiesFromPacket<model::NamespaceInfo>(packet, ionet::IsSizeValid<model::NamespaceInfo>);
				return !result.empty();
			}
		};

		struct MosaicInfosTraits {
		public:
			using ResultType = model::EntityRange<model::MosaicInfo>;
			static constexpr auto PacketType() { return ionet::PacketType::Mosaic_Infos; }
			static constexpr auto FriendlyName() { return "mosaic infos"; }

			static auto CreateRequestPacketPayload(model::EntityRange<MosaicId>&& mosaicIds) {
				return ionet::PacketPayload::FromFixedSizeRange(PacketType(), std::move(mosaicIds));
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractFixedSizeStructuresFromPacket<model::MosaicInfo>(packet);
				return !result.empty();
			}
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
			FutureType<AccountInfosTraits> accountInfos(model::AddressRange&& addresses) const override {
				return m_impl.dispatch(AccountInfosTraits(), std::move(addresses));
			}

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

			FutureType<NamespaceInfosTraits> namespaceInfos(model::EntityRange<NamespaceId>&& namespaceIds) const override {
				return m_impl.dispatch(NamespaceInfosTraits(), std::move(namespaceIds));
			}

			FutureType<MosaicInfosTraits> mosaicInfos(model::EntityRange<MosaicId>&& mosaicIds) const override {
				return m_impl.dispatch(MosaicInfosTraits(), std::move(mosaicIds));
			}

		private:
			mutable api::RemoteRequestDispatcher m_impl;
		};
	}

	std::unique_ptr<RemoteDiagnosticApi> CreateRemoteDiagnosticApi(ionet::PacketIo& io) {
		return std::make_unique<DefaultRemoteDiagnosticApi>(io);
	}
}}
