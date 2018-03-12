#pragma once
#include "Block.h"
#include "ContainerTypes.h"
#include "Transaction.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/NonCopyable.h"
#include <memory>

namespace catapult { namespace model {

	/// Tuple composed of an entity and its associated metadata.
	template<typename TEntity>
	struct EntityInfo : public utils::MoveOnly {
	public:
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

	public:
		/// Returns \c true if the info is non-empty and contains a valid entity pointer, \c false otherwise.
		explicit operator bool() const noexcept {
			return !!pEntity;
		}
	};

	/// Hasher object for an EntityInfo.
	template<typename TEntity>
	struct EntityInfoHasher {
		/// Hashes \a info.
		size_t operator()(const EntityInfo<TEntity>& info) const {
			return utils::ArrayHasher<Hash256>()(info.EntityHash);
		}
	};

	/// Comparer object for an EntityInfo.
	template<typename TEntity>
	struct EntityInfoComparer {
		/// Returns \c true if \a lhs and \a rhs are equal.
		bool operator()(const EntityInfo<TEntity>& lhs, const EntityInfo<TEntity>& rhs) const {
			return lhs.EntityHash == rhs.EntityHash;
		}
	};

	/// A transaction and its (partial) associated metadata.
	/// \note This info does not contain `MerkleComponentHash` because it is detached from a block.
	struct DetachedTransactionInfo : EntityInfo<const Transaction> {
	public:
		using EntityInfo<const Transaction>::EntityInfo;

	public:
		/// Creates a transaction info.
		DetachedTransactionInfo() = default;

		/// Creates a transaction info around \a pTransaction without any metadata.
		explicit DetachedTransactionInfo(const std::shared_ptr<const Transaction>& pTransaction)
				: DetachedTransactionInfo(pTransaction, Hash256())
		{}

	public:
		/// Creates a (shallow) copy of this info.
		DetachedTransactionInfo copy() const {
			auto transactionInfo = DetachedTransactionInfo(pEntity, EntityHash);
			transactionInfo.OptionalExtractedAddresses = OptionalExtractedAddresses;
			return transactionInfo;
		}

	public:
		/// The optional extracted addresses.
		std::shared_ptr<const AddressSet> OptionalExtractedAddresses;
	};

	/// A transaction and its associated metadata.
	struct TransactionInfo : DetachedTransactionInfo {
	public:
		/// Creates a transaction info.
		TransactionInfo() = default;

		/// Creates a transaction info around \a pTransaction without any metadata.
		explicit TransactionInfo(const std::shared_ptr<const Transaction>& pTransaction)
				: TransactionInfo(pTransaction, Hash256())
		{}

		/// Creates a transaction info around \a pTransaction and its associated metadata (\a hash).
		explicit TransactionInfo(const std::shared_ptr<const Transaction>& pTransaction, const Hash256& hash)
				: DetachedTransactionInfo(pTransaction, hash)
				, MerkleComponentHash()
		{}

	public:
		/// Creates a (shallow) copy of this info.
		TransactionInfo copy() const {
			auto transactionInfo = TransactionInfo(pEntity, EntityHash);
			transactionInfo.OptionalExtractedAddresses = OptionalExtractedAddresses;
			transactionInfo.MerkleComponentHash = MerkleComponentHash;
			return transactionInfo;
		}

	public:
		/// The modified hash that should be used as a hash in the merkle tree.
		Hash256 MerkleComponentHash;
	};
}}
