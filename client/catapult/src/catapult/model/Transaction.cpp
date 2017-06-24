#include "Transaction.h"
#include "TransactionPlugin.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace model {

	namespace {
		bool TryCalculateRealSize(const Transaction& transaction, const TransactionRegistry& registry, uint64_t& realSize) {
			const auto* pPlugin = registry.findPlugin(transaction.Type);
			if (!pPlugin) {
				CATAPULT_LOG(warning) << "rejected transaction with type: " << transaction.Type;
				return false;
			}

			realSize = pPlugin->calculateRealSize(transaction);
			return true;
		}
	}

	bool IsSizeValid(const Transaction& transaction, const TransactionRegistry& registry) {
		uint64_t realSize;
		if (!TryCalculateRealSize(transaction, registry, realSize))
			return false;

		if (transaction.Size == realSize)
			return true;

		CATAPULT_LOG(warning) << transaction.Type << " transaction failed size validation with size " << transaction.Size
				<< " (expected " << realSize << ")";
		return false;
	}
}}
