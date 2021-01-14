/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "HarvesterBlockGenerator.h"
#include "HarvestingUtFacadeFactory.h"
#include "ScheduledHarvesterTask.h"
#include "UnlockedAccounts.h"
#include "UnlockedAccountsUpdater.h"
#include "catapult/cache_core/ImportanceView.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/config/CatapultKeys.h"
#include "catapult/extensions/ConfigurationUtils.h"
#include "catapult/extensions/ExecutionConfigurationFactory.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "catapult/model/EntityRange.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/utils/HexParser.h"

namespace catapult { namespace harvesting {

	namespace {
		// region CreateUnlockedAccountsUpdater

		struct UnlockedAccountsHolder {
			std::shared_ptr<UnlockedAccounts> pUnlockedAccounts;
			std::shared_ptr<UnlockedAccountsUpdater> pUnlockedAccountsUpdater;
		};

		BlockGeneratorAccountDescriptor CreateBlockGeneratorAccountDescriptor(const HarvestingConfiguration& config) {
			if (!config.EnableAutoHarvesting)
				return BlockGeneratorAccountDescriptor();

			return BlockGeneratorAccountDescriptor(
					crypto::KeyPair::FromString(config.HarvesterSigningPrivateKey),
					crypto::KeyPair::FromString(config.HarvesterVrfPrivateKey));
		}

		std::shared_ptr<UnlockedAccounts> CreateUnlockedAccounts(
				const HarvestingConfiguration& config,
				const cache::CatapultCache& cache,
				BlockGeneratorAccountDescriptor&& blockGeneratorAccountDescriptor) {
			auto harvesterSigningPublicKey = blockGeneratorAccountDescriptor.signingKeyPair().publicKey();
			auto harvesterVrfPublicKey = blockGeneratorAccountDescriptor.vrfKeyPair().publicKey();

			auto pUnlockedAccounts = std::make_shared<UnlockedAccounts>(
					config.MaxUnlockedAccounts,
					CreateDelegatePrioritizer(config.DelegatePrioritizationPolicy, cache, harvesterSigningPublicKey));

			if (config.EnableAutoHarvesting) {
				// unlock configured account if it's eligible to harvest the next block
				auto unlockResult = pUnlockedAccounts->modifier().add(std::move(blockGeneratorAccountDescriptor));
				CATAPULT_LOG(important)
						<< std::endl << "Unlocked harvesting account with result " << unlockResult
						<< std::endl << "+ Signing " << harvesterSigningPublicKey
						<< std::endl << "+ VRF     " << harvesterVrfPublicKey;
			}

			return pUnlockedAccounts;
		}

		UnlockedAccountsHolder CreateUnlockedAccountsHolder(
				const HarvestingConfiguration& config,
				const extensions::ServiceState& state,
				const crypto::KeyPair& encryptionKeyPair) {
			auto blockGeneratorAccountDescriptor = CreateBlockGeneratorAccountDescriptor(config);
			auto harvesterSigningPublicKey = blockGeneratorAccountDescriptor.signingKeyPair().publicKey();

			auto pUnlockedAccounts = CreateUnlockedAccounts(config, state.cache(), std::move(blockGeneratorAccountDescriptor));

			auto pUnlockedAccountsUpdater = std::make_shared<UnlockedAccountsUpdater>(
					state.cache(),
					*pUnlockedAccounts,
					harvesterSigningPublicKey,
					encryptionKeyPair,
					config::CatapultDataDirectory(state.config().User.DataDirectory));
			pUnlockedAccountsUpdater->load();

			return { pUnlockedAccounts, pUnlockedAccountsUpdater };
		}

		// endregion

		// region harvesting task

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

