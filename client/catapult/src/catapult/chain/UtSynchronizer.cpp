#include "UtSynchronizer.h"
#include "EntitiesSynchronizer.h"
#include "catapult/api/RemoteTransactionApi.h"

namespace catapult { namespace chain {

	namespace {
		struct UtTraits {
		public:
			using RemoteApiType = api::RemoteTransactionApi;
			static constexpr auto Name = "unconfirmed transactions";

		public:
			explicit UtTraits(
					const ShortHashesSupplier& shortHashesSupplier,
					const handlers::TransactionRangeHandler& transactionRangeConsumer)
					: m_shortHashesSupplier(shortHashesSupplier)
					, m_transactionRangeConsumer(transactionRangeConsumer)
			{}

		public:
			thread::future<model::TransactionRange> apiCall(const RemoteApiType& api) const {
				return api.unconfirmedTransactions(m_shortHashesSupplier());
			}

			void consume(model::TransactionRange&& range) const {
				m_transactionRangeConsumer(std::move(range));
			}

		private:
			ShortHashesSupplier m_shortHashesSupplier;
			handlers::TransactionRangeHandler m_transactionRangeConsumer;
		};
	}

	RemoteNodeSynchronizer<api::RemoteTransactionApi> CreateUtSynchronizer(
			const ShortHashesSupplier& shortHashesSupplier,
			const handlers::TransactionRangeHandler& transactionRangeConsumer) {
		auto traits = UtTraits(shortHashesSupplier, transactionRangeConsumer);
		auto pSynchronizer = std::make_shared<EntitiesSynchronizer<UtTraits>>(std::move(traits));
		return CreateRemoteNodeSynchronizer(pSynchronizer);
	}
}}
