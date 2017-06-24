#include "DispatcherService.h"
#include "HashPredicateFactory.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/chain/UnconfirmedTransactionsUpdater.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/consumers/BlockConsumers.h"
#include "catapult/consumers/TransactionConsumers.h"
#include "catapult/disruptor/ConsumerDispatcher.h"
#include "catapult/local/DispatcherUtils.h"
#include "catapult/local/ExecutionConfigurationFactory.h"
#include "catapult/local/LocalNodeChainScore.h"
#include "catapult/local/PluginUtils.h"
#include "catapult/local/Sinks.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/thread/MultiServicePool.h"
#include "catapult/utils/NetworkTime.h"
#include "catapult/validators/AggregateEntityValidator.h"

using namespace catapult::consumers;
using namespace catapult::disruptor;

namespace catapult { namespace local { namespace p2p {
	namespace {
		// region BlockDispatcher

		using DisruptorConsumers = std::vector<DisruptorConsumer>;
		using BlockConsumers = std::vector<BlockConsumer>;

		void AddHashCheckConsumer(BlockConsumers& consumers, const config::NodeConfiguration& nodeConfig) {
			auto hashCheckOptions = CreateHashCheckOptions(nodeConfig.ShortLivedCacheBlockDuration, nodeConfig);
			consumers.push_back(CreateBlockHashCheckConsumer(&utils::NetworkTime, hashCheckOptions));
		}

		void AddStatelessValidationConsumer(
				BlockConsumers& consumers,
				const plugins::PluginManager& pluginManager,
				const std::shared_ptr<thread::IoServiceThreadPool>& pValidatorPool,
				const model::MatchingEntityPredicate& unknownTransactionPredicate) {
			auto pConsumer = CreateBlockStatelessValidationConsumer(
					CreateStatelessValidator(pluginManager),
					validators::CreateParallelValidationPolicy(pValidatorPool),
					unknownTransactionPredicate);
			consumers.push_back(std::move(pConsumer));
		}

		BlockChainSyncHandlers::StateChangeFunc CreateSyncStateChangeHandler(LocalNodeChainScore& localScore) {
			return [&localScore](const auto& changeInfo) {
				localScore += changeInfo.ScoreDelta;
			};
		}

		void AddNewBlockConsumer(DisruptorConsumers& consumers, const NewBlockSink& newBlockSink) {
			consumers.push_back(CreateNewBlockConsumer(newBlockSink, InputSource::Local));
		}

		std::unique_ptr<ConsumerDispatcher> CreateBlockDispatcher(
				cache::CatapultCache& cache,
				state::CatapultState& state,
				io::BlockStorageCache& storage,
				LocalNodeChainScore& score,
				const plugins::PluginManager& pluginManager,
				const model::MatchingEntityPredicate& unknownTransactionPredicate,
				chain::UnconfirmedTransactionsUpdater& unconfirmedTransactionsUpdater,
				const NewBlockSink& newBlockSink,
				const config::LocalNodeConfiguration& config,
				const std::shared_ptr<thread::IoServiceThreadPool>& pValidatorPool) {
			std::vector<BlockConsumer> consumers;
			AddHashCalculatorConsumer(consumers, pluginManager.transactionRegistry());
			AddHashCheckConsumer(consumers, config.Node);
			AddBlockChainCheckConsumer(consumers, config.Node);

			AddStatelessValidationConsumer(consumers, pluginManager, pValidatorPool, unknownTransactionPredicate);
			auto disruptorConsumers = DisruptorConsumersFromBlockConsumers(consumers);

			BlockChainSyncHandlers syncHandlers;
			syncHandlers.DifficultyChecker = CreateSyncDifficultyChecker(config.BlockChain);
			syncHandlers.UndoBlock = CreateSyncUndoBlockHandler(CreateUndoEntityObserver(pluginManager));
			syncHandlers.Processor = CreateSyncProcessor(config.BlockChain, CreateExecutionConfiguration(pluginManager));
			syncHandlers.StateChange = CreateSyncStateChangeHandler(score);
			syncHandlers.TransactionsChange = CreateSyncTransactionsChangeHandler(unconfirmedTransactionsUpdater);
			AddBlockChainSyncConsumer(disruptorConsumers, cache, state, storage, syncHandlers);

			AddNewBlockConsumer(disruptorConsumers, newBlockSink);

			return CreateConsumerDispatcher(CreateBlockConsumerDispatcherOptions(config.Node), disruptorConsumers);
		}

