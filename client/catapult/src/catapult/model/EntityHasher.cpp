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

	void UpdateHashes(const TransactionRegistry& transactionRegistry, TransactionElement& txElement) {
		const auto& transaction = txElement.Transaction;
		const auto* pPlugin = transactionRegistry.findPlugin(transaction.Type);
		txElement.EntityHash = model::CalculateHash(transaction, pPlugin->dataBuffer(transaction));

		auto supplementaryBuffers = pPlugin->merkleSupplementaryBuffers(transaction);
		if (!supplementaryBuffers.empty()) {
			crypto::Sha3_256_Builder sha3;
			sha3.update(txElement.EntityHash);
			for (const auto& supplementaryBuffer : supplementaryBuffers)
				sha3.update(supplementaryBuffer);

			sha3.final(txElement.MerkleComponentHash);
		} else {
			txElement.MerkleComponentHash = txElement.EntityHash;
		}
	}
}}
