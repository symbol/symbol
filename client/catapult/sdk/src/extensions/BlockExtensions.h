/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/model/Elements.h"

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace extensions {

	/// Possible results of verifying a full block.
	enum class VerifyFullBlockResult {
		/// Block is valid.
		Success,

		/// Block signature is invalid.
		Invalid_Block_Signature,

		/// Block transactions hash is invalid.
		Invalid_Block_Transactions_Hash,

		/// Transaction signature is invalid.
		Invalid_Transaction_Signature
	};

	/// Extensions for working with blocks.
	class BlockExtensions {
	public:
		/// Creates extensions for blocks containing only basic transactions for the network with the specified
		/// generation hash seed (\a generationHashSeed).
		explicit BlockExtensions(const GenerationHashSeed& generationHashSeed);

		/// Creates extensions for blocks containing transactions registered in \a transactionRegistry for the network with the specified
		/// generation hash seed (\a generationHashSeed).
		BlockExtensions(const GenerationHashSeed& generationHashSeed, const model::TransactionRegistry& transactionRegistry);

	public:
		/// Calculates and updates the block transactions hash of \a block.
		/// \note This function requires a full block and will calculate all transaction hashes.
		void updateBlockTransactionsHash(model::Block& block) const;

		/// Calculates the block transactions hash of \a block into \a blockTransactionsHash.
		/// \note This function requires a full block and will calculate all transaction hashes.
		void calculateBlockTransactionsHash(const model::Block& block, Hash256& blockTransactionsHash) const;

		/// Cryptographically signs a full \a block with \a signer.
		void signFullBlock(const crypto::KeyPair& signer, model::Block& block) const;

		/// Cryptographically verifies a full \a block by checking all signatures and hashes.
		VerifyFullBlockResult verifyFullBlock(const model::Block& block) const;

		/// Converts \a block to a block element with the specified block generation hash (\a generationHash).
		/// \note This function requires a full block and will calculate all block and transaction hashes.
		model::BlockElement convertBlockToBlockElement(const model::Block& block, const GenerationHash& generationHash) const;

	private:
		GenerationHashSeed m_generationHashSeed;
		std::function<Hash256 (const model::Transaction&)> m_calculateTransactionEntityHash;
		std::function<Hash256 (const model::Transaction&, const Hash256&)> m_calculateTransactionMerkleComponentHash;
	};
}}
