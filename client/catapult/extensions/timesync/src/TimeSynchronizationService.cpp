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

#include "TimeSynchronizationService.h"
#include "NodeNetworkTimeRequestor.h"
#include "TimeSynchronizationConfiguration.h"
#include "TimeSynchronizationState.h"
#include "TimeSynchronizationUtils.h"
#include "TimeSynchronizer.h"
#include "timesync/src/handlers/TimeSyncHandlers.h"
#include "catapult/config/CatapultKeys.h"
#include "catapult/extensions/NetworkUtils.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/utils/NetworkTime.h"

namespace catapult { namespace timesync {

	namespace {
		constexpr auto Service_Group = "timesync";
		constexpr auto Requestor_Service_Name = "timesync.requestor";
		constexpr auto Synchronizer_Service_Name = "timesync.synchronizer";
		constexpr auto State_Service_Name = "timesync.state";

		using NetworkTimeSupplier = extensions::ExtensionManager::NetworkTimeSupplier;

		class TimeSynchronizationServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			TimeSynchronizationServiceRegistrar(
					const TimeSynchronizationConfiguration& timeSyncConfig,
					const std::shared_ptr<TimeSynchronizationState>& pTimeSyncState)
					: m_timeSyncConfig(timeSyncConfig)
					, m_pTimeSyncState(pTimeSyncState)
			{}

		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "TimeSynchronization", extensions::ServiceRegistrarPhase::Post_Packet_Io_Pickers };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				auto addStateCounter = [&locator](const auto& counterName, auto supplier) {
					locator.registerServiceCounter<TimeSynchronizationState>(State_Service_Name, counterName, supplier);
				};
				addStateCounter("TS OFFSET ABS", [](const auto& state) { return state.absoluteOffset(); });
				addStateCounter("TS OFFSET DIR", [](const auto& state) { return utils::to_underlying_type(state.offsetDirection()); });
				addStateCounter("TS NODE AGE", [](const auto& state) { return static_cast<uint64_t>(state.nodeAge().unwrap()); });

				auto addRequestorCounter = [&locator](const auto& counterName, auto supplier) {
					locator.registerServiceCounter<NodeNetworkTimeRequestor>(Requestor_Service_Name, counterName, supplier);
				};
				addRequestorCounter("TS TOTAL REQ", [](const auto& requestor) { return requestor.numTotalRequests();});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				// create filters
				std::vector<filters::SynchronizationFilter> filters{
					filters::CreateReversedTimestampsFilter(),
					filters::CreateResponseDelayDetectionFilter(),
					filters::CreateClampingFilter()
				};

				// register services
				auto connectionSettings = extensions::GetConnectionSettings(state.config());
				auto pServiceGroup = state.pool().pushServiceGroup(Service_Group);
				auto pNodeNetworkTimeRequestor = pServiceGroup->pushService(
						CreateNodeNetworkTimeRequestor,
						locator.keys().caPublicKey(),
						connectionSettings);

				locator.registerService(Requestor_Service_Name, pNodeNetworkTimeRequestor);

				auto pTimeSynchronizer = std::make_shared<TimeSynchronizer>(
						filters::AggregateSynchronizationFilter(filters),
						state.config().BlockChain.TotalChainImportance);
				locator.registerRootedService(Synchronizer_Service_Name, pTimeSynchronizer);

				locator.registerRootedService(State_Service_Name, m_pTimeSyncState);

				// set handler
				handlers::RegisterTimeSyncNetworkTimeHandler(state.packetHandlers(), state.timeSupplier());

				// add task
				auto sampleSupplier = [&requestor = *pNodeNetworkTimeRequestor](const auto& node) {
					return net::BeginRequestFuture(requestor, node);
				};
				state.tasks().push_back(CreateTimeSyncTask(
						*pTimeSynchronizer,
						m_timeSyncConfig,
						sampleSupplier,
						state,
						*m_pTimeSyncState,
						state.timeSupplier()));
			}

		private:
			TimeSynchronizationConfiguration m_timeSyncConfig;
			std::shared_ptr<TimeSynchronizationState> m_pTimeSyncState;
		};
	}

	DECLARE_SERVICE_REGISTRAR(TimeSynchronization)(
			const TimeSynchronizationConfiguration& timeSyncConfig,
			const std::shared_ptr<TimeSynchronizationState>& pTimeSyncState) {
		return std::make_unique<TimeSynchronizationServiceRegistrar>(timeSyncConfig, pTimeSyncState);
	}
}}
