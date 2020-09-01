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
#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/src/handlers/FinalizationHandlers.h"
#include "finalization/src/handlers/ProofHandlers.h"
#include "catapult/extensions/ServiceState.h"

namespace catapult { namespace finalization {

	namespace {
		class FinalizationSyncSourceServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			explicit FinalizationSyncSourceServiceRegistrar(bool enableVoting) : m_enableVoting(enableVoting)
			{}

		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "FinalizationSyncSource", extensions::ServiceRegistrarPhase::Post_Extended_Range_Consumers };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				const auto& messageAggregator = GetMultiRoundMessageAggregator(locator);
				const auto& hooks = GetFinalizationServerHooks(locator);
				const auto& proofStorage = GetProofStorageCache(locator);

				// register handlers
				auto& packetHandlers = state.packetHandlers();
				handlers::RegisterFinalizationStatisticsHandler(packetHandlers, proofStorage);
				handlers::RegisterFinalizationProofAtPointHandler(packetHandlers, proofStorage);
				handlers::RegisterFinalizationProofAtHeightHandler(packetHandlers, proofStorage);

				if (m_enableVoting) {
					handlers::RegisterPushMessagesHandler(packetHandlers, hooks.messageRangeConsumer());
					handlers::RegisterPullMessagesHandler(packetHandlers, [&messageAggregator](auto point, const auto& shortHashes) {
						return messageAggregator.view().unknownMessages(point, shortHashes);
					});
				}
			}

		private:
			bool m_enableVoting;
		};
	}

	DECLARE_SERVICE_REGISTRAR(FinalizationSyncSource)(bool enableVoting) {
		return std::make_unique<FinalizationSyncSourceServiceRegistrar>(enableVoting);
	}
}}
