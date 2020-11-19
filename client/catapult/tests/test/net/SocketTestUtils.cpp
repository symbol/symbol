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

#include "SocketTestUtils.h"
#include "CertificateLocator.h"
#include "ClientSocket.h"
#include "NodeTestUtils.h"
#include "catapult/crypto/Signer.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/ConnectionSettings.h"
#include "catapult/thread/IoThreadPool.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include <boost/asio/ssl.hpp>
#include <boost/asio/steady_timer.hpp>

namespace catapult { namespace test {

	// region TcpAcceptor

	namespace {
		void EnableAddressReuse(boost::asio::ip::tcp::acceptor& acceptor) {
			acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

#ifndef _WIN32
			using reuse_port = boost::asio::detail::socket_option::boolean<BOOST_ASIO_OS_DEF(SOL_SOCKET), SO_REUSEPORT>;
			acceptor.set_option(reuse_port(true));
#endif
		}

		void BindAcceptor(boost::asio::ip::tcp::acceptor& acceptor, const boost::asio::ip::tcp::endpoint& endpoint) {
			acceptor.open(endpoint.protocol());
			EnableAddressReuse(acceptor);
			acceptor.bind(endpoint);
		}
	}

	class TcpAcceptor::Impl : public std::enable_shared_from_this<TcpAcceptor::Impl> {
	public:
		Impl(boost::asio::io_context& ioContext, const boost::asio::ip::tcp::endpoint& endpoint)
				: m_ioContext(ioContext)
				, m_endpoint(endpoint)
				, m_acceptorStrand(m_ioContext)
				, m_timer(m_ioContext)
				, m_isClosed(false)
				, m_pAcceptor(CreateLocalHostAcceptor(m_ioContext, m_endpoint))
		{}

	public:
		auto& ioContext() {
			return m_ioContext;
		}

		auto& acceptor() {
			return *m_pAcceptor;
		}

		auto& strand() {
			return m_acceptorStrand;
		}

		bool isStopped() const {
			return m_isClosed;
		}

	public:
		void init() {
			// setup the timer
			m_timer.expires_from_now(std::chrono::seconds(2 * detail::Default_Wait_Timeout));
			m_timer.async_wait([pThis = shared_from_this()](const auto& ec) {
				if (boost::asio::error::operation_aborted == ec)
					return;

				EXPECT_EQ(boost::asio::error::operation_aborted, ec) << "TcpAcceptor timer fired, forcing close of acceptor";
				pThis->closeAcceptor();
			});
		}

		void destroy() {
			// cancel the timer
			m_timer.cancel();

			// forcibly close the acceptor (if not already closed)
			closeAcceptor();
		}

	private:
		void closeAcceptor() {
			CATAPULT_LOG(debug) << "dispatching close of socket acceptor";
			boost::asio::dispatch(m_acceptorStrand, [pThis = shared_from_this()]() {
				Close(*pThis->m_pAcceptor, pThis->m_isClosed, pThis->m_endpoint.port());
			});
		}

	private:
		static std::unique_ptr<boost::asio::ip::tcp::acceptor> CreateLocalHostAcceptor(
				boost::asio::io_context& ioContext,
				const boost::asio::ip::tcp::endpoint& endpoint) {
			if (GetLocalHostPort() == endpoint.port()) {
				if (Has_Outstanding_Acceptor)
					CATAPULT_THROW_INVALID_ARGUMENT("detected creation of multiple localhost acceptors - probably a bug");

				Has_Outstanding_Acceptor = true;
			}

			auto pAcceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(ioContext);
			BindAcceptor(*pAcceptor, endpoint);
			pAcceptor->listen();
			return pAcceptor;
		}

		static void Close(boost::asio::ip::tcp::acceptor& acceptor, std::atomic_bool& isClosed, unsigned short port) {
			CATAPULT_LOG(debug) << "closing socket acceptor";

			acceptor.close();

			if (GetLocalHostPort() == port)
				Has_Outstanding_Acceptor = false;

			isClosed = true;

			CATAPULT_LOG(debug) << "socket acceptor closed";
		}

