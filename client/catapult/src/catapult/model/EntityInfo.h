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
		EntityInfo(const std::shared_ptr<TEntity>& pEntityParam, const Hash256& hash)
				: pEntity(pEntityParam)
				, EntityHash(hash)
		{}

		/// Entity pointer.
		std::shared_ptr<TEntity> pEntity;

		/// Entity hash.
		Hash256 EntityHash;

	public:
		/// Returns \c true if the info is not empty and contains a valid entity pointer, \c false otherwise.
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

	/// Transaction and its (partial) associated metadata.
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
		/// Extracted addresses (optional).
		std::shared_ptr<const UnresolvedAddressSet> OptionalExtractedAddresses;
	};

	/// Transaction and its associated metadata.
	struct TransactionInfo : DetachedTransactionInfo {
	public:
		/// Creates a transaction info.
		TransactionInfo() = default;

		/// Creates a transaction info around \a pTransaction without any metadata.
		explicit TransactionInfo(const std::shared_ptr<const Transaction>& pTransaction)
				: TransactionInfo(pTransaction, Hash256())
		{}

		/// Creates a transaction info around \a pTransaction and its associated metadata (\a hash).
		TransactionInfo(const std::shared_ptr<const Transaction>& pTransaction, const Hash256& hash)
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
		/// Modified hash that should be used as a hash in the merkle tree.
		Hash256 MerkleComponentHash;
	};
}}
