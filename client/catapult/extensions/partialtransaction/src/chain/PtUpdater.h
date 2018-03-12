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
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace chain {

	// region results

	/// The result of a transaction update.
	struct TransactionUpdateResult {
		/// Possible update types.
		enum class UpdateType {
			/// A new transaction.
			New,

			/// An existing transaction.
			Existing,

			/// An invalid transaction.
			Invalid
		};

		/// The type of update.
		UpdateType Type;

		/// The number of cosignatures added.
		size_t NumCosignaturesAdded;
	};

	/// The result of a cosignature update.
	enum class CosignatureUpdateResult {
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
		/// using \a pPool for parallelization.
		PtUpdater(
				cache::MemoryPtCacheProxy& transactionsCache,
				std::unique_ptr<const PtValidator>&& pValidator,
				const CompletedTransactionSink& completedTransactionSink,
				const FailedTransactionSink& failedTransactionSink,
				const std::shared_ptr<thread::IoServiceThreadPool>& pPool);

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
