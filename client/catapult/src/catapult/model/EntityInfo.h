#pragma once
#include "Block.h"
#include "Transaction.h"
#include "catapult/utils/NonCopyable.h"
#include <memory>

namespace catapult { namespace model {

	/// Tuple composed of an entity and its associated metadata.
	template<typename TEntity>
	struct EntityInfo : public utils::MoveOnly {
		/// Creates an entity info.
		EntityInfo() = default;

		/// Creates an entity info around \a pEntityParam and its associated metadata (\a hash).
		explicit EntityInfo(const std::shared_ptr<TEntity>& pEntityParam, const Hash256& hash)
				: pEntity(pEntityParam)
				, EntityHash(hash)
		{}

		/// The entity.
		std::shared_ptr<TEntity> pEntity;

		/// The entity hash.
		Hash256 EntityHash;
	};

	/// A block and its associated metadata..
	using BlockInfo = EntityInfo<const Block>;

	/// A transaction and its associated metadata.
	struct TransactionInfo : EntityInfo<const Transaction> {
	public:
		/// Creates a transaction info.
		TransactionInfo() = default;

		/// Creates a transaction info around \a pEntityParam without any metadata.
		explicit TransactionInfo(const std::shared_ptr<const Transaction>& pEntityParam)
				: TransactionInfo(pEntityParam, Hash256(), Hash256())
		{}

		/// Creates a transaction info around \a pEntityParam and its associated hashes \a entityHash and \a merkleComponentHash.
		explicit TransactionInfo(
				const std::shared_ptr<const Transaction>& pEntityParam,
				const Hash256& entityHash,
				const Hash256& merkleComponentHash)
				: EntityInfo<const Transaction>(pEntityParam, entityHash)
				, MerkleComponentHash(merkleComponentHash)
		{}

	public:
		/// Creates a copy of this info.
		TransactionInfo copy() const {
			return TransactionInfo(pEntity, EntityHash, MerkleComponentHash);
		}

	public:
		/// The modified hash that should be used as a hash in the merkle tree.
		Hash256 MerkleComponentHash;
	};
}}
