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

#include "NetworkHeightService.h"
#include "NetworkChainHeight.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/io/BlockStorageCache.h"

namespace catapult { namespace networkheight {

	namespace {
		thread::Task CreateChainHeightDetectionTask(
				const extensions::RemoteChainHeightsRetriever& remoteChainHeightsRetriever,
				NetworkChainHeight& networkChainHeight) {
			static constexpr size_t Num_Peers = 3;
			return thread::CreateNamedTask("network chain height detection", [remoteChainHeightsRetriever, &networkChainHeight]() {
				CATAPULT_LOG(trace) << "starting chain height task";
				return remoteChainHeightsRetriever(Num_Peers).then([&networkChainHeight](auto&& heightsFuture) {
					auto heights = heightsFuture.get();
					if (heights.empty())
						return thread::TaskResult::Continue;

					auto maxHeight = (*std::max_element(heights.cbegin(), heights.cend())).unwrap();
					auto currentHeight = networkChainHeight.load();
					if (currentHeight < maxHeight) {
						networkChainHeight = maxHeight;
						CATAPULT_LOG(debug) << "network chain height increased from " << currentHeight << " to " << maxHeight;
					}

					return thread::TaskResult::Continue;
				});
			});
		}

		class NetworkHeightServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "NetworkHeight", extensions::ServiceRegistrarPhase::Post_Remote_Peers };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				auto pNetworkChainHeight = std::make_shared<NetworkChainHeight>(0);
				locator.registerRootedService("networkChainHeight", pNetworkChainHeight);

				// if the local node's chain is too far behind, some operations like harvesting or
				// processing of pushed elements should not be allowed
				// for a public network this should always return true since an evil node could supply a fake chain height
				state.hooks().setChainSyncedPredicate([&storage = state.storage(), &networkChainHeight = *pNetworkChainHeight]() {
					auto storageView = storage.view();
					return networkChainHeight.load() < storageView.chainHeight().unwrap() + 4;
				});

				// add tasks
				state.tasks().push_back(CreateChainHeightDetectionTask(state.hooks().remoteChainHeightsRetriever(), *pNetworkChainHeight));
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(NetworkHeight)() {
		return std::make_unique<NetworkHeightServiceRegistrar>();
	}
}}
