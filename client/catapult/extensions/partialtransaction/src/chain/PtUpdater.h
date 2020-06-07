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
#include "catapult/chain/ChainFunctions.h"
#include "catapult/thread/Future.h"
#include <memory>

namespace catapult {
	namespace cache { class MemoryPtCacheProxy; }
	namespace chain { class PtValidator; }
	namespace model {
		struct DetachedCosignature;
		struct Transaction;
		struct TransactionInfo;
	}
	namespace thread { class IoThreadPool; }
}

namespace catapult { namespace chain {

	// region results

	/// Result of a transaction update.
	struct TransactionUpdateResult {
		/// Possible update types.
		enum class UpdateType {
			/// New transaction.
			New,

			/// Existing transaction.
			Existing,

			/// Invalid transaction.
			Invalid
		};

		/// Type of the update.
		UpdateType Type;

		/// Number of cosignatures added.
		size_t NumCosignaturesAdded;
	};

	/// Result of a cosignature update.
	enum class CosignatureUpdateResult {
		/// Error occurred during processing of cosignature.
		Error,

		/// Cosignature is ineligible.
		Ineligible,

		/// Cosignature is unverifiable.
		Unverifiable,

		/// Cosignature is redundant.
		Redundant,

		/// Cosignature is added and did not complete the owning transaction.
		Added_Incomplete,

		/// Cosignature is added and completed the owning transaction.
		Added_Complete
	};

	// endregion

	/// Provides updating of a partial transactions cache.
	class PtUpdater {
	public:
		/// Sink that is passed completed transactions.
		using CompletedTransactionSink = consumer<std::unique_ptr<model::Transaction>&&>;

	public:
		/// Creates an updater around \a transactionsCache, \a pValidator, \a completedTransactionSink and \a failedTransactionSink
		/// using \a pool for parallelization.
		PtUpdater(
				cache::MemoryPtCacheProxy& transactionsCache,
				std::unique_ptr<const PtValidator>&& pValidator,
				const CompletedTransactionSink& completedTransactionSink,
				const FailedTransactionSink& failedTransactionSink,
				thread::IoThreadPool& pool);

		/// Destroys the updater.
		~PtUpdater();

	public:
		/// Updates this cache by adding a new transaction info (\a transactionInfo).
		thread::future<TransactionUpdateResult> update(const model::TransactionInfo& transactionInfo);

		/// Updates this cache by adding a new \a cosignature.
		thread::future<CosignatureUpdateResult> update(const model::DetachedCosignature& cosignature);

	private:
		class Impl;
		std::shared_ptr<Impl> m_pImpl; // shared_ptr to allow use of enable_shared_from_this
	};
}}
