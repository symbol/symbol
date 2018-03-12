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

	/// A local host tcp acceptor facade with timeout.
	class TcpAcceptor {
	public:
		/// Creates an acceptor around \a service.
		explicit TcpAcceptor(boost::asio::io_service& service);

		/// Destroys the acceptor.
		~TcpAcceptor();

	public:
		/// Gets the underlying acceptor.
		boost::asio::ip::tcp::acceptor& get() const;

		/// Gets a strand that should be used when calling the acceptor.
		boost::asio::strand& strand() const;

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};

	/// Creates an implicitly closed local host acceptor around \a service.
	/// \note This acceptor can only be used in tests where it is implicitly closed by stopping \a service.
	std::shared_ptr<boost::asio::ip::tcp::acceptor> CreateImplicitlyClosedLocalHostAcceptor(boost::asio::io_service& service);

	/// Function representing custom work that a socket should perform using a packet aware socket.
	using PacketSocketWork = consumer<const std::shared_ptr<ionet::PacketSocket>&>;

	/// Spawns custom server work on \a service by passing an accepted socket to \a serverWork.
	void SpawnPacketServerWork(boost::asio::io_service& service, const PacketSocketWork& serverWork);

	/// Spawns custom server work using \a acceptor by passing an accepted socket to \a serverWork.
	void SpawnPacketServerWork(const TcpAcceptor& acceptor, const PacketSocketWork& serverWork);

	/// Spawns custom client work on \a service by passing an accepted socket to \a clientWork.
	void SpawnPacketClientWork(boost::asio::io_service& service, const PacketSocketWork& clientWork);

	/// Gets a value indicating whether or not \a socket is open.
	bool IsSocketOpen(ionet::PacketSocket& socket);

	/// Function representing transforming a PacketIo into a different implementation.
	using PacketIoTransform = std::function<std::shared_ptr<ionet::PacketIo> (const std::shared_ptr<ionet::PacketSocket>&)>;

	/// Asserts that the PacketIo returned by \a transform can write multiple consecutive payloads.
	void AssertWriteCanWriteMultipleConsecutivePayloads(const PacketIoTransform& transform);

	/// Asserts that the PacketIo returned by \a transform can write multiple simultaneous payloads.
	void AssertWriteCanWriteMultipleSimultaneousPayloadsWithoutInterleaving(const PacketIoTransform& transform);

	/// Asserts that the PacketIo returned by \a transform can read multiple consecutive payloads.
	void AssertReadCanReadMultipleConsecutivePayloads(const PacketIoTransform& transform);

	/// Asserts that the PacketIo returned by \a transform can read multiple simultaneous payloads.
	void AssertReadCanReadMultipleSimultaneousPayloadsWithoutInterleaving(const PacketIoTransform& transform);

	/// Asserts that \a readCode indicates the socket was closed during read.
	void AssertSocketClosedDuringRead(ionet::SocketOperationCode readCode);

	/// Asserts that \a writeCode indicates the socket was closed during write.
	void AssertSocketClosedDuringWrite(ionet::SocketOperationCode writeCode);
}}
