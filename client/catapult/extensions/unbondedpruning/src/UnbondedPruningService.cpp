#include "UnbondedPruningService.h"
#include "HashLockUtils.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace unbondedpruning {

	namespace {
		std::shared_ptr<model::NotificationPublisher> CreateNotificationPublisher(extensions::ServiceState& state) {
			return model::CreateNotificationPublisher(state.pluginManager().transactionRegistry(), model::PublicationMode::Custom);
		}

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
				auto pNotificationPublisher = CreateNotificationPublisher(state);

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
