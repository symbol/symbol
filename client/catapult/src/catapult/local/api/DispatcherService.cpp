#include "DispatcherService.h"
#include "LocalNodeApiState.h"
#include "UnconfirmedTransactionsCacheViewProvider.h"
#include "catapult/cache/ImportanceView.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/consumers/TransactionConsumers.h"
#include "catapult/disruptor/ConsumerDispatcher.h"
#include "catapult/local/DispatcherUtils.h"
#include "catapult/local/ExecutionConfigurationFactory.h"
#include "catapult/local/LocalNodeChainScore.h"
#include "catapult/local/PluginUtils.h"
#include "catapult/local/Sinks.h"
#include "catapult/plugins/PluginManager.h"

using namespace catapult::consumers;
using namespace catapult::disruptor;

namespace catapult { namespace local { namespace api {
	namespace {
		// region BlockDispatcher

		BlockChainSyncHandlers::StateChangeFunc CreateSyncStateChangeHandler(
				LocalNodeChainScore& localScore,
				const LocalNodeStateListeners& listeners) {
			return [&localScore, &listeners](const auto& changeInfo) {
				localScore += changeInfo.ScoreDelta;

				// note: changeInfo contains only DELTA, listener will get both
				// current local score and chainInfo
				listeners.notifyStateChange(localScore.get(), changeInfo);
			};
		}

		std::unique_ptr<ConsumerDispatcher> CreateBlockDispatcher(
				cache::CatapultCache& cache,
				state::CatapultState& state,
				io::BlockStorageCache& storage,
				LocalNodeChainScore& score,
				const plugins::PluginManager& pluginManager,
				chain::UnconfirmedTransactionsUpdater& unconfirmedTransactionsUpdater,
				const config::LocalNodeConfiguration& config,
				const LocalNodeStateListeners& listeners) {
			// note that a hash check consumer isn't needed because new blocks aren't pushed to api nodes (only chains are pulled)
			std::vector<BlockConsumer> consumers;
			AddHashCalculatorConsumer(consumers, pluginManager.transactionRegistry());
			AddBlockChainCheckConsumer(consumers, config.Node);
			auto disruptorConsumers = DisruptorConsumersFromBlockConsumers(consumers);

			BlockChainSyncHandlers syncHandlers;
			syncHandlers.DifficultyChecker = CreateSyncDifficultyChecker(config.BlockChain);
			syncHandlers.UndoBlock = CreateSyncUndoBlockHandler(CreateUndoEntityObserver(pluginManager));
			syncHandlers.Processor = CreateSyncProcessor(config.BlockChain, CreateExecutionConfiguration(pluginManager));
			syncHandlers.StateChange = CreateSyncStateChangeHandler(score, listeners);
			syncHandlers.TransactionsChange = CreateSyncTransactionsChangeHandler(unconfirmedTransactionsUpdater);
			AddBlockChainSyncConsumer(disruptorConsumers, cache, state, storage, syncHandlers);

			return CreateConsumerDispatcher(CreateBlockConsumerDispatcherOptions(config.Node), disruptorConsumers);
		}

		// endregion

		// region TransactionDispatcher

		using TransactionConsumers = std::vector<TransactionConsumer>;

		void AddHashCheckConsumer(
				TransactionConsumers& consumers,
				const UnconfirmedTransactionsCacheViewProvider& unconfirmedTransactionsCacheViewProvider,
				const config::NodeConfiguration& nodeConfig) {
			local::AddHashCheckConsumer(
					consumers,
					nodeConfig,
					[viewProvider = unconfirmedTransactionsCacheViewProvider](auto, const auto& hash) {
						return viewProvider().contains(hash);
					});
		}

		std::unique_ptr<ConsumerDispatcher> CreateTransactionDispatcher(
				const model::TransactionRegistry& transactionRegistry,
				const UnconfirmedTransactionsCacheViewProvider& unconfirmedTransactionsCacheViewProvider,
				chain::UnconfirmedTransactionsUpdater& unconfirmedTransactionsUpdater,
				const SharedNewTransactionsSink& newTransactionsSink,
				const config::LocalNodeConfiguration& config) {
			TransactionConsumers consumers;
			AddHashCalculatorConsumer(consumers, transactionRegistry);
			AddHashCheckConsumer(consumers, unconfirmedTransactionsCacheViewProvider, config.Node);

			auto disruptorConsumers = DisruptorConsumersFromTransactionConsumers(consumers);
			AddNewTransactionsConsumer(disruptorConsumers, unconfirmedTransactionsUpdater, newTransactionsSink);

			return CreateConsumerDispatcher(CreateTransactionConsumerDispatcherOptions(config.Node), disruptorConsumers);
		}

		// endregion
	}

	DispatcherService::DispatcherService(
			const LocalNodeApiState& apiState,
			const plugins::PluginManager& pluginManager,
			const UnconfirmedTransactionsCacheViewProvider& unconfirmedTransactionsCacheViewProvider,
			cache::UtCache& unconfirmedTransactionsCache,
			const SharedNewTransactionsSink& newTransactionsSink)
			: BasicDispatcherService(apiState.stateRef().Config.Node.TransactionBatchPeriod)
			, m_apiState(apiState)
			, m_pluginManager(pluginManager)
			, m_unconfirmedTransactionsCacheViewProvider(unconfirmedTransactionsCacheViewProvider)
			, m_unconfirmedTransactionsCache(unconfirmedTransactionsCache)
			, m_newTransactionsSink(newTransactionsSink)
	{}

	void DispatcherService::boot(thread::MultiServicePool& pool) {
		auto state = m_apiState.stateRef();
		m_pUnconfirmedTransactionsUpdater = CreateUnconfirmedTransactionsUpdater(
				state,
				m_unconfirmedTransactionsCache,
				CreateExecutionConfiguration(m_pluginManager));

		auto pBlockDispatcher = CreateBlockDispatcher(
				state.Cache,
				state.State,
				state.Storage,
				state.Score,
				m_pluginManager,
				*m_pUnconfirmedTransactionsUpdater,
				state.Config,
				m_apiState.listeners());

		auto pTransactionDispatcher = CreateTransactionDispatcher(
				m_pluginManager.transactionRegistry(),
				m_unconfirmedTransactionsCacheViewProvider,
				*m_pUnconfirmedTransactionsUpdater,
				m_newTransactionsSink,
				state.Config);

		BasicDispatcherService::boot(pool, std::move(pBlockDispatcher), std::move(pTransactionDispatcher));
	}
}}}
