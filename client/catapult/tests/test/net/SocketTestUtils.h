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
#include "catapult/ionet/IoTypes.h"
#include "catapult/ionet/PacketSocketOptions.h"
#include "catapult/ionet/SocketOperationCode.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"
#include <functional>
#include <thread>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace ionet {
		class PacketIo;
		class PacketSocket;
	}
	namespace net { struct ConnectionSettings; }
	namespace thread { class IoThreadPool; }
}

namespace catapult { namespace test {

	/// Function representing transforming a PacketIo into a different implementation.
	using PacketIoTransform = std::function<std::shared_ptr<ionet::PacketIo> (const std::shared_ptr<ionet::PacketSocket>&)>;

	/// Function representing custom work that a socket should perform using a packet aware socket.
	using PacketSocketWork = consumer<const std::shared_ptr<ionet::PacketSocket>&>;

	// region TcpAcceptor

	/// Local host tcp acceptor facade with timeout.
	class TcpAcceptor : public utils::MoveOnly {
	public:
		/// Creates an acceptor around \a ioContext.
		explicit TcpAcceptor(boost::asio::io_context& ioContext);

		/// Creates an acceptor around \a ioContext and \a port.
		TcpAcceptor(boost::asio::io_context& ioContext, unsigned short port);

		/// Destroys the acceptor.
		~TcpAcceptor();

	public:
		/// Gets the underlying acceptor.
		boost::asio::ip::tcp::acceptor& get() const;

		/// Gets a strand that should be used when calling the acceptor.
		boost::asio::io_context::strand& strand() const;

	private:
		class Impl;
		std::shared_ptr<Impl> m_pImpl;

	private:
		friend void SpawnPacketServerWork(
				const TcpAcceptor& acceptor,
				const ionet::PacketSocketOptions& options,
				const PacketSocketWork& serverWork);
	};

	// endregion

	// region factories

	/// Creates a local host endpoint with a default port.
	boost::asio::ip::tcp::endpoint CreateLocalHostEndpoint();

	/// Creates a local host endpoint with the specified \a port.
	boost::asio::ip::tcp::endpoint CreateLocalHostEndpoint(unsigned short port);

	/// Creates a default PacketSocketSslOptions.
	ionet::PacketSocketSslOptions CreatePacketSocketSslOptions();

	/// Creates a default PacketSocketOptions.
	ionet::PacketSocketOptions CreatePacketSocketOptions();

	/// Creates a default ConnectionSettings.
	net::ConnectionSettings CreateConnectionSettings();

	/// Creates an implicitly closed local host acceptor around \a service.
	/// \note This acceptor can only be used in tests where it is implicitly closed by stopping \a ioContext.
	std::shared_ptr<boost::asio::ip::tcp::acceptor> CreateImplicitlyClosedLocalHostAcceptor(boost::asio::io_context& ioContext);

	// endregion

	// region spawn work helpers

	/// Spawns custom server work on \a ioContext by passing an accepted socket to \a serverWork.
	void SpawnPacketServerWork(boost::asio::io_context& ioContext, const PacketSocketWork& serverWork);

	/// Spawns custom server work on \a ioContext by passing an accepted socket to \a serverWork with custom \a options.
	void SpawnPacketServerWork(
			boost::asio::io_context& ioContext,
			const ionet::PacketSocketOptions& options,
			const PacketSocketWork& serverWork);

	/// Spawns custom server work using \a acceptor by passing an accepted socket to \a serverWork.
	void SpawnPacketServerWork(const TcpAcceptor& acceptor, const PacketSocketWork& serverWork);

	/// Spawns custom server work using \a acceptor by passing an accepted socket to \a serverWork with custom \a options.
	void SpawnPacketServerWork(const TcpAcceptor& acceptor, const ionet::PacketSocketOptions& options, const PacketSocketWork& serverWork);

	/// Spawns custom client work on \a ioContext by passing an accepted socket to \a clientWork.
	void SpawnPacketClientWork(boost::asio::io_context& ioContext, const PacketSocketWork& clientWork);

	// endregion

	// region packet socket utils

	/// Gets a value indicating whether or not \a socket is open.
	bool IsSocketOpen(ionet::PacketSocket& socket);

	/// Waits for \a socket to be closed.
	void WaitForClosedSocket(ionet::PacketSocket& socket);

	// endregion

	// region write tests

	/// Asserts that the PacketIo returned by \a transform can write multiple consecutive payloads.
	void AssertWriteCanWriteMultipleConsecutivePayloads(const PacketIoTransform& transform);

	/// Asserts that the PacketIo returned by \a transform can write multiple simultaneous payloads.
	void AssertWriteCanWriteMultipleSimultaneousPayloadsWithoutInterleaving(const PacketIoTransform& transform);

	/// Asserts that \a writeCode indicates the socket was closed during write.
	void AssertSocketClosedDuringWrite(ionet::SocketOperationCode writeCode);

	// endregion

	// region read tests

	/// Asserts that the PacketIo returned by \a transform can read multiple consecutive payloads.
	void AssertReadCanReadMultipleConsecutivePayloads(const PacketIoTransform& transform);

	/// Asserts that the PacketIo returned by \a transform can read multiple simultaneous payloads.
	void AssertReadCanReadMultipleSimultaneousPayloadsWithoutInterleaving(const PacketIoTransform& transform);

	/// Asserts that \a readCode indicates the socket was closed during read.
	void AssertSocketClosedDuringRead(ionet::SocketOperationCode readCode);

	// endregion
}}
