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
		/// Creates a checker around \a networkFingerprint.
		explicit NodePingResponseCompatibilityChecker(const model::UniqueNetworkFingerprint& networkFingerprint)
				: m_networkFingerprint(networkFingerprint)
		{}

	public:
		/// Returns \c true if \a requestNode and \a responseNode are compatible nodes.
		bool isResponseCompatible(const ionet::Node& requestNode, const ionet::Node& responseNode) const {
			if (IsNodeCompatible(responseNode, m_networkFingerprint, requestNode.identity().PublicKey))
				return true;

			CATAPULT_LOG(warning) << "rejecting incompatible partner node '" << responseNode << "'";
			return false;
		}

	private:
		model::UniqueNetworkFingerprint m_networkFingerprint;
	};

	/// Node ping request policy.
	struct NodePingRequestPolicy {
		using ResponseType = ionet::Node;

		static constexpr auto Friendly_Name = "ping";

		static thread::future<ResponseType> CreateFuture(ionet::PacketIo& packetIo, const std::string& host) {
			return api::CreateRemoteNodeApi(packetIo)->nodeInfo().then([host](auto&& nodeFuture) {
				const auto& node = nodeFuture.get();
				return ionet::Node({ node.identity().PublicKey, host }, node.endpoint(), node.metadata());
			});
		}
	};

	/// Brief server requestor for requesting node ping information.
	using NodePingRequestor = net::BriefServerRequestor<NodePingRequestPolicy, NodePingResponseCompatibilityChecker>;

	/// Creates a node ping requestor for a server with specified \a serverPublicKey and a network identified by \a networkFingerprint
	/// using \a pool and configured with \a settings.
	std::shared_ptr<NodePingRequestor> CreateNodePingRequestor(
			thread::IoThreadPool& pool,
			const Key& serverPublicKey,
			const net::ConnectionSettings& settings,
			const model::UniqueNetworkFingerprint& networkFingerprint);
}}
