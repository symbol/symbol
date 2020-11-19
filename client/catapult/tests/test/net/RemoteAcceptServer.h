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
#include "SocketTestUtils.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/thread/IoThreadPool.h"
#include "tests/test/nodeps/Filesystem.h"

namespace catapult { namespace test {

	/// Remote server that accepts client connections.
	class RemoteAcceptServer {
	public:
		/// Creates a remote accept server.
		RemoteAcceptServer();

		/// Creates a remote accept server with specified \a caKeyPair.
		explicit RemoteAcceptServer(const crypto::KeyPair& caKeyPair);

	public:
		/// Gets the underlying io context.
		boost::asio::io_context& ioContext();

		/// Gets the server CA public key.
		const Key& caPublicKey() const;

	public:
		/// Starts the server so that it can accept a single connection.
		void start();

		/// Starts the server so that it can accept a single connection and execute \a serverWork.
		void start(const PacketSocketWork& serverWork);

		/// Starts the server so that it can accept a single connection with \a acceptor and execute \a serverWork.
		void start(const TcpAcceptor& acceptor, const PacketSocketWork& serverWork);

		/// Waits for the server to complete.
		void join();

	private:
		TempDirectoryGuard m_directoryGuard;
		std::unique_ptr<thread::IoThreadPool> m_pPool;
		crypto::KeyPair m_caKeyPair;
		crypto::KeyPair m_nodeKeyPair;
	};
}}
