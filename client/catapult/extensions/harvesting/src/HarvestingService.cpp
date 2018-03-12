#include "HarvestingService.h"
#include "HarvestingConfiguration.h"
#include "ScheduledHarvesterTask.h"
#include "UnlockedAccounts.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/cache_core/ImportanceView.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/io/BlockStorageCache.h"

namespace catapult { namespace harvesting {

	namespace {
		std::shared_ptr<UnlockedAccounts> CreateUnlockedAccounts(const HarvestingConfiguration& config) {
			auto pUnlockedAccounts = std::make_shared<UnlockedAccounts>(config.MaxUnlockedAccounts);
			if (config.IsAutoHarvestingEnabled) {
				// unlock boot account if it's eligible to harvest the next block
				auto unlockResult = pUnlockedAccounts->modifier().add(crypto::KeyPair::FromString(config.HarvestKey));
				CATAPULT_LOG(info) << "Unlocked boot key for harvesting with result " << unlockResult;
			}

			return pUnlockedAccounts;
		}

		ScheduledHarvesterTaskOptions CreateHarvesterTaskOptions(extensions::ServiceState& state) {
			ScheduledHarvesterTaskOptions options;
			options.HarvestingAllowed = state.hooks().chainSyncedPredicate();
			options.LastBlockElementSupplier = [&storage = state.storage()]() {
				auto storageView = storage.view();
				return storageView.loadBlockElement(storageView.chainHeight());
			};
			options.TimeSupplier = state.timeSupplier();
			options.RangeConsumer = state.hooks().completionAwareBlockRangeConsumerFactory()(disruptor::InputSource::Local);
			return options;
		}

		void PruneUnlockedAccounts(UnlockedAccounts& unlockedAccounts, const cache::CatapultCache& cache, Amount minHarvesterBalance) {
			auto cacheView = cache.createView();
			auto height = cacheView.height() + Height(1);
			auto readOnlyAccountStateCache = cache::ReadOnlyAccountStateCache(cacheView.sub<cache::AccountStateCache>());
			unlockedAccounts.modifier().removeIf([height, minHarvesterBalance, &readOnlyAccountStateCache](const auto& key) {
				cache::ImportanceView view(readOnlyAccountStateCache);
				return !view.canHarvest(key, height, minHarvesterBalance);
			});
		}

		thread::Task CreateHarvestingTask(extensions::ServiceState& state, UnlockedAccounts& unlockedAccounts) {
			const auto& cache = state.cache();
			const auto& blockChainConfig = state.config().BlockChain;
			auto pHarvesterTask = std::make_shared<ScheduledHarvesterTask>(
					CreateHarvesterTaskOptions(state),
					std::make_unique<Harvester>(
							cache,
							blockChainConfig,
							unlockedAccounts,
							CreateTransactionsInfoSupplier(state.utCache())));

			auto minHarvesterBalance = blockChainConfig.MinHarvesterBalance;
			return thread::CreateNamedTask("harvesting task", [&cache, &unlockedAccounts, pHarvesterTask, minHarvesterBalance]() {
				// prune accounts that are not eligible to harvest the next block
				PruneUnlockedAccounts(unlockedAccounts, cache, minHarvesterBalance);

				// harvest the next block
				pHarvesterTask->harvest();
				return thread::make_ready_future(thread::TaskResult::Continue);
			});
		}

		class HarvestingServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			explicit HarvestingServiceRegistrar(const HarvestingConfiguration& config) : m_config(config)
			{}

			extensions::ServiceRegistrarInfo info() const override {
				return { "Harvesting", extensions::ServiceRegistrarPhase::Post_Range_Consumers };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				locator.registerServiceCounter<UnlockedAccounts>(
						"unlockedAccounts",
						"UNLKED ACCTS",
						[](const auto& accounts) { return accounts.view().size(); });
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				auto pUnlockedAccounts = CreateUnlockedAccounts(m_config);
				locator.registerRootedService("unlockedAccounts", pUnlockedAccounts);

				// add tasks
				state.tasks().push_back(CreateHarvestingTask(state, *pUnlockedAccounts));
			}

		private:
			HarvestingConfiguration m_config;
		};
	}

	DECLARE_SERVICE_REGISTRAR(Harvesting)(const HarvestingConfiguration& config) {
		return std::make_unique<HarvestingServiceRegistrar>(config);
	}
}}
