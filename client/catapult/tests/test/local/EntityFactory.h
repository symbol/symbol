#pragma once
#include "catapult/types.h"
#include <memory>
#include <vector>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace model {
		struct Block;
		struct Mosaic;
		struct Transaction;
	}
}

namespace catapult { namespace test {

	// notice that these helper functions create TransferTransaction
	// they are in local test utils because non-local tests should be using MockTransaction

	/// Creates basic unsigned TransferTransaction with specified \a signerPublicKey, \a recipient, \a message and \a mosaics.
	std::unique_ptr<model::Transaction> CreateUnsignedTransferTransaction(
			const Key& signerPublicKey,
			const Address& recipient,
			const std::vector<uint8_t>& message,
			const std::vector<model::Mosaic>& mosaics);

	/// Creates basic unsigned TransferTransaction with specified \a signerPublicKey, \a recipient and \a amount.
	std::unique_ptr<model::Transaction> CreateUnsignedTransferTransaction(
			const Key& signerPublicKey,
			const Address& recipient,
			Amount amount);

	/// Creates basic signed TransferTransaction with \a signer, \a recipient and \a amount.
	std::unique_ptr<model::Transaction> CreateTransferTransaction(
			const crypto::KeyPair& signer,
			const Address& recipient,
			Amount amount);

	/// Creates basic signed TransferTransaction with \a signer, \a recipientPublicKey and \a amount.
	std::unique_ptr<model::Transaction> CreateTransferTransaction(
			const crypto::KeyPair& signer,
			const Key& recipientPublicKey,
			Amount amount);

	/// Generates a TransferTransaction with random data.
	std::unique_ptr<model::Transaction> GenerateRandomTransferTransaction();

	/// Generates a block with \a numTransactions TransferTransactions at \a height.
	std::unique_ptr<model::Block> GenerateBlockWithTransferTransactionsAtHeight(size_t numTransactions, Height height);
}}
