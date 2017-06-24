#include "SchedulerService.h"
#include "LocalNodeFunctionalPlugins.h"
#include "ScheduledTasks.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/chain/TransactionsInfo.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/local/LocalNodeChainScore.h"
#include "catapult/local/LocalNodeStats.h"
#include "catapult/local/SyncScheduledTasks.h"
#include "catapult/net/PacketWriters.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/thread/MultiServicePool.h"
#include "catapult/thread/Scheduler.h"

namespace catapult { namespace local { namespace p2p {

	namespace {
		void EmptyProcessingCompleteFunc(disruptor::DisruptorElementId, disruptor::ConsumerResult)
		{}

		BlockRangeConsumerFactoryFunc CreateBlockRangeConsumerFactory(
				const CompletionAwareBlockRangeConsumerFactoryFunc& completionAwareBlockRangeConsumerFactory) {
			return [factory = completionAwareBlockRangeConsumerFactory](auto source) {
				return [factory, source](auto&& range) {
					factory(source)(std::move(range), EmptyProcessingCompleteFunc);
				};
			};
		}
	}

	SchedulerService::SchedulerService(
			const LocalNodeStateConstRef& stateRef,
			const plugins::PluginManager& pluginManager,
			const cache::MemoryUtCache& unconfirmedTransactionsCache,
			NetworkChainHeight& networkChainHeight,
			const CompletionAwareBlockRangeConsumerFactoryFunc& completionAwareBlockRangeConsumerFactory,
			const chain::TransactionRangeConsumerFunc& transactionRangeConsumer)
			: m_stateRef(stateRef)
			, m_pluginManager(pluginManager)
			, m_unconfirmedTransactionsCache(unconfirmedTransactionsCache)
			, m_networkChainHeight(networkChainHeight)
			, m_completionAwareBlockRangeConsumerFactory(completionAwareBlockRangeConsumerFactory)
			, m_blockRangeConsumerFactory(CreateBlockRangeConsumerFactory(m_completionAwareBlockRangeConsumerFactory))
			, m_transactionRangeConsumer(transactionRangeConsumer)
	{}

	size_t SchedulerService::numScheduledTasks() const {
		return GetStatsValue(m_pScheduler.lock(), &thread::Scheduler::numScheduledTasks);
	}

	void SchedulerService::boot(
			thread::MultiServicePool& pool,
			net::PacketWriters& packetWriters,
			chain::UnlockedAccounts& unlockedAccounts) {
		auto pScheduler = pool.pushServiceGroup("scheduler")->pushService(thread::CreateScheduler);
		pScheduler->addTask(createConnectPeersTask(packetWriters));
		pScheduler->addTask(createSynchronizerTask(packetWriters));
		pScheduler->addTask(createChainHeightDetectionTask(packetWriters));

		pScheduler->addTask(createHarvesterTask(unlockedAccounts));
		pScheduler->addTask(createLoggingTask());
		m_pScheduler = pScheduler;
	}

	thread::Task SchedulerService::createConnectPeersTask(net::PacketWriters& packetWriters) {
		return CreateConnectPeersTask(m_stateRef.Config.Peers, packetWriters);
	}

	thread::Task SchedulerService::createSynchronizerTask(net::PacketWriters& packetWriters) {
		const auto& score = m_stateRef.Score;
		const auto& unconfirmedTransactionsCache = m_unconfirmedTransactionsCache;
		return CreateSynchronizerTask(
				m_stateRef.Config,
				m_stateRef.Storage,
				m_pluginManager.transactionRegistry(),
				packetWriters,
				[&score]() { return score.get(); },
				[&unconfirmedTransactionsCache]() { return unconfirmedTransactionsCache.view().shortHashes(); },
				m_completionAwareBlockRangeConsumerFactory(disruptor::InputSource::Remote_Pull),
				m_transactionRangeConsumer);
	}

	thread::Task SchedulerService::createChainHeightDetectionTask(net::PacketWriters& packetWriters) {
		return CreateChainHeightDetectionTask(CreateRemoteChainHeightsRetriever(packetWriters), m_networkChainHeight);
	}

	thread::Task SchedulerService::createHarvesterTask(chain::UnlockedAccounts& unlockedAccounts) {
		const auto& networkChainHeight = m_networkChainHeight;
		const auto& unconfirmedTransactionsCache = m_unconfirmedTransactionsCache;
		return CreateHarvestingTask(
				m_stateRef.Cache,
				m_stateRef.Storage,
				m_stateRef.Config.BlockChain,
				unlockedAccounts,
				m_completionAwareBlockRangeConsumerFactory(disruptor::InputSource::Local),
				chain::CreateTransactionsInfoSupplier(unconfirmedTransactionsCache),
				CreateChainSyncedPredicate(m_stateRef.Storage, [&networkChainHeight]() { return networkChainHeight.load(); }));
	}

	thread::Task SchedulerService::createLoggingTask() {
		std::vector<utils::DiagnosticCounter> counters;
		m_pluginManager.addDiagnosticCounters(counters, m_stateRef.Cache);
		return CreateLoggingTask(counters);
	}
}}}
