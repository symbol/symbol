#include "SchedulerService.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/local/LocalNodeChainScore.h"
#include "catapult/local/LocalNodeStats.h"
#include "catapult/local/SyncScheduledTasks.h"
#include "catapult/thread/MultiServicePool.h"
#include "catapult/thread/Scheduler.h"

namespace catapult { namespace local { namespace api {

	SchedulerService::SchedulerService(
			const LocalNodeStateConstRef& stateRef,
			const model::TransactionRegistry& transactionRegistry,
			const UnconfirmedTransactionsCacheViewProvider& unconfirmedTransactionsCacheViewProvider,
			const CompletionAwareBlockRangeConsumerFactoryFunc& completionAwareBlockRangeConsumerFactory)
			: m_stateRef(stateRef)
			, m_transactionRegistry(transactionRegistry)
			, m_unconfirmedTransactionsCacheViewProvider(unconfirmedTransactionsCacheViewProvider)
			, m_completionAwareBlockRangeConsumerFactory(completionAwareBlockRangeConsumerFactory)
	{}

	size_t SchedulerService::numScheduledTasks() const {
		return GetStatsValue(m_pScheduler.lock(), &thread::Scheduler::numScheduledTasks);
	}

	void SchedulerService::boot(thread::MultiServicePool& pool, net::PacketWriters& packetWriters) {
		auto pScheduler = pool.pushServiceGroup("scheduler")->pushService(thread::CreateScheduler);
		pScheduler->addTask(createConnectPeersTask(packetWriters));
		pScheduler->addTask(createSynchronizerTask(packetWriters));
		m_pScheduler = pScheduler;
	}

	thread::Task SchedulerService::createConnectPeersTask(net::PacketWriters& packetWriters) {
		return CreateConnectPeersTask(m_stateRef.Config.Peers, packetWriters);
	}

	thread::Task SchedulerService::createSynchronizerTask(net::PacketWriters& packetWriters) {
		const auto& score = m_stateRef.Score;
		return CreateSynchronizerTask(
				m_stateRef.Config,
				m_stateRef.Storage,
				m_transactionRegistry,
				packetWriters,
				[&score]() { return score.get(); },
				[&viewProvider = m_unconfirmedTransactionsCacheViewProvider]() { return viewProvider().shortHashes(); },
				m_completionAwareBlockRangeConsumerFactory(disruptor::InputSource::Remote_Pull),
				[](auto&&) {});
	}
}}}
