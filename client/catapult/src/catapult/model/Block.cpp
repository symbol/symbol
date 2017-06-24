#include "Block.h"
#include <algorithm>
#include <cstring>

namespace catapult { namespace model {

	size_t GetTransactionPayloadSize(const BlockHeader& header) {
		return header.Size - sizeof(BlockHeader);
	}

	bool IsSizeValid(const Block& block, const TransactionRegistry& registry) {
		if (block.Size < sizeof(Block)) {
			CATAPULT_LOG(warning) << block.Type << " block failed size validation with size " << block.Size;
			return false;
		}

		auto transactions = block.Transactions(EntityContainerErrorPolicy::Suppress);
		auto areAllTransactionsValid = std::all_of(transactions.cbegin(), transactions.cend(), [&registry](const auto& transaction) {
			return IsSizeValid(transaction, registry);
		});

		if (areAllTransactionsValid && !transactions.hasError())
			return true;

		CATAPULT_LOG(warning) << "block transactions failed size validation (valid sizes? " << areAllTransactionsValid
				<< ", errors? " << transactions.hasError() << ")";
		return false;
	}
}}
