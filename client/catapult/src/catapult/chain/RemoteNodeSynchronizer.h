#pragma once
#include "NodeInteractionResult.h"
#include "catapult/thread/FutureUtils.h"
#include <functional>

namespace catapult { namespace chain {

	/// Function signature for synchronizing with a remote node.
	template<typename TRemoteApi>
	using RemoteNodeSynchronizer = std::function<thread::future<NodeInteractionResult> (const TRemoteApi&)>;

	/// Creates a remote node synchronizer around \a pSynchronizer.
	template<typename TSynchronizer>
	RemoteNodeSynchronizer<typename TSynchronizer::RemoteApiType> CreateRemoteNodeSynchronizer(
			const std::shared_ptr<TSynchronizer>& pSynchronizer) {
		return [pSynchronizer](const auto& remoteApi) {
			// pSynchronizer is captured in the second lambda to compose, which extends its lifetime until
			// the async operation is complete
			return thread::compose(pSynchronizer->operator()(remoteApi), [pSynchronizer](auto&& future) {
				return std::move(future);
			});
		};
	}
}}