	private:
		// on MacOS, there is a potential race condition when kevent (triggered by async_await) and close are called concurrently
		// this is now *properly* mitigated by wrapping acceptor operations in a strand
		boost::asio::io_context& m_ioContext;
		boost::asio::ip::tcp::endpoint m_endpoint;
		boost::asio::io_context::strand m_acceptorStrand;
		boost::asio::steady_timer m_timer;
		std::atomic_bool m_isClosed;
		std::unique_ptr<boost::asio::ip::tcp::acceptor> m_pAcceptor;

	private:
		// use a global to detect (and fail) tests that create multiple acceptors (not foolproof but should detect egregious misuse)
		static bool Has_Outstanding_Acceptor;
	};

	bool TcpAcceptor::Impl::Has_Outstanding_Acceptor = false;

	TcpAcceptor::TcpAcceptor(boost::asio::io_context& ioContext) : TcpAcceptor(ioContext, GetLocalHostPort())
	{}

	TcpAcceptor::TcpAcceptor(boost::asio::io_context& ioContext, unsigned short port)
			: TcpAcceptor(ioContext, CreateLocalHostEndpoint(port))
	{}

	TcpAcceptor::TcpAcceptor(boost::asio::io_context& ioContext, const boost::asio::ip::tcp::endpoint& endpoint)
			: m_pImpl(std::make_shared<Impl>(ioContext, endpoint)) {
		m_pImpl->init();
	}

	TcpAcceptor::~TcpAcceptor() {
		if (!isStopped())
			m_pImpl->destroy();
	}

	boost::asio::ip::tcp::acceptor& TcpAcceptor::get() const {
		return m_pImpl->acceptor();
	}

	boost::asio::io_context::strand& TcpAcceptor::strand() const {
		return m_pImpl->strand();
	}

	bool TcpAcceptor::isStopped() const {
		return m_pImpl->isStopped();
	}

	void TcpAcceptor::stop() {
		return m_pImpl->destroy();
	}

	// endregion

	// region factories

	boost::asio::ip::tcp::endpoint CreateLocalHostEndpoint() {
		return CreateLocalHostEndpoint(GetLocalHostPort());
	}

	boost::asio::ip::tcp::endpoint CreateLocalHostEndpointIPv6() {
		return CreateLocalHostEndpointIPv6(GetLocalHostPort());
	}

	boost::asio::ip::tcp::endpoint CreateLocalHostEndpoint(unsigned short port) {
		return boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port);
	}

