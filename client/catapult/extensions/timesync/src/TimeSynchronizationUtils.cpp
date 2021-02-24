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

#include "TimeSynchronizationUtils.h"
#include "ImportanceAwareNodeSelector.h"
#include "TimeSynchronizationConfiguration.h"
#include "TimeSynchronizationState.h"
#include "TimeSynchronizer.h"
#include "constants.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/ImportanceView.h"
#include "catapult/config/CatapultConfiguration.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/ionet/NodeContainer.h"

namespace catapult { namespace timesync {

	namespace {
		using NetworkTimeSupplier = extensions::ExtensionManager::NetworkTimeSupplier;

		ImportanceAwareNodeSelector CreateImportanceAwareNodeSelector(const TimeSynchronizationConfiguration& timeSyncConfig) {
			auto serviceId = ionet::ServiceIdentifier(0x53594E43);
			return ImportanceAwareNodeSelector(serviceId, timeSyncConfig.MaxNodes, timeSyncConfig.MinImportance);
		}

		struct SamplesResult {
		public:
			explicit SamplesResult(size_t count)
					: Samples(count)
					, NumValidSamples(0)
			{}

		public:
			std::vector<TimeSynchronizationSample> Samples;
			std::atomic<size_t> NumValidSamples;
		};

		ionet::NodeSet SelectNodes(
				const cache::AccountStateCache& cache,
				const ImportanceAwareNodeSelector& selector,
				const ionet::NodeContainer& nodes,
				Height height) {
			// to prevent deadlock with node selection, nodes lock must be acquired before cache lock
			auto nodesView = nodes.view();
			auto cacheView = cache.createView();
			cache::ImportanceView importanceView(cacheView->asReadOnly());
			auto selectedNodes = selector.selectNodes(importanceView, nodesView, height);
			CATAPULT_LOG(debug) << "timesync: number of selected nodes: " << selectedNodes.size();
			return selectedNodes;
		}
	}

	thread::future<TimeSynchronizationSamples> RetrieveSamples(
			const ionet::NodeSet& nodes,
			const TimeSyncResultSupplier& requestResultFutureSupplier,
			const NetworkTimeSupplier& networkTimeSupplier) {
		if (nodes.empty())
			return thread::make_ready_future<TimeSynchronizationSamples>(TimeSynchronizationSamples());

		auto pSamplesResult = std::make_shared<SamplesResult>(nodes.size());
		std::vector<thread::future<bool>> futures;
		for (const auto& node : nodes) {
			auto pLocalTimestamps = std::make_shared<CommunicationTimestamps>();
			pLocalTimestamps->SendTimestamp = networkTimeSupplier();
			auto future = requestResultFutureSupplier(node)
				.then([pSamplesResult, node, pLocalTimestamps, networkTimeSupplier](auto&& resultFuture) {
					pLocalTimestamps->ReceiveTimestamp = networkTimeSupplier();
					auto pair = resultFuture.get();
					auto& samples = pSamplesResult->Samples;
					if (net::NodeRequestResult::Success == pair.first) {
						auto index = (pSamplesResult->NumValidSamples)++;
						samples[index] = TimeSynchronizationSample(node.identity().PublicKey, *pLocalTimestamps, pair.second);
						CATAPULT_LOG(info) << "'" << node << "': time offset is " << samples[index].timeOffsetToRemote();
						return true;
					}

					CATAPULT_LOG(warning)
							<< "unable to retrieve network time from node '" << node
							<< "', request result: " << pair.first;
					return false;
				});
			futures.push_back(std::move(future));
		}

		return thread::when_all(std::move(futures)).then([pSamplesResult](auto&&) {
			auto& samples = pSamplesResult->Samples;
			samples.resize(pSamplesResult->NumValidSamples);
			return TimeSynchronizationSamples(samples.cbegin(), samples.cend());
		});
	}

	thread::Task CreateTimeSyncTask(
			TimeSynchronizer& timeSynchronizer,
			const TimeSynchronizationConfiguration& timeSyncConfig,
			const TimeSyncResultSupplier& resultSupplier,
			const extensions::ServiceState& state,
			TimeSynchronizationState& timeSyncState,
			const NetworkTimeSupplier& networkTimeSupplier) {
		const auto& cache = state.cache().sub<cache::AccountStateCache>();
		const auto& nodes = state.nodes();
		const auto& storage = state.storage();
		auto selector = CreateImportanceAwareNodeSelector(timeSyncConfig);
		return thread::CreateNamedTask("time synchronization task", [&, resultSupplier, networkTimeSupplier, selector]() {
			auto height = storage.view().chainHeight();

			// select nodes
			auto selectedNodes = SelectNodes(cache, selector, nodes, height);

			// retrieve samples from selected nodes
			auto samplesFuture = RetrieveSamples(selectedNodes, resultSupplier, networkTimeSupplier);
			return samplesFuture.then([&timeSynchronizer, &timeSyncState, &cache, height](auto&& future) {
				auto samples = future.get();
				CATAPULT_LOG(debug) << "timesync: number of retrieved samples: " << samples.size();

				// calculate new offset and update state
				auto view = cache.createView();
				auto offset = timeSynchronizer.calculateTimeOffset(*view, height, std::move(samples), timeSyncState.nodeAge());
				timeSyncState.update(offset);

				return thread::TaskResult::Continue;
			});
		});
	}
}}
