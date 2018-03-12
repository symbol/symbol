#include "TransactionSinkService.h"
#include "catapult/extensions/ServerHooksUtils.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/handlers/TransactionHandlers.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace transactionsink {

	namespace {
		class TransactionSinkServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "TransactionSink", extensions::ServiceRegistrarPhase::Post_Range_Consumers };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator&, extensions::ServiceState& state) override {
				// add handlers
				auto pushTransactionsCallback = CreateTransactionPushEntityCallback(state.hooks());
				handlers::RegisterPushTransactionsHandler(
						state.packetHandlers(),
						state.pluginManager().transactionRegistry(),
						pushTransactionsCallback);
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(TransactionSink)() {
		return std::make_unique<TransactionSinkServiceRegistrar>();
	}
}}
