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

#pragma once
#include "CommunicationTimestamps.h"
#include "TimeSynchronizationSample.h"
#include "catapult/extensions/ExtensionManager.h"
#include "catapult/ionet/NodeSet.h"
#include "catapult/net/NodeRequestResult.h"
#include "catapult/thread/Task.h"

namespace catapult {
	namespace extensions { class ServiceState; }
	namespace timesync {
		struct TimeSynchronizationConfiguration;
		class TimeSynchronizationState;
		class TimeSynchronizer;
	}
}

namespace catapult { namespace timesync {

	/// Time synchronization request result pair.
	using TimeSyncRequestResultPair = std::pair<net::NodeRequestResult, CommunicationTimestamps>;

	/// Prototype for a time synchronization result supplier.
	using TimeSyncResultSupplier = std::function<thread::future<TimeSyncRequestResultPair> (const ionet::Node&)>;

	/// Gets the time synchronization samples derived from communication timestamps retrieved from \a nodes using \a resultSupplier
	/// and \a networkTimeSupplier.
	thread::future<TimeSynchronizationSamples> RetrieveSamples(
			const ionet::NodeSet& nodes,
			const TimeSyncResultSupplier& resultSupplier,
			const extensions::ExtensionManager::NetworkTimeSupplier& networkTimeSupplier);

	/// Creates a time synchronization task around \a timeSynchronizer, \a timeSyncConfig, \a resultSupplier, \a state,
	/// \a timeSyncState and \a networkTimeSupplier.
	thread::Task CreateTimeSyncTask(
			TimeSynchronizer& timeSynchronizer,
			const TimeSynchronizationConfiguration& timeSyncConfig,
			const TimeSyncResultSupplier& resultSupplier,
			const extensions::ServiceState& state,
			TimeSynchronizationState& timeSyncState,
			const extensions::ExtensionManager::NetworkTimeSupplier& networkTimeSupplier);
}}
