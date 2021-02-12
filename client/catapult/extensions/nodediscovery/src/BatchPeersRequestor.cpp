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

#include "BatchPeersRequestor.h"
#include "nodediscovery/src/api/RemoteNodeApi.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/utils/ThrottleLogger.h"
#include "catapult/utils/TimeSpan.h"

namespace catapult { namespace nodediscovery {

	BatchPeersRequestor::BatchPeersRequestor(const net::PacketIoPickerContainer& packetIoPickers, const NodesConsumer& nodesConsumer)
			: m_packetIoPickers(packetIoPickers)
			, m_nodesConsumer(nodesConsumer)
	{}

	thread::future<BatchPeersRequestor::RemoteApiResults> BatchPeersRequestor::findPeersOfPeers(const utils::TimeSpan& timeout) const {
		auto packetIoPairs = m_packetIoPickers.pickMatching(timeout, ionet::NodeRoles::None);
		if (packetIoPairs.empty()) {
			CATAPULT_LOG_THROTTLE(warning, utils::TimeSpan::FromMinutes(1).millis()) << "no packet io available for requesting peers";
			return thread::make_ready_future(std::vector<ionet::NodeInteractionResult>());
		}

		auto i = 0u;
		std::vector<thread::future<ionet::NodeInteractionResult>> futures(packetIoPairs.size());
		for (const auto& packetIoPair : packetIoPairs) {
			auto peersInfoFuture = api::CreateRemoteNodeApi(*packetIoPair.io())->peersInfo();
			auto identity = packetIoPair.node().identity();
			futures[i++] = peersInfoFuture.then([nodesConsumer = m_nodesConsumer, packetIoPair, identity](auto&& nodesFuture) {
				try {
					auto nodes = nodesFuture.get();
					CATAPULT_LOG(debug) << "partner node " << packetIoPair.node() << " returned " << nodes.size() << " peers";
					nodesConsumer(nodes);
					return ionet::NodeInteractionResult(identity, ionet::NodeInteractionResultCode::Success);
				} catch (const catapult_runtime_error& e) {
					CATAPULT_LOG(warning) << "exception thrown while requesting peers: " << e.what();
					return ionet::NodeInteractionResult(identity, ionet::NodeInteractionResultCode::Failure);
				}
			});
		}

		return thread::when_all(std::move(futures)).then([](auto&& resultsFuture) {
			return thread::get_all(resultsFuture.get());
		});
	}
}}
