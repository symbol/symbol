#pragma once
#include "NetworkHeight.h"
#include "catapult/local/ConsumerFactory.h"
#include "catapult/local/LocalNodeStateRef.h"
#include "catapult/utils/NonCopyable.h"

namespace catapult {
	namespace cache { class MemoryUtCache; }
	namespace chain { class UnlockedAccounts; }
	namespace net { class PacketWriters; }
	namespace plugins { class PluginManager; }
	namespace thread {
		class MultiServicePool;
		class Scheduler;
		struct Task;
	}
}

namespace catapult { namespace local { namespace p2p {

	/// A service for managing peer scheduled tasks.
	struct SchedulerService : public utils::NonCopyable {
	public:
		/// Creates a new peer scheduler service around state composed of \a stateRef, \a pluginManager,
		/// \a unconfirmedTransactionsCache, and \a networkChainHeight.
		/// \a completionAwareBlockRangeConsumerFactory and \a transactionRangeConsumer are used to configure
		/// block and transaction range processing.
		explicit SchedulerService(
				const LocalNodeStateConstRef& stateRef,
				const plugins::PluginManager& pluginManager,
				const cache::MemoryUtCache& unconfirmedTransactionsCache,
				NetworkChainHeight& networkChainHeight,
				const CompletionAwareBlockRangeConsumerFactoryFunc& completionAwareBlockRangeConsumerFactory,
				const chain::TransactionRangeConsumerFunc& transactionRangeConsumer);

	public:
		/// Returns the total number of scheduled tasks.
		size_t numScheduledTasks() const;

	public:
		/// Boots the service using \a pool parallelization with \a packetWriters and \a unlockedAccounts.
		void boot(thread::MultiServicePool& pool, net::PacketWriters& packetWriters, chain::UnlockedAccounts& unlockedAccounts);

	private:
		thread::Task createConnectPeersTask(net::PacketWriters& packetWriters);

		thread::Task createSynchronizerTask(net::PacketWriters& packetWriters);

		thread::Task createChainHeightDetectionTask(net::PacketWriters& packetWriters);

		thread::Task createHarvesterTask(chain::UnlockedAccounts& unlockedAccounts);

		thread::Task createLoggingTask();

	private:
		// state
		LocalNodeStateConstRef m_stateRef;
		const plugins::PluginManager& m_pluginManager;
		const cache::MemoryUtCache& m_unconfirmedTransactionsCache;
		NetworkChainHeight& m_networkChainHeight;
		CompletionAwareBlockRangeConsumerFactoryFunc m_completionAwareBlockRangeConsumerFactory;
		BlockRangeConsumerFactoryFunc m_blockRangeConsumerFactory;
		chain::TransactionRangeConsumerFunc m_transactionRangeConsumer;

		// services
		std::weak_ptr<thread::Scheduler> m_pScheduler;
	};
}}}
