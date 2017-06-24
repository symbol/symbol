#pragma once
#include "NetworkHeight.h"
#include "catapult/chain/ChainSynchronizer.h"
#include "catapult/chain/ScheduledHarvesterTask.h"
#include "catapult/chain/TransactionsInfo.h"
#include "catapult/thread/Scheduler.h"

namespace catapult {
	namespace cache {
		class CatapultCache;
		class MemoryUtCache;
	}
	namespace chain { class UnlockedAccounts; }
	namespace io { class BlockStorageCache; }
	namespace utils { class DiagnosticCounter; }
}

namespace catapult { namespace local { namespace p2p {

	/// Creates a harvesting task around the specified \a cache, \a storage, \a config, \a unlockedAccounts,
	/// \a blockRangeConsumer, \a transactionsInfoSupplier and \a harvestingAllowed.
	thread::Task CreateHarvestingTask(
			const cache::CatapultCache& cache,
			const io::BlockStorageCache& storage,
			const model::BlockChainConfiguration& config,
			chain::UnlockedAccounts& unlockedAccounts,
			const chain::CompletionAwareBlockRangeConsumerFunc& blockRangeConsumer,
			const chain::TransactionsInfoSupplier& transactionsInfoSupplier,
			const chain::ScheduledHarvesterTaskOptions::HarvestingPredicate& harvestingAllowed);

	/// Creates a logging task around \a counters.
	thread::Task CreateLoggingTask(const std::vector<utils::DiagnosticCounter>& counters);

	/// Create a task that detects the block chain height of the network.
	/// The task uses \a remoteChainHeightsRetriever to retrieve the remote chain heights
	//// and updates the network height (\a networkHeight).
	thread::Task CreateChainHeightDetectionTask(
			const RemoteChainHeightsRetriever& remoteChainHeightsRetriever,
			NetworkChainHeight& networkHeight);
}}}
