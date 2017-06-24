#include "RemoteTransactionApi.h"
#include "RemoteApiUtils.h"
#include "RemoteRequestDispatcher.h"
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/ionet/PacketIo.h"

namespace catapult { namespace api {

	namespace {
		// region traits

		struct UnconfirmedTransactionsTraits : public RegistryDependentTraits<model::Transaction> {
		public:
			using ResultType = model::TransactionRange;
			static constexpr auto PacketType() { return ionet::PacketType::Pull_Transactions; }
			static constexpr auto FriendlyName() { return "pull unconfirmed transactions"; }

			static auto CreateRequestPacketPayload(model::ShortHashRange&& knownShortHashes) {
				return ionet::PacketPayload::FromFixedSizeRange(PacketType(), std::move(knownShortHashes));
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
			explicit DefaultRemoteTransactionApi(
					const std::shared_ptr<ionet::PacketIo>& pIo,
					const std::shared_ptr<const model::TransactionRegistry>& pRegistry)
					: m_pRegistry(pRegistry)
					, m_impl(pIo)
			{}

		public:
			FutureType<UnconfirmedTransactionsTraits> unconfirmedTransactions(
					model::ShortHashRange&& knownShortHashes) const override {
				return m_impl.dispatch(UnconfirmedTransactionsTraits(m_pRegistry), std::move(knownShortHashes));
			}

		private:
			std::shared_ptr<const model::TransactionRegistry> m_pRegistry;
			mutable RemoteRequestDispatcher m_impl;
		};
	}

	std::unique_ptr<RemoteTransactionApi> CreateRemoteTransactionApi(
			const std::shared_ptr<ionet::PacketIo>& pIo,
			const std::shared_ptr<const model::TransactionRegistry>& pRegistry) {
		return std::make_unique<DefaultRemoteTransactionApi>(pIo, pRegistry);
	}
}}
