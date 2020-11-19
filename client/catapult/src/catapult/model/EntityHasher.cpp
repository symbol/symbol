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

#include "EntityHasher.h"
#include "Elements.h"
#include "TransactionPlugin.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"

namespace catapult { namespace model {

	namespace {
		RawBuffer EntityDataBuffer(const VerifiableEntity& entity, size_t totalSize) {
			auto headerSize = VerifiableEntity::Header_Size;
			return { reinterpret_cast<const uint8_t*>(&entity) + headerSize, totalSize - headerSize };
		}

		Hash256 CalculateHash(const VerifiableEntity& entity, const RawBuffer& buffer, const GenerationHashSeed* pGenerationHashSeed) {
			Hash256 entityHash;
			crypto::Sha3_256_Builder sha3;

			// add full signature and public key (this is different than Sign/Verify)
			sha3.update(entity.Signature);
			sha3.update(entity.SignerPublicKey);

			if (pGenerationHashSeed)
				sha3.update(*pGenerationHashSeed);

			sha3.update(buffer);
			sha3.final(entityHash);
			return entityHash;
		}
	}

	Hash256 CalculateHash(const Block& block) {
		return CalculateHash(block, GetBlockHeaderDataBuffer(block), nullptr);
	}

	Hash256 CalculateHash(const Transaction& transaction, const GenerationHashSeed& generationHashSeed) {
		return CalculateHash(transaction, EntityDataBuffer(transaction, transaction.Size), &generationHashSeed);
	}

	Hash256 CalculateHash(const Transaction& transaction, const GenerationHashSeed& generationHashSeed, const RawBuffer& buffer) {
		return CalculateHash(transaction, buffer, &generationHashSeed);
	}

	Hash256 CalculateMerkleComponentHash(
			const Transaction& transaction,
			const Hash256& transactionHash,
			const TransactionRegistry& transactionRegistry) {
		const auto& plugin = *transactionRegistry.findPlugin(transaction.Type);

		auto supplementaryBuffers = plugin.merkleSupplementaryBuffers(transaction);
		if (supplementaryBuffers.empty())
			return transactionHash;

		crypto::Sha3_256_Builder sha3;
		sha3.update(transactionHash);
		for (const auto& supplementaryBuffer : supplementaryBuffers)
			sha3.update(supplementaryBuffer);

		Hash256 merkleComponentHash;
		sha3.final(merkleComponentHash);
		return merkleComponentHash;
	}

	std::vector<Hash256> CalculateMerkleTree(const std::vector<TransactionElement>& transactionElements) {
		crypto::MerkleHashBuilder builder;
		for (const auto& transactionElement : transactionElements)
			builder.update(transactionElement.MerkleComponentHash);

		std::vector<Hash256> merkleTree;
		builder.final(merkleTree);
		return merkleTree;
	}

	void UpdateHashes(
			const TransactionRegistry& transactionRegistry,
			const GenerationHashSeed& generationHashSeed,
			TransactionElement& transactionElement) {
		const auto& transaction = transactionElement.Transaction;
		const auto& plugin = *transactionRegistry.findPlugin(transaction.Type);

		transactionElement.EntityHash = CalculateHash(transaction, generationHashSeed, plugin.dataBuffer(transaction));
		transactionElement.MerkleComponentHash = CalculateMerkleComponentHash(
				transaction,
				transactionElement.EntityHash,
				transactionRegistry);
	}
}}
