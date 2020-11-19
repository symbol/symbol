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

#include "UnbondedPruningService.h"
#include "HashLockUtils.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace unbondedpruning {

	namespace {
		class UnbondedPruningServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				// needs to be registered after PtBootstrapperService (addTransactionEventHandler | Initial)
				// needs to be registered before DispatcherService (transactionsChangeHandler | Post_Remote_Peers)
				return { "UnbondedPruning", extensions::ServiceRegistrarPhase::Post_Transaction_Event_Handlers };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator&, extensions::ServiceState& state) override {
				auto eventHandler = state.hooks().transactionEventHandler();
				auto pNotificationPublisher = utils::UniqueToShared(
						state.pluginManager().createNotificationPublisher(model::PublicationMode::Custom));

				state.hooks().addTransactionsChangeHandler([eventHandler, pNotificationPublisher](const auto& changeInfo) {
					for (const auto& transactionInfo : changeInfo.RevertedTransactionInfos) {
						auto dependentHashes = FindDependentTransactionHashes(transactionInfo, *pNotificationPublisher);
						for (const auto& hash : dependentHashes)
							eventHandler({ hash, extensions::TransactionEvent::Dependency_Removed });
					}
				});
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(UnbondedPruning)() {
		return std::make_unique<UnbondedPruningServiceRegistrar>();
	}
}}
