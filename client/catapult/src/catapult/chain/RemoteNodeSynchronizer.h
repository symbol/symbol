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
#include "catapult/ionet/NodeInteractionResultCode.h"
#include "catapult/thread/FutureUtils.h"
#include <functional>

namespace catapult { namespace chain {

	/// Function signature for synchronizing with a remote node.
	template<typename TRemoteApi>
	using RemoteNodeSynchronizer = std::function<thread::future<ionet::NodeInteractionResultCode> (const TRemoteApi&)>;

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

	/// Creates a conditional remote node synchronizer around \a pSynchronizer that only executes when \a shouldExecute returns \c true.
	template<typename TSynchronizer>
	RemoteNodeSynchronizer<typename TSynchronizer::RemoteApiType> CreateConditionalRemoteNodeSynchronizer(
			const std::shared_ptr<TSynchronizer>& pSynchronizer,
			const predicate<>& shouldExecute) {
		return [pSynchronizer, shouldExecute](const auto& remoteApi) {
			if (!shouldExecute())
				return thread::make_ready_future(ionet::NodeInteractionResultCode::Neutral);

			// pSynchronizer is captured in the second lambda to compose, which extends its lifetime until
			// the async operation is complete
			return thread::compose(pSynchronizer->operator()(remoteApi), [pSynchronizer](auto&& future) {
				return std::move(future);
			});
		};
	}
}}
