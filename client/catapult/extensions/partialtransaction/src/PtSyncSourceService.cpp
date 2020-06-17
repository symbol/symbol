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

#include "PtSyncSourceService.h"
#include "PtBootstrapperService.h"
#include "partialtransaction/src/handlers/CosignatureHandlers.h"
#include "partialtransaction/src/handlers/PtHandlers.h"
#include "catapult/cache_tx/MemoryPtCache.h"
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
