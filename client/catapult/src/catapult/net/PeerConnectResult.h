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
#include "PeerConnectCode.h"
#include "catapult/model/NodeIdentity.h"
#include <memory>

namespace catapult { namespace ionet { class PacketSocket; } }

namespace catapult { namespace net {

	/// Peer connection result.
	struct PeerConnectResult {
	public:
		/// Creates a default result.
		PeerConnectResult() : PeerConnectResult(static_cast<PeerConnectCode>(-1))
		{}

		/// Creates a result around \a code.
		PeerConnectResult(PeerConnectCode code) : PeerConnectResult(code, model::NodeIdentity())
		{}

		/// Creates a result around \a code and \a identity.
		PeerConnectResult(PeerConnectCode code, const model::NodeIdentity& identity)
				: Code(code)
				, Identity(PeerConnectCode::Accepted == code ? identity : model::NodeIdentity())
		{}

	public:
		/// Connection result code.
		PeerConnectCode Code;

		/// Connection identity.
		/// \note This is only valid if Code is PeerConnectCode::Accepted.
		model::NodeIdentity Identity;
	};

	/// Peer connection result with socket.
	/// \note This attaches a socket to a PeerConnectResult in order to allow more targeted PacketWriters tests.
	struct PeerConnectResultEx : public PeerConnectResult {
	public:
		/// Creates a default result.
		PeerConnectResultEx() : PeerConnectResult()
		{}

		/// Creates a result around \a code.
		PeerConnectResultEx(PeerConnectCode code) : PeerConnectResult(code)
		{}

		/// Creates a result around \a code, \a identity and \a pSocket.
		PeerConnectResultEx(PeerConnectCode code, const model::NodeIdentity& identity, const std::shared_ptr<ionet::PacketSocket>& pSocket)
				: PeerConnectResult(code, identity)
				, pPeerSocket(PeerConnectCode::Accepted == code ? pSocket : nullptr)
		{}

	public:
		/// Peer socket.
		std::shared_ptr<ionet::PacketSocket> pPeerSocket;
	};
}}
