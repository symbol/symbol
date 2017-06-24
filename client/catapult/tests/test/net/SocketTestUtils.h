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
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace test {

	/// The endpoint representing the local host.
	extern const boost::asio::ip::tcp::endpoint Local_Host;

	/// Creates a local host endpoint with the specified \a port.
	boost::asio::ip::tcp::endpoint CreateLocalHostEndpoint(unsigned short port);

	/// Creates a default PacketSocketOptions.
	ionet::PacketSocketOptions CreatePacketSocketOptions();

	/// Creates a local host acceptor around \a service.
	std::shared_ptr<boost::asio::ip::tcp::acceptor> CreateLocalHostAcceptor(boost::asio::io_service& service);

	/// Function representing custom work that a socket should perform using a packet aware socket.
	using PacketSocketWork = std::function<void (const std::shared_ptr<ionet::PacketSocket>&)>;

	/// Spawns custom server work on \a service by passing an accepted socket to \a serverWork.
	void SpawnPacketServerWork(boost::asio::io_service& service, const PacketSocketWork& serverWork);

	/// Spawns custom client work on \a service by passing an accepted socket to \a clientWork.
	void SpawnPacketClientWork(boost::asio::io_service& service, const PacketSocketWork& clientWork);

	/// Gets a value indicating whether or not \a socket is open.
	bool IsSocketOpen(ionet::PacketSocket& socket);

	/// Function representing transforming a PacketIo into a different implementation.
	using PacketIoTransform = std::function<std::shared_ptr<ionet::PacketIo> (
			boost::asio::io_service&,
			const std::shared_ptr<ionet::PacketIo>&)>;

	/// Asserts that the PacketIo returned by \a transform can write multiple consecutive payloads.
	void AssertWriteCanWriteMultipleConsecutivePayloads(const PacketIoTransform& transform);

	/// Asserts that the PacketIo returned by \a transform can write multiple simultaneous payloads.
	void AssertWriteCanWriteMultipleSimultaneousPayloadsWithoutInterleaving(const PacketIoTransform& transform);

	/// Asserts that the PacketIo returned by \a transform can read multiple consecutive payloads.
	void AssertReadCanReadMultipleConsecutivePayloads(const PacketIoTransform& transform);

	/// Asserts that the PacketIo returned by \a transform can read multiple simultaneous payloads.
	void AssertReadCanReadMultipleSimultaneousPayloadsWithoutInterleaving(const PacketIoTransform& transform);

	/// Asserts that \a readResult indicates the socket was closed during read.
	void AssertSocketClosedDuringRead(const ionet::SocketOperationCode& readResult);

	/// Asserts that \a writeResult indicates the socket was closed during write.
	void AssertSocketClosedDuringWrite(const ionet::SocketOperationCode& writeResult);
}}
