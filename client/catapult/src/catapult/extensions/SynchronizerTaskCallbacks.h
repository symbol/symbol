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
#include "NodeInteractionUtils.h"
#include "ServiceState.h"
#include "catapult/chain/RemoteApiForwarder.h"
#include "catapult/chain/RemoteNodeSynchronizer.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace extensions {

	/// Creates a synchronizer task callback for \a synchronizer named \a taskName that does not require the local chain to be synced.
	/// \a packetIoPicker is used to select peers and \a remoteApiFactory wraps an api around peers.
	/// \a state provides additional service information.
	template<typename TRemoteApi, typename TRemoteApiFactory>
	thread::TaskCallback CreateSynchronizerTaskCallback(
			chain::RemoteNodeSynchronizer<TRemoteApi>&& synchronizer,
			TRemoteApiFactory remoteApiFactory,
			net::PacketIoPicker& packetIoPicker,
			const extensions::ServiceState& state,
			const std::string& taskName) {
		auto syncTimeout = state.config().Node.SyncTimeout;
		chain::RemoteApiForwarder forwarder(packetIoPicker, state.pluginManager().transactionRegistry(), syncTimeout, taskName);

		auto syncHandler = [&nodes = state.nodes()](auto&& future) {
			auto result = future.get();
			IncrementNodeInteraction(nodes, result);
			return thread::TaskResult::Continue;
		};

		return [forwarder, syncHandler, synchronizer, remoteApiFactory]() {
			return forwarder.processSync(synchronizer, remoteApiFactory).then(syncHandler);
		};
	}

	/// Creates a synchronizer task callback for \a synchronizer named \a taskName that requires the local chain to be synced.
	/// \a packetIoPicker is used to select peers and \a remoteApiFactory wraps an api around peers.
	/// \a state provides additional service information.
	template<typename TRemoteApi, typename TRemoteApiFactory>
	thread::TaskCallback CreateChainSyncAwareSynchronizerTaskCallback(
			chain::RemoteNodeSynchronizer<TRemoteApi>&& synchronizer,
			TRemoteApiFactory remoteApiFactory,
			net::PacketIoPicker& packetIoPicker,
			const extensions::ServiceState& state,
			const std::string& taskName) {
		const auto& chainSynced = state.hooks().chainSyncedPredicate();
		auto synchronize = CreateSynchronizerTaskCallback(std::move(synchronizer), remoteApiFactory, packetIoPicker, state, taskName);
		return [chainSynced, synchronize]() {
			if (!chainSynced())
				return thread::make_ready_future(thread::TaskResult::Continue);

			return synchronize();
		};
	}
}}
