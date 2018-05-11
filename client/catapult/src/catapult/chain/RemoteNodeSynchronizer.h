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
