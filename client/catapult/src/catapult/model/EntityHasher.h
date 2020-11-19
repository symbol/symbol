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
#include "Block.h"

namespace catapult {
	namespace model {
		struct TransactionElement;
		class TransactionRegistry;
	}
}

namespace catapult { namespace model {

	/// Calculates the hash for the given \a block header.
	Hash256 CalculateHash(const Block& block);

	/// Calculates the hash for the given \a transaction for the network with the specified generation hash seed (\a generationHashSeed).
	Hash256 CalculateHash(const Transaction& transaction, const GenerationHashSeed& generationHashSeed);

	/// Calculates the hash for the given \a transaction with data \a buffer for the network with the specified
	/// generation hash seed (\a generationHashSeed).
	Hash256 CalculateHash(const Transaction& transaction, const GenerationHashSeed& generationHashSeed, const RawBuffer& buffer);

	/// Calculates the merkle component hash for the given \a transaction with \a transactionHash
	/// using transaction information from \a transactionRegistry.
	Hash256 CalculateMerkleComponentHash(
			const Transaction& transaction,
			const Hash256& transactionHash,
			const TransactionRegistry& transactionRegistry);

	/// Calculates the merkle tree from \a transactionElements.
	std::vector<Hash256> CalculateMerkleTree(const std::vector<TransactionElement>& transactionElements);

	/// Calculates the hashes for \a transactionElement in place for the network with the specified
	/// generation hash seed (\a generationHashSeed) using transaction information from \a transactionRegistry.
	void UpdateHashes(
				const TransactionRegistry& transactionRegistry,
				const GenerationHashSeed& generationHashSeed,
				TransactionElement& transactionElement);
}}
