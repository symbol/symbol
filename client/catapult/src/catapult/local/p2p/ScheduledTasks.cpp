#include "ScheduledTasks.h"
#include "UnlockedAccountsUtils.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/utils/DiagnosticCounter.h"
#include "catapult/utils/NetworkTime.h"

namespace catapult { namespace local { namespace p2p {

	namespace {
		using HarvestingPredicate = chain::ScheduledHarvesterTaskOptions::HarvestingPredicate;

		chain::ScheduledHarvesterTaskOptions CreateHarvesterTaskOptions(
				const io::BlockStorageCache& storage,
				const chain::CompletionAwareBlockRangeConsumerFunc& blockRangeConsumer,
				const HarvestingPredicate& harvestingAllowed) {
			chain::ScheduledHarvesterTaskOptions options;
			options.HarvestingAllowed = harvestingAllowed;
			options.LastBlockElementSupplier = [&storage]() {
				auto storageView = storage.view();
				return storageView.loadBlockElement(storageView.chainHeight());
			};
			options.TimeGenerator = &utils::NetworkTime;
			options.RangeConsumer = blockRangeConsumer;
			return options;
		}
	}

	thread::Task CreateHarvestingTask(
			const cache::CatapultCache& cache,
			const io::BlockStorageCache& storage,
			const model::BlockChainConfiguration& config,
			chain::UnlockedAccounts& unlockedAccounts,
			const chain::CompletionAwareBlockRangeConsumerFunc& blockRangeConsumer,
			const chain::TransactionsInfoSupplier& transactionsInfoSupplier,
			const HarvestingPredicate& harvestingAllowed) {
		thread::Task task;

		// delay start of harvesting a bit to give the node a chance to connect to peers and estimate the network chain height
		task.StartDelay = utils::TimeSpan::FromSeconds(30);
		task.RepeatDelay = utils::TimeSpan::FromSeconds(1);
		task.Name = "harvesting task";

		auto pHarvesterTask = std::make_shared<chain::ScheduledHarvesterTask>(
				CreateHarvesterTaskOptions(storage, blockRangeConsumer, harvestingAllowed),
				std::make_unique<chain::Harvester>(cache, config, unlockedAccounts, transactionsInfoSupplier));

		task.Callback = [&cache, &unlockedAccounts, pHarvesterTask, minHarvesterBalance = config.MinHarvesterBalance]() {
			// prune accounts that are not eligible to harvest the next block
			PruneUnlockedAccounts(unlockedAccounts, cache, minHarvesterBalance);

			// harvest the next block
			pHarvesterTask->harvest();
			return thread::make_ready_future(thread::TaskResult::Continue);
		};
		return task;
	}

	namespace {
		template<typename TCache>
		size_t GetCacheSize(const TCache& cache) {
			return cache.createView()->size();
		}
	}

	thread::Task CreateLoggingTask(const std::vector<utils::DiagnosticCounter>& counters) {
		thread::Task task;
		task.StartDelay = utils::TimeSpan::FromMinutes(1);
		task.RepeatDelay = utils::TimeSpan::FromMinutes(10);
		task.Name = "logging task";

		task.Callback = [counters]() {
			std::ostringstream table;
			table << "--- current counter values ---";
			for (const auto& counter : counters) {
				table.width(utils::DiagnosticCounterId::Max_Counter_Name_Size);
				table << std::endl << counter.id().name() << " : " << counter.value();
			}

			CATAPULT_LOG(debug) << table.str();
			return thread::make_ready_future(thread::TaskResult::Continue);
		};
		return task;
	}

	thread::Task CreateChainHeightDetectionTask(
			const RemoteChainHeightsRetriever& remoteChainHeightsRetriever,
			NetworkChainHeight& networkHeight) {
		static constexpr size_t Num_Peers = 3;
		thread::Task task;
		task.StartDelay = utils::TimeSpan::FromSeconds(1);
		task.RepeatDelay = utils::TimeSpan::FromSeconds(15);
		task.Name = "network chain height detection";

		task.Callback = [remoteChainHeightsRetriever, &networkHeight]() {
			CATAPULT_LOG(trace) << "starting chain height task";
			return remoteChainHeightsRetriever(Num_Peers).then([&networkHeight](auto&& heightsFuture) {
				auto heights = heightsFuture.get();
				if (heights.empty())
					return thread::TaskResult::Continue;

				auto maxHeight = (*std::max_element(heights.cbegin(), heights.cend())).unwrap();
				auto currentHeight = networkHeight.load();
				if (currentHeight < maxHeight) {
					networkHeight = maxHeight;
					CATAPULT_LOG(debug) << "completed chain height task, network chain height is " << maxHeight;
				}

				return thread::TaskResult::Continue;
			});
		};
		return task;
	}
}}}
