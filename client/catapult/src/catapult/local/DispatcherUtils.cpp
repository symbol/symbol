#include "DispatcherUtils.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/BlockDifficultyCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/ImportanceView.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/chain/BlockExecutor.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/chain/ChainUtils.h"
#include "catapult/chain/UnconfirmedTransactionsUpdater.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/consumers/BlockConsumers.h"
#include "catapult/consumers/ReclaimMemoryInspector.h"
#include "catapult/consumers/TransactionConsumers.h"
#include "catapult/utils/NetworkTime.h"

using namespace catapult::consumers;

namespace catapult { namespace local {

	HashCheckOptions CreateHashCheckOptions(const utils::TimeSpan& cacheDuration, const config::NodeConfiguration& nodeConfig) {
		return HashCheckOptions(
				cacheDuration.millis(),
				nodeConfig.ShortLivedCachePruneInterval.millis(),
				nodeConfig.ShortLivedCacheMaxSize);
	}

	// region block consumers

	void AddHashCalculatorConsumer(
			std::vector<disruptor::BlockConsumer>& consumers,
			const model::TransactionRegistry& transactionRegistry) {
		consumers.push_back(CreateBlockHashCalculatorConsumer(transactionRegistry));
	}

	void AddBlockChainCheckConsumer(std::vector<disruptor::BlockConsumer>& consumers, const config::NodeConfiguration& nodeConfig) {
		consumers.push_back(CreateBlockChainCheckConsumer(nodeConfig.MaxBlocksPerSyncAttempt));
	}

	BlockChainSyncHandlers::DifficultyCheckerFunc CreateSyncDifficultyChecker(const model::BlockChainConfiguration& blockChainConfig) {
		return [blockChainConfig](const auto& blocks, const cache::CatapultCache& cache) {
			auto result = chain::CheckDifficulties(cache.sub<cache::BlockDifficultyCache>(), blocks, blockChainConfig);
			return blocks.size() == result;
		};
	}

	BlockChainSyncHandlers::UndoBlockFunc CreateSyncUndoBlockHandler(
			const std::shared_ptr<const observers::EntityObserver>& pUndoObserver) {
		return [pUndoObserver](const auto& blockElement, const auto& state) {
			chain::RollbackBlock(blockElement, *pUndoObserver, state);
		};
	}

	BlockChainProcessor CreateSyncProcessor(
			const model::BlockChainConfiguration& blockChainConfig,
			const chain::ExecutionConfiguration& executionConfig) {
		return CreateBlockChainProcessor(
				[&blockChainConfig](const cache::ReadOnlyCatapultCache& cache) {
					cache::ImportanceView view(cache.sub<cache::AccountStateCache>());
					return chain::BlockHitPredicate(
							blockChainConfig,
							[view](const auto& publicKey, auto height) {
								return view.getAccountImportanceOrDefault(publicKey, height);
							}
					);
				},
				chain::CreateBatchEntityProcessor(executionConfig));
	}

	BlockChainSyncHandlers::TransactionsChangeFunc CreateSyncTransactionsChangeHandler(
			chain::UnconfirmedTransactionsUpdater& unconfirmedTransactionsUpdater) {
		return [&unconfirmedTransactionsUpdater](const auto& addedTransactionHashes, auto&& revertedTransactionInfos) {
			unconfirmedTransactionsUpdater.update(addedTransactionHashes, std::move(revertedTransactionInfos));
		};
	}

	void AddBlockChainSyncConsumer(
			std::vector<disruptor::DisruptorConsumer>& consumers,
			cache::CatapultCache& cache,
			state::CatapultState& state,
			io::BlockStorageCache& storage,
			const BlockChainSyncHandlers& syncHandlers) {
		consumers.push_back(CreateBlockChainSyncConsumer(cache, state, storage, syncHandlers));
	}

	// endregion

	// region transaction consumers

	void AddHashCalculatorConsumer(
			std::vector<disruptor::TransactionConsumer>& consumers,
			const model::TransactionRegistry& transactionRegistry) {
		consumers.push_back(CreateTransactionHashCalculatorConsumer(transactionRegistry));
	}

	void AddHashCheckConsumer(
			std::vector<disruptor::TransactionConsumer>& consumers,
			const config::NodeConfiguration& nodeConfig,
			const std::function<bool (Timestamp, const Hash256&)>& knownHashPredicate) {
		auto pConsumer = CreateTransactionHashCheckConsumer(
				&utils::NetworkTime,
				CreateHashCheckOptions(nodeConfig.ShortLivedCacheTransactionDuration, nodeConfig),
				knownHashPredicate);
		consumers.push_back(std::move(pConsumer));
	}

	void AddNewTransactionsConsumer(
			std::vector<disruptor::DisruptorConsumer>& consumers,
			chain::UnconfirmedTransactionsUpdater& unconfirmedTransactionsUpdater,
			const std::function<void (const consumers::TransactionInfos&)>& newTransactionsSink) {
		auto pConsumer = CreateNewTransactionsConsumer([&unconfirmedTransactionsUpdater, newTransactionsSink](auto&& infos) {
			newTransactionsSink(infos);
			unconfirmedTransactionsUpdater.update(std::move(infos));
		});
		consumers.push_back(std::move(pConsumer));
	}

	// endregion

	disruptor::ConsumerDispatcherOptions CreateBlockConsumerDispatcherOptions(const config::NodeConfiguration& config) {
		return { "block dispatcher", config.BlockDisruptorSize, config.BlockElementTraceInterval };
	}

	disruptor::ConsumerDispatcherOptions CreateTransactionConsumerDispatcherOptions(const config::NodeConfiguration& config) {
		return { "transaction dispatcher", config.TransactionDisruptorSize, config.TransactionElementTraceInterval };
	}

	std::unique_ptr<disruptor::ConsumerDispatcher> CreateConsumerDispatcher(
			const disruptor::ConsumerDispatcherOptions& options,
			const std::vector<disruptor::DisruptorConsumer>& consumers) {
		return std::make_unique<disruptor::ConsumerDispatcher>(options, consumers, CreateReclaimMemoryInspector());
	}
}}
