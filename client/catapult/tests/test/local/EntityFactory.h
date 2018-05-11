/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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
	std::unique_ptr<model::Transaction> CreateTransferTransaction(const crypto::KeyPair& signer, const Address& recipient, Amount amount);

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
