/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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
				// unlock configured account if it's eligible to harvest the next block
				auto keyPair = crypto::KeyPair::FromString(config.HarvestKey);
				auto publicKey = keyPair.publicKey();

				auto unlockResult = pUnlockedAccounts->modifier().add(std::move(keyPair));
				CATAPULT_LOG(info)
						<< "Unlocked harvesting account " << utils::HexFormat(publicKey)
						<< " for harvesting with result " << unlockResult;
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
