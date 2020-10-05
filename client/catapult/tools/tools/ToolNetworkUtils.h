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
#include "catapult/net/ConnectionSettings.h"
#include "catapult/thread/Future.h"
#include "catapult/types.h"

namespace catapult {
	namespace ionet {
		class Node;
		class PacketIo;
		class PacketSocketInfo;
	}
	namespace thread { class IoThreadPool; }
}

namespace catapult { namespace tools {

	/// Future that returns a packet io shared pointer.
	using PacketIoFuture = thread::future<std::shared_ptr<ionet::PacketIo>>;

	/// Future that returns a packet socket info.
	using PacketSocketInfoFuture = thread::future<ionet::PacketSocketInfo>;

	/// Connects to \a node as a client with certificates in \a certificateDirectory using \a pool.
	PacketIoFuture ConnectToNode(const std::string& certificateDirectory, const ionet::Node& node, thread::IoThreadPool& pool);

	/// Connects to \a node as a client with \a connectionSettings using \a pool.
	PacketIoFuture ConnectToNode(const net::ConnectionSettings& connectionSettings, const ionet::Node& node, thread::IoThreadPool& pool);

	/// Creates tool connection settings around certificates in \a certificateDirectory.
	net::ConnectionSettings CreateToolConnectionSettings(const std::string& certificateDirectory);

	/// Helper class for connecting to multiple nodes.
	class MultiNodeConnector {
	public:
		/// Creates a connector around certificates in \a certificateDirectory.
		explicit MultiNodeConnector(const std::string& certificateDirectory);

		/// Destroys the connector.
		~MultiNodeConnector();

	public:
		/// Gets the underlying pool used by the connector.
		thread::IoThreadPool& pool();

	public:
		/// Connects to \a node.
		PacketSocketInfoFuture connect(const ionet::Node& node);

	private:
		std::string m_certificateDirectory;
		std::unique_ptr<thread::IoThreadPool> m_pPool;
	};
}}
