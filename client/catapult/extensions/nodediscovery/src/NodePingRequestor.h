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
#include "NodePingUtils.h"
#include "nodediscovery/src/api/RemoteNodeApi.h"
#include "catapult/net/BriefServerRequestor.h"

namespace catapult { namespace nodediscovery {

	/// Node ping response compatibility checker.
	class NodePingResponseCompatibilityChecker {
	public:
		/// Creates a checker around \a networkIdentifier.
		explicit NodePingResponseCompatibilityChecker(model::NetworkIdentifier networkIdentifier)
				: m_networkIdentifier(networkIdentifier)
		{}

	public:
		/// Returns \a true if \a requestNode and \a responseNode are compatible nodes.
		bool isResponseCompatible(const ionet::Node& requestNode, const ionet::Node& responseNode) const {
			if (IsNodeCompatible(responseNode, m_networkIdentifier, requestNode.identityKey()))
				return true;

			CATAPULT_LOG(warning) << "rejecting incompatible partner node '" << responseNode << "'";
			return false;
		}

	private:
		model::NetworkIdentifier m_networkIdentifier;
	};

	/// Node ping request policy.
	class NodePingRequestPolicy {
	public:
		using ResponseType = ionet::Node;

	public:
		static constexpr const char* FriendlyName() {
			return "ping";
		}

		static thread::future<ResponseType> CreateFuture(ionet::PacketIo& packetIo) {
			return api::CreateRemoteNodeApi(packetIo)->nodeInfo();
		}
	};

	/// A brief server requestor for requesting node ping information.
	using NodePingRequestor = net::BriefServerRequestor<NodePingRequestPolicy, NodePingResponseCompatibilityChecker>;

	/// Creates a node ping requestor for a server with a key pair of \a keyPair and a network identified by \a networkIdentifier
	/// using \a pPool and configured with \a settings.
	std::shared_ptr<NodePingRequestor> CreateNodePingRequestor(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const crypto::KeyPair& keyPair,
			const net::ConnectionSettings& settings,
			model::NetworkIdentifier networkIdentifier);
}}
