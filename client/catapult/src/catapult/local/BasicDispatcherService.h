#pragma once
#include "ConsumerFactory.h"
#include "catapult/chain/UnconfirmedTransactionsUpdater.h"
#include "catapult/disruptor/BatchRangeDispatcher.h"
#include "catapult/disruptor/ConsumerDispatcher.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/utils/TimeSpan.h"

namespace catapult {
	namespace local { struct LocalNodeStateRef; }
	namespace thread {
		class MultiServicePool;
		struct Task;
	}
}

namespace catapult { namespace local {

	/// Status information about a dispatcher.
	struct DispatcherStatus {
		/// The dispatcher name.
		std::string Name;

		/// The dispatcher size.
		size_t Size;

		/// \c true if the dispatcher is currently running.
		bool IsRunning;
	};

	/// A basic service for managing dispatchers.
	class BasicDispatcherService : public utils::NonCopyable {
	protected:
		/// Creates a new basic dispatcher service with a transaction batch period of \a transactionBatchPeriod.
		explicit BasicDispatcherService(const utils::TimeSpan& transactionBatchPeriod);

	public:
		/// Creates a factory for creating a block range consumer bound to an input source that forwards blocks
		/// to the block dispatcher.
		BlockRangeConsumerFactoryFunc createBlockRangeConsumerFactory();

		/// Creates a factory for creating a completion aware block range consumer bound to an input source that forwards blocks
		/// to the block dispatcher.
		CompletionAwareBlockRangeConsumerFactoryFunc createCompletionAwareBlockRangeConsumerFactory();

		/// Creates a factory for creating a transaction range consumer bound to an input source that forwards transactions
		/// to the transaction dispatcher.
		TransactionRangeConsumerFactoryFunc createTransactionRangeConsumerFactory();

	public:
		/// Returns the total number of block elements that have been added to the dispatcher.
		size_t numAddedBlockElements() const;

		/// Returns the total number of transaction elements that have been added to the dispatcher.
		size_t numAddedTransactionElements() const;

		/// Returns the current status of the block dispatcher.
		DispatcherStatus blockDispatcherStatus() const;

		/// Returns the current status of the transaction dispatcher.
		DispatcherStatus transactionDispatcherStatus() const;

	protected:
		/// Boots common services given the dispatchers \a pBlockDispatcher and \a pTransactionDispatcher
		/// with \a pool parallelization.
		void boot(
				thread::MultiServicePool& pool,
				std::unique_ptr<disruptor::ConsumerDispatcher>&& pBlockDispatcher,
				std::unique_ptr<disruptor::ConsumerDispatcher>&& pTransactionDispatcher);

	protected:
		/// Creates an appropriate unconfirmed transactions updater given state (composed of \a stateRef
		/// and \a unconfirmedTransactionsCache) and an execution configuration (\a executionConfiguration).
		static std::unique_ptr<chain::UnconfirmedTransactionsUpdater> CreateUnconfirmedTransactionsUpdater(
				const LocalNodeStateRef& stateRef,
				cache::UtCache& unconfirmedTransactionsCache,
				const chain::ExecutionConfiguration& executionConfiguration);

	private:
		thread::Task createBatchTransactionTask() const;

	private:
		using TransactionBatchRangeDispatcher = disruptor::BatchRangeDispatcher<model::TransactionRange>;

	private:
		// state
		utils::TimeSpan m_transactionBatchPeriod;

		// services
		std::weak_ptr<disruptor::ConsumerDispatcher> m_pBlockDispatcher;
		std::weak_ptr<disruptor::ConsumerDispatcher> m_pTransactionDispatcher;
		std::unique_ptr<TransactionBatchRangeDispatcher> m_pTransactionBatchRangeDispatcher;
	};
}}