		// endregion

		// region TransactionDispatcher

		using TransactionConsumers = std::vector<TransactionConsumer>;

		void AddHashCheckConsumer(
				TransactionConsumers& consumers,
				const consumers::KnownHashPredicate& knownHashPredicate,
				const config::NodeConfiguration& nodeConfig) {
			local::AddHashCheckConsumer(consumers, nodeConfig, knownHashPredicate);
		}

		void AddStatelessValidationConsumer(
				TransactionConsumers& consumers,
				const plugins::PluginManager& pluginManager,
				const std::shared_ptr<thread::IoServiceThreadPool>& pValidatorPool) {
			auto pConsumer = CreateTransactionStatelessValidationConsumer(
					CreateStatelessValidator(pluginManager),
					validators::CreateParallelValidationPolicy(pValidatorPool));
			consumers.push_back(std::move(pConsumer));
		}

		std::unique_ptr<ConsumerDispatcher> CreateTransactionDispatcher(
				const plugins::PluginManager& pluginManager,
				const consumers::KnownHashPredicate& knownHashPredicate,
				chain::UnconfirmedTransactionsUpdater& unconfirmedTransactionsUpdater,
				const SharedNewTransactionsSink& newTransactionsSink,
				const config::LocalNodeConfiguration& config,
				const std::shared_ptr<thread::IoServiceThreadPool>& pValidatorPool) {
			TransactionConsumers consumers;
			AddHashCalculatorConsumer(consumers, pluginManager.transactionRegistry());
			AddHashCheckConsumer(consumers, knownHashPredicate, config.Node);
			AddStatelessValidationConsumer(consumers, pluginManager, pValidatorPool);

			auto disruptorConsumers = DisruptorConsumersFromTransactionConsumers(consumers);
			AddNewTransactionsConsumer(disruptorConsumers, unconfirmedTransactionsUpdater, newTransactionsSink);

			return CreateConsumerDispatcher(CreateTransactionConsumerDispatcherOptions(config.Node), disruptorConsumers);
		}

		// endregion
	}

	DispatcherService::DispatcherService(
			const LocalNodeStateRef& stateRef,
			const plugins::PluginManager& pluginManager,
			cache::MemoryUtCache& unconfirmedTransactionsCache,
			const HashPredicateFactory& hashPredicateFactory,
			const NewBlockSink& newBlockSink,
			const SharedNewTransactionsSink& newTransactionsSink)
			: BasicDispatcherService(stateRef.Config.Node.TransactionBatchPeriod)
			, m_stateRef(stateRef)
			, m_pluginManager(pluginManager)
			, m_unconfirmedTransactionsCache(unconfirmedTransactionsCache)
			, m_hashPredicateFactory(hashPredicateFactory)
			, m_newBlockSink(newBlockSink)
			, m_newTransactionsSink(newTransactionsSink)
	{}

	void DispatcherService::boot(thread::MultiServicePool& pool) {
		m_pUnconfirmedTransactionsUpdater = CreateUnconfirmedTransactionsUpdater(
				m_stateRef,
				m_unconfirmedTransactionsCache,
				CreateExecutionConfiguration(m_pluginManager));

		auto pValidatorPool = pool.pushIsolatedPool(thread::MultiServicePool::DefaultPoolConcurrency(), "validator");

		auto pBlockDispatcher = CreateBlockDispatcher(
				m_stateRef.Cache,
				m_stateRef.State,
				m_stateRef.Storage,
				m_stateRef.Score,
				m_pluginManager,
				m_hashPredicateFactory.createUnknownTransactionPredicate(),
				*m_pUnconfirmedTransactionsUpdater,
				m_newBlockSink,
				m_stateRef.Config,
				pValidatorPool);

		auto pTransactionDispatcher = CreateTransactionDispatcher(
				m_pluginManager,
				m_hashPredicateFactory.createKnownHashPredicate(),
				*m_pUnconfirmedTransactionsUpdater,
				m_newTransactionsSink,
				m_stateRef.Config,
				pValidatorPool);

		BasicDispatcherService::boot(pool, std::move(pBlockDispatcher), std::move(pTransactionDispatcher));
	}
}}}
