#include "PtSyncSourceService.h"
#include "PtBootstrapperService.h"
#include "partialtransaction/src/handlers/CosignatureHandler.h"
#include "partialtransaction/src/handlers/PtHandlers.h"
#include "catapult/cache/MemoryPtCache.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace partialtransaction {

	namespace {
		class PtSyncSourceServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "PtSyncSource", extensions::ServiceRegistrarPhase::Post_Extended_Range_Consumers };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				const auto& ptCache = GetMemoryPtCache(locator);
				const auto& hooks = GetPtServerHooks(locator);

				// register handlers
				handlers::RegisterPushPartialTransactionsHandler(
						state.packetHandlers(),
						state.pluginManager().transactionRegistry(),
						hooks.ptRangeConsumer());

				handlers::RegisterPullPartialTransactionInfosHandler(state.packetHandlers(), [&ptCache](const auto& shortHashPairs) {
					return ptCache.view().unknownTransactions(shortHashPairs);
				});

				handlers::RegisterPushCosignaturesHandler(state.packetHandlers(), hooks.cosignatureRangeConsumer());
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(PtSyncSource)() {
		return std::make_unique<PtSyncSourceServiceRegistrar>();
	}
}}
