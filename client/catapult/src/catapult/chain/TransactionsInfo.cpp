#include "TransactionsInfo.h"
#include "catapult/cache/MemoryUtCache.h"

namespace catapult { namespace chain {

	TransactionsInfoSupplier CreateTransactionsInfoSupplier(const cache::MemoryUtCache& unconfirmedTransactionsCache) {
		return [&unconfirmedTransactionsCache](auto count) {
			TransactionsInfo info;
			std::vector<const model::TransactionInfo*> transactionInfos;

			auto view = unconfirmedTransactionsCache.view();
			if (0 != count) {
				view.forEach([count, &info, &transactionInfos](const auto& txInfo) {
					info.Transactions.push_back(txInfo.pEntity);
					transactionInfos.push_back(&txInfo);
					return info.Transactions.size() != count;
				});
			}

			CalculateBlockTransactionsHash(transactionInfos, info.TransactionsHash);
			return info;
		};
	}
}}
