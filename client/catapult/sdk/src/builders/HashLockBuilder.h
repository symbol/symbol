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
#include "TransactionBuilder.h"
#include "plugins/txes/lock_hash/src/model/HashLockTransaction.h"

namespace catapult { namespace builders {

	/// Builder for a hash lock transaction.
	class HashLockBuilder : public TransactionBuilder {
	public:
		using Transaction = model::HashLockTransaction;
		using EmbeddedTransaction = model::EmbeddedHashLockTransaction;

	public:
		/// Creates a hash lock builder for building a hash lock transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		HashLockBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the lock mosaic to \a mosaic.
		void setMosaic(const model::UnresolvedMosaic& mosaic);

		/// Sets the number of blocks for which a lock should be valid to \a duration.
		void setDuration(BlockDuration duration);

		/// Sets the lock hash to \a hash.
		void setHash(const Hash256& hash);

	public:
		/// Gets the size of hash lock transaction.
		/// \note This returns size of a normal transaction not embedded transaction.
		size_t size() const;

		/// Builds a new hash lock transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded hash lock transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		size_t sizeImpl() const;

		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		model::UnresolvedMosaic m_mosaic;
		BlockDuration m_duration;
		Hash256 m_hash;
	};
}}
