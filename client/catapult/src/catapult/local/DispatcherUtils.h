#pragma once
#include "catapult/consumers/BlockChainSyncHandlers.h"
#include "catapult/consumers/HashCheckOptions.h"
#include "catapult/consumers/InputUtils.h"
#include "catapult/disruptor/ConsumerDispatcher.h"
#include "catapult/disruptor/DisruptorConsumer.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace chain {
		struct ExecutionConfiguration;
		class UnconfirmedTransactionsUpdater;
	}
	namespace config { struct NodeConfiguration; }
	namespace io { class BlockStorageCache; }
	namespace model {
		struct BlockChainConfiguration;
		class TransactionRegistry;
	}
	namespace state { struct CatapultState; }
	namespace utils { class TimeSpan; }
}

namespace catapult { namespace local {

	/// Creates hash check options given \a cacheDuration and \a nodeConfig.
	consumers::HashCheckOptions CreateHashCheckOptions(const utils::TimeSpan& cacheDuration, const config::NodeConfiguration& nodeConfig);

	// region block consumers

	/// Adds a hash calculator consumer to \a consumers with \a transactionRegistry.
	void AddHashCalculatorConsumer(
			std::vector<disruptor::BlockConsumer>& consumers,
			const model::TransactionRegistry& transactionRegistry);

	/// Adds a block chain check consumer to \a consumers with \a nodeConfig configuration.
	void AddBlockChainCheckConsumer(std::vector<disruptor::BlockConsumer>& consumers, const config::NodeConfiguration& nodeConfig);

	/// Creates a sync difficulty checker for the block chain described by \a blockChainConfig.
	consumers::BlockChainSyncHandlers::DifficultyCheckerFunc CreateSyncDifficultyChecker(
			const model::BlockChainConfiguration& blockChainConfig);

	/// Creates a sync undo block handler using \a pUndoObserver.
	consumers::BlockChainSyncHandlers::UndoBlockFunc CreateSyncUndoBlockHandler(
			const std::shared_ptr<const observers::EntityObserver>& pUndoObserver);

	/// Creates a sync processor for the block chain described by \a blockChainConfig using \a executionConfig.
	consumers::BlockChainProcessor CreateSyncProcessor(
			const model::BlockChainConfiguration& blockChainConfig,
			const chain::ExecutionConfiguration& executionConfig);

	/// Creates a sync transactions change handler around \a unconfirmedTransactionsUpdater.
	consumers::BlockChainSyncHandlers::TransactionsChangeFunc CreateSyncTransactionsChangeHandler(
			chain::UnconfirmedTransactionsUpdater& unconfirmedTransactionsUpdater);

	/// Adds a block chain sync consumer to \a consumers with state composed of \a cache, \a state and \a storage
	/// and handlers \a syncHandlers.
	void AddBlockChainSyncConsumer(
			std::vector<disruptor::DisruptorConsumer>& consumers,
			cache::CatapultCache& cache,
			state::CatapultState& state,
			io::BlockStorageCache& storage,
			const consumers::BlockChainSyncHandlers& syncHandlers);

	// endregion

	// region transaction consumers

	/// Adds a hash calculator consumer to \a consumers with \a transactionRegistry.
	void AddHashCalculatorConsumer(
			std::vector<disruptor::TransactionConsumer>& consumers,
			const model::TransactionRegistry& transactionRegistry);

	/// Adds a hash check consumer to \a consumers with \a nodeConfig configuration and a \a knownHashPredicate.
	void AddHashCheckConsumer(
			std::vector<disruptor::TransactionConsumer>& consumers,
			const config::NodeConfiguration& nodeConfig,
			const std::function<bool (Timestamp, const Hash256&)>& knownHashPredicate);

	/// Adds a new transactions consumer to \a consumers that passes new transactions first to \a newTransactionsSink
	/// and then to \a unconfirmedTransactionsUpdater.
	void AddNewTransactionsConsumer(
			std::vector<disruptor::DisruptorConsumer>& consumers,
			chain::UnconfirmedTransactionsUpdater& unconfirmedTransactionsUpdater,
			const std::function<void (const consumers::TransactionInfos&)>& newTransactionsSink);

	// endregion

	/// Creates consumer dispatcher options for a block dispatcher from \a config.
	disruptor::ConsumerDispatcherOptions CreateBlockConsumerDispatcherOptions(const config::NodeConfiguration& config);

	/// Creates consumer dispatcher options for a transaction dispatcher from \a config.
	disruptor::ConsumerDispatcherOptions CreateTransactionConsumerDispatcherOptions(const config::NodeConfiguration& config);

	/// Creates a dispatcher of \a consumers configured with \a options.
	std::unique_ptr<disruptor::ConsumerDispatcher> CreateConsumerDispatcher(
			const disruptor::ConsumerDispatcherOptions& options,
			const std::vector<disruptor::DisruptorConsumer>& consumers);
}}
