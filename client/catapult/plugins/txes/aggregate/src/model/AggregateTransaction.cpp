#include "AggregateTransaction.h"

namespace catapult { namespace model {

	size_t GetTransactionPayloadSize(const AggregateTransactionHeader& header) {
		return header.PayloadSize;
	}

	namespace {
		constexpr bool IsPayloadSizeValid(const AggregateTransaction& aggregate) {
			return
					aggregate.Size >= sizeof(AggregateTransaction) &&
					aggregate.Size - sizeof(AggregateTransaction) >= aggregate.PayloadSize &&
					0 == (aggregate.Size - sizeof(AggregateTransaction) - aggregate.PayloadSize) % sizeof(Cosignature);
		}
	}

	bool IsSizeValid(const AggregateTransaction& aggregate, const TransactionRegistry& registry) {
		if (!IsPayloadSizeValid(aggregate)) {
			CATAPULT_LOG(warning) << "aggregate transaction failed size validation with size "
					<< aggregate.Size << " and payload size " << aggregate.PayloadSize;
			return false;
		}

		auto transactions = aggregate.Transactions(EntityContainerErrorPolicy::Suppress);
		auto areAllTransactionsValid = std::all_of(transactions.cbegin(), transactions.cend(), [&registry](const auto& transaction) {
			return IsSizeValid(transaction, registry);
		});

		if (areAllTransactionsValid && !transactions.hasError())
			return true;

		CATAPULT_LOG(warning) << "aggregate transactions failed size validation (valid sizes? " << areAllTransactionsValid
				<< ", errors? " << transactions.hasError() << ")";
		return false;
	}
}}
