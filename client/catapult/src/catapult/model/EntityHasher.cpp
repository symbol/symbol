#include "EntityHasher.h"
#include "Elements.h"
#include "TransactionPlugin.h"
#include "catapult/crypto/Hashes.h"

namespace catapult { namespace model {

	namespace {
		RawBuffer EntityDataBuffer(const VerifiableEntity& entity, size_t totalSize) {
			auto headerSize = VerifiableEntity::Header_Size;
			return { reinterpret_cast<const uint8_t*>(&entity) + headerSize, totalSize - headerSize };
		}
	}

	Hash256 CalculateHash(const Block& block) {
		return CalculateHash(block, EntityDataBuffer(block, sizeof(Block)));
	}

	Hash256 CalculateHash(const Transaction& transaction) {
		return CalculateHash(transaction, EntityDataBuffer(transaction, transaction.Size));
	}

	Hash256 CalculateHash(const VerifiableEntity& entity, const RawBuffer& buffer) {
		Hash256 entityHash;
		crypto::Sha3_256_Builder sha3;
		// "R" part of a signature
		sha3.update({ entity.Signature.data(), Signature_Size / 2 });

		// public key is added here to match Sign/Verify behavior, which explicitly hashes it
		sha3.update(entity.Signer);

		sha3.update(buffer);
		sha3.final(entityHash);
		return entityHash;
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

	void UpdateHashes(const TransactionRegistry& transactionRegistry, TransactionElement& transactionElement) {
		const auto& transaction = transactionElement.Transaction;
		const auto& plugin = *transactionRegistry.findPlugin(transaction.Type);

		transactionElement.EntityHash = CalculateHash(transaction, plugin.dataBuffer(transaction));
		transactionElement.MerkleComponentHash = CalculateMerkleComponentHash(
				transaction,
				transactionElement.EntityHash,
				transactionRegistry);
	}
}}
