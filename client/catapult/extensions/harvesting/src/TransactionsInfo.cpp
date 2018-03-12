#include "TransactionsInfo.h"
#include "catapult/cache/MemoryUtCache.h"

namespace catapult { namespace harvesting {

	TransactionsInfoSupplier CreateTransactionsInfoSupplier(const cache::MemoryUtCache& utCache) {
		return [&utCache](auto count) {
			TransactionsInfo info;
			std::vector<const model::TransactionInfo*> transactionInfos;

			auto view = utCache.view();
			if (0 != count) {
				view.forEach([count, &info, &transactionInfos](const auto& transactionInfo) {
					info.Transactions.push_back(transactionInfo.pEntity);
					transactionInfos.push_back(&transactionInfo);
					return info.Transactions.size() != count;
				});
			}

			CalculateBlockTransactionsHash(transactionInfos, info.TransactionsHash);
			return info;
		};
	}
}}
