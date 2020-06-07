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
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/Node.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/net/ConnectionSettings.h"
#include "tests/test/nodeps/Filesystem.h"

namespace catapult {
	namespace api { class RemoteChainApi; }
	namespace ionet {
		class PacketIo;
		class PacketSocket;
	}
	namespace net { class ServerConnector; }
	namespace thread { class IoThreadPool; }
}

namespace catapult { namespace test {

	/// Represents an external source connection.
	class ExternalSourceConnection {
	public:
		/// Creates an external source connection to default local node with public \a key.
		explicit ExternalSourceConnection(const Key& key);

		/// Creates an external source connection to specified local \a node.
		explicit ExternalSourceConnection(const ionet::Node& node);

	public:
		/// Gets the connected io.
		std::shared_ptr<ionet::PacketIo> io() const;

	public:
		/// Connects to the local node and calls \a onConnect with socket on completion.
		void connect(const consumer<const std::shared_ptr<ionet::PacketSocket>&>& onConnect);

		/// Connects to the local node and calls \a onConnect with api on completion.
		void apiCall(const consumer<const std::shared_ptr<api::RemoteChainApi>&>& onConnect);

	private:
		net::ConnectionSettings createConnectionSettings();

	private:
		static model::TransactionRegistry CreateTransactionRegistry();

	private:
		std::unique_ptr<thread::IoThreadPool> m_pPool;
		crypto::KeyPair m_caKeyPair;
		TempDirectoryGuard m_tempDirectoryGuard;
		std::shared_ptr<net::ServerConnector> m_pConnector;
		ionet::Node m_localNode;

		std::shared_ptr<ionet::PacketIo> m_pIo;
	};
}}