	boost::asio::ip::tcp::endpoint CreateLocalHostEndpointIPv6(unsigned short port) {
		return boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("::1"), port);
	}

	ionet::PacketSocketSslOptions CreatePacketSocketSslOptions() {
		return CreatePacketSocketSslOptions(Key());
	}

	ionet::PacketSocketSslOptions CreatePacketSocketSslOptions(const Key& publicKey) {
		ionet::PacketSocketSslOptions options;
		options.ContextSupplier = ionet::CreateSslContextSupplier(GetDefaultCertificateDirectory());
		options.VerifyCallbackSupplier = [publicKey]() {
			return [publicKey](auto& verifyContext) {
				verifyContext.setPublicKey(publicKey);
				return true;
			};
		};
		return options;
	}

	ionet::PacketSocketOptions CreatePacketSocketOptions() {
		return CreatePacketSocketOptions(Key());
	}

	ionet::PacketSocketOptions CreatePacketSocketOptions(const Key& publicKey) {
		return CreateConnectionSettings(publicKey).toSocketOptions();
	}

	net::ConnectionSettings CreateConnectionSettings() {
		return CreateConnectionSettings(Key());
	}

	net::ConnectionSettings CreateConnectionSettings(const Key& publicKey) {
		auto settings = net::ConnectionSettings();
		settings.Timeout = utils::TimeSpan::FromMinutes(1);
		settings.SslOptions = CreatePacketSocketSslOptions(publicKey);
		return settings;
	}

	std::shared_ptr<boost::asio::ip::tcp::acceptor> CreateImplicitlyClosedLocalHostAcceptor(boost::asio::io_context& ioContext) {
		auto pAcceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(ioContext);
		BindAcceptor(*pAcceptor, CreateLocalHostEndpoint());
		pAcceptor->listen();
		return pAcceptor;
	}

	// endregion

	// region spawn work helpers

	void SpawnPacketServerWork(boost::asio::io_context& ioContext, const PacketSocketWork& serverWork) {
		SpawnPacketServerWork(ioContext, CreatePacketSocketOptions(), serverWork);
	}

	void SpawnPacketServerWork(
			boost::asio::io_context& ioContext,
			const ionet::PacketSocketOptions& options,
			const PacketSocketWork& serverWork) {
		// Arrange: start listening immediately
		//          this will prevent a race condition where the client work (async_connect) is dispatched before
		//          the server work has set up a listener (listen), resulting in an unwanted connection refused error
		CATAPULT_LOG(debug) << "starting server listening on " << CreateLocalHostEndpoint();
		auto pAcceptor = std::make_shared<TcpAcceptor>(ioContext);

		// - post the work to the thread pool and extend the acceptor lifetime
		SpawnPacketServerWork(*pAcceptor, options, [serverWork, pAcceptor](const auto& pSocket) {
			serverWork(pSocket);
		});
	}

	void SpawnPacketServerWork(const TcpAcceptor& acceptor, const PacketSocketWork& serverWork) {
		SpawnPacketServerWork(acceptor, CreatePacketSocketOptions(), serverWork);
	}

	void SpawnPacketServerWork(
			const TcpAcceptor& acceptor,
			const ionet::PacketSocketOptions& options,
			const PacketSocketWork& serverWork) {
		// Arrange: post the work to the thread pool
		std::weak_ptr<TcpAcceptor::Impl> pAcceptorImplWeak = acceptor.m_pImpl;
		boost::asio::post(acceptor.strand(), [options, serverWork, pAcceptorImplWeak]() {
			auto pAcceptorImpl = pAcceptorImplWeak.lock();
			if (!pAcceptorImpl) {
				CATAPULT_LOG(warning) << "acceptor destroyed before accept initiated";
				return;
			}

			// - accept a connection
			ionet::Accept(pAcceptorImpl->ioContext(), pAcceptorImpl->acceptor(), options, [serverWork](const auto& socketInfo) {
				CATAPULT_LOG(debug) << "server socket accepted: " << socketInfo.socket().get();

				// Act: perform the server work
				if (socketInfo)
					serverWork(socketInfo.socket());
			});
		});
	}

	void SpawnPacketClientWork(boost::asio::io_context& ioContext, const PacketSocketWork& clientWork) {
		boost::asio::post(ioContext, [&ioContext, clientWork]() {
			auto endpoint = CreateLocalHostNodeEndpoint();
			ionet::Connect(ioContext, CreatePacketSocketOptions(), endpoint, [clientWork](auto result, const auto& socketInfo) {
				CATAPULT_LOG(debug) << "client socket connected " << result;
				if (socketInfo)
					clientWork(socketInfo.socket());
			});
		});
	}

	// endregion

	// region packet socket utils

	ionet::PacketSocketInfo CreatePacketSocketInfo(const std::shared_ptr<ionet::PacketSocket>& pPacketSocket) {
		return ionet::PacketSocketInfo("", Key(), pPacketSocket);
	}

	bool IsSocketOpen(ionet::PacketSocket& socket) {
		struct Capture {
		public:
			Capture() : HasStats(false)
			{}

		public:
			ionet::PacketSocket::Stats Stats;
			std::atomic_bool HasStats;
		};

		auto pCapture = std::make_shared<Capture>();
		socket.stats([pCapture](const auto& socketStats) {
			pCapture->Stats = socketStats;
			pCapture->HasStats = true;
		});

		WAIT_FOR(pCapture->HasStats);
		return pCapture->Stats.IsOpen;
	}

	void WaitForClosedSocket(ionet::PacketSocket& socket) {
		WAIT_FOR_EXPR(!IsSocketOpen(socket));
	}

	void AssertEmpty(const ionet::PacketSocketInfo& socketInfo) {
		EXPECT_FALSE(!!socketInfo);
		EXPECT_EQ("", socketInfo.host());
		EXPECT_EQ(Key(), socketInfo.publicKey());
		EXPECT_FALSE(!!socketInfo.socket());
	}

	// endregion

	// region write tests

	namespace {
		const size_t Large_Buffer_Size = 100 * 1024;

		struct LargeWritePayload {
		public:
			explicit LargeWritePayload(uint32_t bufferSize)
					: Buffer(GenerateRandomPacketBuffer(bufferSize))
					, pPacket(BufferToPacket(Buffer))
			{}

		public:
			const ionet::ByteBuffer Buffer;
			const std::shared_ptr<ionet::Packet> pPacket;
			ionet::SocketOperationCode Code;

		public:
			template<typename T>
			bool isBufferEqualTo(T start, T end) const {
				return std::equal(Buffer.cbegin(), Buffer.cend(), start, end);
			}
		};
	}

	void AssertWriteCanWriteMultipleConsecutivePayloads(const PacketIoTransform& transform) {
		// Arrange: set up payloads
		LargeWritePayload payload1(Large_Buffer_Size);
		LargeWritePayload payload2(Large_Buffer_Size);
		ionet::ByteBuffer receiveBuffer(2 * Large_Buffer_Size);

		// Act: "server" - starts two chained async write operations
		//      "client" - reads a payload from the socket
		auto pPool = CreateStartedIoThreadPool();
		SpawnPacketServerWork(pPool->ioContext(), [&](const auto& pServerSocket) {
			auto pIo = transform(pServerSocket);
			pIo->write(ionet::PacketPayload(payload1.pPacket), [pIo, &payload1, &payload2](auto writeCode1) {
				payload1.Code = writeCode1;

				pIo->write(ionet::PacketPayload(payload2.pPacket), [&payload2](auto writeCode2) {
					payload2.Code = writeCode2;
				});
			});
		});
		auto pClientSocket = AddClientReadBufferTask(pPool->ioContext(), receiveBuffer);
		pPool->join();

		// Assert: both writes should have succeeded and no data should have been interleaved
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload1.Code);
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload2.Code);

		auto pReceiveBuffer1End = receiveBuffer.cbegin() + Large_Buffer_Size;
		EXPECT_TRUE(payload1.isBufferEqualTo(receiveBuffer.cbegin(), pReceiveBuffer1End));
		EXPECT_TRUE(payload2.isBufferEqualTo(pReceiveBuffer1End, receiveBuffer.cend()));
	}

	void AssertWriteCanWriteMultipleSimultaneousPayloadsWithoutInterleaving(const PacketIoTransform& transform) {
		// Arrange: set up payloads
		LargeWritePayload payload1(Large_Buffer_Size);
		LargeWritePayload payload2(Large_Buffer_Size);
		ionet::ByteBuffer receiveBuffer(2 * Large_Buffer_Size);

		// Act: "server" - starts two concurrent async write operations
		//      "client" - reads a payload from the socket
		auto pPool = CreateStartedIoThreadPool();
		SpawnPacketServerWork(pPool->ioContext(), [&](const auto& pServerSocket) {
			auto pIo = transform(pServerSocket);
			pIo->write(ionet::PacketPayload(payload1.pPacket), [&payload1](auto writeCode) {
				payload1.Code = writeCode;
			});
			pIo->write(ionet::PacketPayload(payload2.pPacket), [&payload2](auto writeCode) {
				payload2.Code = writeCode;
			});
		});
		auto pClientSocket = AddClientReadBufferTask(pPool->ioContext(), receiveBuffer);
		pPool->join();

		// Assert: both writes should have succeeded and no data should have been interleaved
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload1.Code);
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload2.Code);

		auto pReceiveBuffer1End = receiveBuffer.cbegin() + Large_Buffer_Size;
		EXPECT_TRUE(payload1.isBufferEqualTo(receiveBuffer.cbegin(), pReceiveBuffer1End));
		EXPECT_TRUE(payload2.isBufferEqualTo(pReceiveBuffer1End, receiveBuffer.cend()));
	}

	void AssertSocketClosedDuringWrite(ionet::SocketOperationCode writeCode) {
		// Assert: the socket was closed
		//         the underlying socket can return operation cancelled (Write_Error)
		EXPECT_EQ(ionet::SocketOperationCode::Write_Error, writeCode);
	}

	// endregion

	// region read tests

	namespace {
		struct LargeReadPayload {
		public:
			explicit LargeReadPayload(uint32_t bufferSize) : Buffer(GenerateRandomPacketBuffer(bufferSize))
			{}

		public:
			const ionet::ByteBuffer Buffer;
			ionet::ByteBuffer ReadBuffer;
			ionet::SocketOperationCode Code;

		public:
			void update(ionet::SocketOperationCode code, const ionet::Packet* pPacket) {
				Code = code;
				if (!pPacket)
					return;

				ReadBuffer = CopyPacketToBuffer(*pPacket);
			}

			bool isBufferRead() const {
				return std::equal(Buffer.cbegin(), Buffer.cend(), ReadBuffer.cbegin(), ReadBuffer.cend());
			}
		};

		auto AddClientWriteBuffersTask(
				boost::asio::io_context& ioContext,
				const LargeReadPayload& payload1,
				const LargeReadPayload& payload2) {
			return test::AddClientWriteBuffersTask(ioContext, { payload1.Buffer, payload2.Buffer });
		}
	}

	void AssertReadCanReadMultipleConsecutivePayloads(const PacketIoTransform& transform) {
		// Arrange: set up payloads
		LargeReadPayload payload1(Large_Buffer_Size);
		LargeReadPayload payload2(Large_Buffer_Size);

		// Act: "server" - starts two chained async read operations
		//      "client" - writes the payloads to the socket
		auto pPool = CreateStartedIoThreadPool();
		SpawnPacketServerWork(pPool->ioContext(), [&](const auto& pServerSocket) {
			auto pIo = transform(pServerSocket);
			pIo->read([pIo, &payload1, &payload2](auto result1, const auto* pPacket1) {
				payload1.update(result1, pPacket1);

				pIo->read([&payload2](auto result2, const auto* pPacket2) {
					payload2.update(result2, pPacket2);
				});
			});
		});
		auto pClientSocket = AddClientWriteBuffersTask(pPool->ioContext(), payload1, payload2);
		pPool->join();

		// Assert: both reads should have succeeded and no data should have been interleaved
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload1.Code);
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload2.Code);

		EXPECT_TRUE(payload1.isBufferRead());
		EXPECT_TRUE(payload2.isBufferRead());
	}

	void AssertReadCanReadMultipleSimultaneousPayloadsWithoutInterleaving(const PacketIoTransform& transform) {
		// Arrange: set up payloads
		LargeReadPayload payload1(Large_Buffer_Size);
		LargeReadPayload payload2(Large_Buffer_Size);

		// Act: "server" - starts two concurrent async read operations
		//      "client" - writes the payloads to the socket
		auto pPool = CreateStartedIoThreadPool();
		SpawnPacketServerWork(pPool->ioContext(), [&](const auto& pServerSocket) {
			auto pIo = transform(pServerSocket);
			pIo->read([&payload1](auto result, const auto* pPacket) {
				payload1.update(result, pPacket);
			});
			pIo->read([&payload2](auto result, const auto* pPacket) {
				payload2.update(result, pPacket);
			});
		});
		auto pClientSocket = AddClientWriteBuffersTask(pPool->ioContext(), payload1, payload2);
		pPool->join();

		// Assert: both reads should have succeeded and no data should have been interleaved
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload1.Code);
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload2.Code);

		EXPECT_TRUE(payload1.isBufferRead());
		EXPECT_TRUE(payload2.isBufferRead());
	}

	void AssertSocketClosedDuringRead(ionet::SocketOperationCode readCode) {
		// Assert: the socket was closed
		//         the underlying socket can either return eof (Closed) or operation cancelled (Read_Error)
		std::set<ionet::SocketOperationCode> possibleValues{
			ionet::SocketOperationCode::Closed,
			ionet::SocketOperationCode::Read_Error
		};
		EXPECT_CONTAINS(possibleValues, readCode);
	}

	// endregion
}}
