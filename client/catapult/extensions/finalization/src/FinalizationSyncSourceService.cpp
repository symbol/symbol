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

#include "FinalizationSyncSourceService.h"
#include "FinalizationBootstrapperService.h"
#include "finalization/src/handlers/FinalizationHandlers.h"
#include "catapult/extensions/ServiceState.h"

namespace catapult { namespace finalization {

	namespace {
		class FinalizationSyncSourceServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "FinalizationSyncSource", extensions::ServiceRegistrarPhase::Post_Extended_Range_Consumers };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				const auto& hooks = GetFinalizationServerHooks(locator);

				// register handlers
				handlers::RegisterPushMessagesHandler(state.packetHandlers(), hooks.messageRangeConsumer());
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(FinalizationSyncSource)() {
		return std::make_unique<FinalizationSyncSourceServiceRegistrar>();
	}
}}
