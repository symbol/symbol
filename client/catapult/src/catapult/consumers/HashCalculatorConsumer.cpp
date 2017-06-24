#include "BlockConsumers.h"
#include "ConsumerResultFactory.h"
#include "TransactionConsumers.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/model/EntityHasher.h"

namespace catapult { namespace consumers {

	namespace {
		class BlockHashCalculatorConsumer {
		public:
			explicit BlockHashCalculatorConsumer(const model::TransactionRegistry& transactionRegistry)
					: m_transactionRegistry(transactionRegistry)
			{}

		public:
			ConsumerResult operator()(BlockElements& elements) const {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				for (auto& element : elements) {
					// note that disruptor input elements have been extracted from a packet (or created within this
					// process), so their sizes have already been validated
					crypto::MerkleHashBuilder transactionsHashBuilder;
					for (const auto& transaction : element.Block.Transactions()) {
						model::TransactionElement txElement(transaction);
						model::UpdateHashes(m_transactionRegistry, txElement);
						element.Transactions.push_back(txElement);

						transactionsHashBuilder.update(txElement.MerkleComponentHash);
					}

					Hash256 transactionsHash;
					transactionsHashBuilder.final(transactionsHash);
					if (element.Block.BlockTransactionsHash != transactionsHash)
						return Abort(Failure_Consumer_Block_Transactions_Hash_Mismatch);

					element.EntityHash = model::CalculateHash(element.Block);
				}

				return Continue();
			}

		private:
			const model::TransactionRegistry& m_transactionRegistry;
		};
	}

	disruptor::BlockConsumer CreateBlockHashCalculatorConsumer(const model::TransactionRegistry& transactionRegistry) {
		return BlockHashCalculatorConsumer(transactionRegistry);
	}

	namespace {
		class TransactionHashCalculatorConsumer {
		public:
			explicit TransactionHashCalculatorConsumer(const model::TransactionRegistry& transactionRegistry)
					: m_transactionRegistry(transactionRegistry)
			{}

		public:
			ConsumerResult operator()(TransactionElements& elements) const {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				for (auto& element : elements)
					model::UpdateHashes(m_transactionRegistry, element);

				return Continue();
			}

		private:
			const model::TransactionRegistry& m_transactionRegistry;
		};
	}

	disruptor::TransactionConsumer CreateTransactionHashCalculatorConsumer(const model::TransactionRegistry& transactionRegistry) {
		return TransactionHashCalculatorConsumer(transactionRegistry);
	}
}}
