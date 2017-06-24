#include "TransactionConsumers.h"
#include "ConsumerResultFactory.h"

namespace catapult { namespace consumers {

	namespace {
		class NewTransactionsConsumer {
		public:
			explicit NewTransactionsConsumer(const NewTransactionsSink& newTransactionsSink)
					: m_newTransactionsSink(newTransactionsSink)
			{}

		public:
			ConsumerResult operator()(disruptor::ConsumerInput& input) const {
				if (input.empty())
					return Abort(Failure_Consumer_Empty_Input);

				// 1. split up the input into its component transactions
				//    - detachTransactionRange transfers ownership of the range from the input
				//      but doesn't invalidate the input elements
				//    - the range is moved into ExtractEntitiesFromRange, which extends the lifetime of the range
				//      to the lifetime of the returned transactions
				auto transactions = model::TransactionRange::ExtractEntitiesFromRange(input.detachTransactionRange());

				// 2. prepare the output
				TransactionInfos transactionInfos;
				transactionInfos.reserve(transactions.size());

				// 3. filter transactions
				//    - the input elements are still valid even though the backing range has been detached
				auto i = 0u;
				for (const auto& element : input.transactions()) {
					if (!element.Skip)
						transactionInfos.emplace_back(transactions[i], element.EntityHash, element.MerkleComponentHash);

					++i;
				}

				// 4. call the sink
				m_newTransactionsSink(std::move(transactionInfos));

				// 5. indicate input was consumed and processing is complete
				return Complete();
			}

		private:
			NewTransactionsSink m_newTransactionsSink;
		};
	}

	disruptor::DisruptorConsumer CreateNewTransactionsConsumer(const NewTransactionsSink& newTransactionsSink) {
		return NewTransactionsConsumer(newTransactionsSink);
	}
}}
