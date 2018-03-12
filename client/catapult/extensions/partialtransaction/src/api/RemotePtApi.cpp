#include "RemotePtApi.h"
#include "CosignedTransactionInfoParser.h"
#include "catapult/api/RemoteApiUtils.h"
#include "catapult/api/RemoteRequestDispatcher.h"

namespace catapult { namespace api {

	namespace {
		// region traits

		struct TransactionInfosTraits : public RegistryDependentTraits<model::Transaction> {
		public:
			using ResultType = partialtransaction::CosignedTransactionInfos;
			static constexpr auto PacketType() { return ionet::PacketType::Pull_Partial_Transaction_Infos; }
			static constexpr auto FriendlyName() { return "pull partial transaction infos"; }

			static auto CreateRequestPacketPayload(cache::ShortHashPairRange&& knownShortHashPairs) {
				return ionet::PacketPayload::FromFixedSizeRange(PacketType(), std::move(knownShortHashPairs));
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
			explicit DefaultRemotePtApi(ionet::PacketIo& io, const model::TransactionRegistry& registry)
					: m_registry(registry)
					, m_impl(io)
			{}

		public:
			FutureType<TransactionInfosTraits> transactionInfos(cache::ShortHashPairRange&& knownShortHashPairs) const override {
				return m_impl.dispatch(TransactionInfosTraits(m_registry), std::move(knownShortHashPairs));
			}

		private:
			const model::TransactionRegistry& m_registry;
			mutable RemoteRequestDispatcher m_impl;
		};
	}

	std::unique_ptr<RemotePtApi> CreateRemotePtApi(ionet::PacketIo& io, const model::TransactionRegistry& registry) {
		return std::make_unique<DefaultRemotePtApi>(io, registry);
	}
}}
