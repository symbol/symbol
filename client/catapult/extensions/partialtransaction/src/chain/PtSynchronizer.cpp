#include "PtSynchronizer.h"
#include "partialtransaction/src/api/RemotePtApi.h"
#include "catapult/chain/EntitiesSynchronizer.h"

namespace catapult { namespace chain {

	namespace {
		struct PtTraits {
		public:
			using RemoteApiType = api::RemotePtApi;
			static constexpr auto Name = "partial transactions";

		public:
			explicit PtTraits(
					const partialtransaction::ShortHashPairsSupplier& shortHashPairsSupplier,
					const partialtransaction::CosignedTransactionInfosConsumer& transactionInfosConsumer)
					: m_shortHashPairsSupplier(shortHashPairsSupplier)
					, m_transactionInfosConsumer(transactionInfosConsumer)
			{}

		public:
			thread::future<partialtransaction::CosignedTransactionInfos> apiCall(const RemoteApiType& api) const {
				return api.transactionInfos(m_shortHashPairsSupplier());
			}

			void consume(partialtransaction::CosignedTransactionInfos&& transactionInfos) const {
				m_transactionInfosConsumer(std::move(transactionInfos));
			}

		private:
			partialtransaction::ShortHashPairsSupplier m_shortHashPairsSupplier;
			partialtransaction::CosignedTransactionInfosConsumer m_transactionInfosConsumer;
		};
	}

	RemoteNodeSynchronizer<api::RemotePtApi> CreatePtSynchronizer(
			const partialtransaction::ShortHashPairsSupplier& shortHashPairsSupplier,
			const partialtransaction::CosignedTransactionInfosConsumer& transactionInfosConsumer) {
		auto traits = PtTraits(shortHashPairsSupplier, transactionInfosConsumer);
		auto pSynchronizer = std::make_shared<EntitiesSynchronizer<PtTraits>>(std::move(traits));
		return CreateRemoteNodeSynchronizer(pSynchronizer);
	}
}}
