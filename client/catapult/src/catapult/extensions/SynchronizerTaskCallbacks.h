#pragma once
#include "ServiceState.h"
#include "catapult/chain/RemoteApiForwarder.h"
#include "catapult/chain/RemoteNodeSynchronizer.h"
#include "catapult/config/LocalNodeConfiguration.h"
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
		chain:: RemoteApiForwarder forwarder(packetIoPicker, state.pluginManager().transactionRegistry(), syncTimeout, taskName);
		return [forwarder, synchronizer, remoteApiFactory]() {
			return forwarder.processSync(synchronizer, remoteApiFactory).then([](auto&&) { return thread::TaskResult::Continue; });
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