		thread::Task CreateHarvestingTask(
				extensions::ServiceState& state,
				const UnlockedAccountsHolder& unlockedAccountsHolder,
				const Address& beneficiaryAddress) {
			auto strategy = state.config().Node.TransactionSelectionStrategy;
			const auto& transactionRegistry = state.pluginManager().transactionRegistry();
			const auto& utCache = const_cast<const extensions::ServiceState&>(state).utCache();

			const auto& cache = state.cache();
			const auto& blockChainConfig = state.config().BlockChain;
			auto executionConfig = extensions::CreateExecutionConfiguration(state.pluginManager());
			HarvestingUtFacadeFactory utFacadeFactory(cache, blockChainConfig, executionConfig, [&storage = state.storage()](auto height) {
				auto hashRange = storage.view().loadHashesFrom(height, 1);
				return *hashRange.cbegin();
			});

			auto pUnlockedAccounts = unlockedAccountsHolder.pUnlockedAccounts;
			auto blockGenerator = CreateHarvesterBlockGenerator(strategy, transactionRegistry, utFacadeFactory, utCache);
			auto pHarvesterTask = std::make_shared<ScheduledHarvesterTask>(
					CreateHarvesterTaskOptions(state),
					std::make_unique<Harvester>(cache, blockChainConfig, beneficiaryAddress, *pUnlockedAccounts, blockGenerator));

			auto pUnlockedAccountsUpdater = unlockedAccountsHolder.pUnlockedAccountsUpdater;
			return thread::CreateNamedTask("harvesting task", [pUnlockedAccountsUpdater, pHarvesterTask]() {
				pUnlockedAccountsUpdater->update();

				// harvest the next block
				pHarvesterTask->harvest();
				return thread::make_ready_future(thread::TaskResult::Continue);
			});
		}

		// endregion

		// region diagnostic handler

		bool IsDiagnosticExtensionEnabled(const config::ExtensionsConfiguration& extensionsConfiguration) {
			const auto& names = extensionsConfiguration.Names;
			return names.cend() != std::find(names.cbegin(), names.cend(), "extension.diagnostics");
		}

		void RegisterDiagnosticUnlockedAccountsHandler(extensions::ServiceState& state, const UnlockedAccounts& unlockedAccounts) {
			auto& handlers = state.packetHandlers();
			handlers.setAllowedHosts(state.config().Node.TrustedHosts);

			handlers.registerHandler(ionet::PacketType::Unlocked_Accounts, [&unlockedAccounts](const auto& packet, auto& context) {
				if (!ionet::IsPacketValid(packet, ionet::PacketType::Unlocked_Accounts))
					return;

				auto view = unlockedAccounts.view();
				std::vector<Key> harvesterPublicKeys;
				view.forEach([&harvesterPublicKeys](const auto& descriptor) {
					harvesterPublicKeys.push_back(descriptor.signingKeyPair().publicKey());
					return true;
				});

				const auto* pHarvesterPublicKeys = reinterpret_cast<const uint8_t*>(harvesterPublicKeys.data());
				context.response(ionet::PacketPayloadFactory::FromFixedSizeRange(
						ionet::PacketType::Unlocked_Accounts,
						model::EntityRange<Key>::CopyFixed(pHarvesterPublicKeys, harvesterPublicKeys.size())));
			});

			handlers.setAllowedHosts({});
		}

		// endregion

		class HarvestingServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			explicit HarvestingServiceRegistrar(const HarvestingConfiguration& config) : m_config(config)
			{}

			extensions::ServiceRegistrarInfo info() const override {
				return { "Harvesting", extensions::ServiceRegistrarPhase::Post_Range_Consumers };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				locator.registerServiceCounter<UnlockedAccounts>("unlockedAccounts", "UNLKED ACCTS", [](const auto& accounts) {
					return accounts.view().size();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				// updater is owned by harvesting task
				auto unlockedAccountsHolder = CreateUnlockedAccountsHolder(m_config, state, locator.keys().nodeKeyPair());
				locator.registerRootedService("unlockedAccounts", unlockedAccountsHolder.pUnlockedAccounts);

				// add tasks
				state.tasks().push_back(CreateHarvestingTask(state, unlockedAccountsHolder, m_config.BeneficiaryAddress));

				if (IsDiagnosticExtensionEnabled(state.config().Extensions))
					RegisterDiagnosticUnlockedAccountsHandler(state, *unlockedAccountsHolder.pUnlockedAccounts);
			}

		private:
			HarvestingConfiguration m_config;
		};
	}

	DECLARE_SERVICE_REGISTRAR(Harvesting)(const HarvestingConfiguration& config) {
		return std::make_unique<HarvestingServiceRegistrar>(config);
	}
}}
