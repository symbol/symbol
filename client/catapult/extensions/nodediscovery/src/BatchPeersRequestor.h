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
#include "catapult/ionet/NodeInteractionResult.h"
#include "catapult/ionet/NodeSet.h"
#include "catapult/net/PacketIoPickerContainer.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace nodediscovery {

	/// Creates a batch peers requestor.
	class BatchPeersRequestor {
	private:
		using NodesConsumer = consumer<const ionet::NodeSet&>;
		using RemoteApiResults = std::vector<ionet::NodeInteractionResult>;

	public:
		/// Creates a requestor around \a packetIoPickers, which is used to find partners. Forwards found nodes to \a nodesConsumer.
		BatchPeersRequestor(const net::PacketIoPickerContainer& packetIoPickers, const NodesConsumer& nodesConsumer);

	public:
		/// Finds and forwards peers of peers within the specified \a timeout.
		thread::future<RemoteApiResults> findPeersOfPeers(const utils::TimeSpan& timeout) const;

	private:
		net::PacketIoPickerContainer m_packetIoPickers; // held by value because packet io pickers is tied to ServiceState
		NodesConsumer m_nodesConsumer;
	};
}}
