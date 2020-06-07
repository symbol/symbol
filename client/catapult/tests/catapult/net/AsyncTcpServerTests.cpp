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

#include "catapult/net/AsyncTcpServer.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/utils/AtomicIncrementDecrementGuard.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/WaitFunctions.h"
#include "tests/test/net/ClientSocket.h"
#include "tests/test/net/SocketTestUtils.h"
#include <boost/asio/steady_timer.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <thread>

namespace catapult { namespace net {

#define TEST_CLASS AsyncTcpServerTests

	namespace {
		constexpr int Wait_Duration_Millis = 5;
		const uint32_t Num_Default_Threads = test::GetNumDefaultPoolThreads();
		const uint32_t Default_Max_Active_Connections = 2 * Num_Default_Threads + 1;
		const auto Empty_Accept_Handler = [](const auto&) {};

		// region PoolServerPair

		class PoolServerPair {
		public:
			PoolServerPair(
					std::unique_ptr<thread::IoThreadPool>&& pPool,
					const boost::asio::ip::tcp::endpoint& endpoint,
					const AsyncTcpServerSettings& settings)
					: m_pPool(std::move(pPool)) {
				m_pServer = CreateAsyncTcpServer(*m_pPool, endpoint, settings);
			}

			~PoolServerPair() {
				if (m_pPool)
					stopAll();
			}

		public:
			void stopAll() {
				// shutdown order is important
				// 1. m_pServer->shutdown stops the accept listener, which allows all threads to complete
				// 2. m_pPool->join waits for threads to complete but must finish before m_pServer is destroyed
				logState("calling m_pServer->shutdown");
				m_pServer->shutdown();
				test::WaitForUnique(m_pServer, "m_pServer");
				logState("calling m_pPool->join");
				m_pPool->join();
				logState("everything stopped");
			}

		private:
			void logState(const char* message) {
				CATAPULT_LOG(debug)
						<< message
						<< "(numWorkerThreads: " << m_pPool->numWorkerThreads()
						<< ", numPendingAccepts: " << m_pServer->numPendingAccepts() << ")";
			}

		public:
			AsyncTcpServer& operator*() {
				return *m_pServer;
			}

			AsyncTcpServer* operator->() {
				return m_pServer.get();
			}

		private:
			std::unique_ptr<thread::IoThreadPool> m_pPool;
			std::shared_ptr<AsyncTcpServer> m_pServer;

		public:
			PoolServerPair(PoolServerPair&& rhs) = default;
		};

		AsyncTcpServerSettings CreateSettings(const AcceptHandler& acceptHandler, const Key& publicKey = Key()) {
			auto settings = AsyncTcpServerSettings(acceptHandler);
			settings.PacketSocketOptions = test::CreatePacketSocketOptions(publicKey);
			settings.MaxActiveConnections = Default_Max_Active_Connections;
			return settings;
		}

		auto CreateLocalHostAsyncTcpServer(const AsyncTcpServerSettings& settings, bool allowAddressReuse = true) {
			// enable AllowAddressReuse in tests in order to avoid sporadic 'address already in use' errors
			auto testSettings = AsyncTcpServerSettings(settings);
			testSettings.AllowAddressReuse = allowAddressReuse;

			auto pPool = test::CreateStartedIoThreadPool();
			return PoolServerPair(std::move(pPool), test::CreateLocalHostEndpoint(), testSettings);
		}

		auto CreateLocalHostAsyncTcpServer(const AcceptHandler& acceptHandler, bool allowAddressReuse = true) {
			return CreateLocalHostAsyncTcpServer(AsyncTcpServerSettings(acceptHandler), allowAddressReuse);
		}

		// endregion

		// region ClientService

		class ClientService {
		public:
			ClientService(
					uint32_t numAttempts,
					uint32_t numThreads,
					test::ClientSocket::ConnectOptions connectOptions = test::ClientSocket::ConnectOptions::Normal,
					size_t timeoutMillis = 1000)
					: m_numAttempts(numAttempts)
					, m_numConnects(0)
					, m_numConnectFailures(0)
					, m_numConnectTimeouts(0) {
				spawnConnectionAttempts(numAttempts, connectOptions, timeoutMillis);
				for (auto i = 0u; i < numThreads; ++i)
					m_threads.create_thread([&]() { m_ioContext.run(); });
			}

			~ClientService() {
				shutdown();
			}

		public:
			uint32_t numConnects() const {
				return m_numConnects;
			}

			uint32_t numConnectFailures() const {
				return m_numConnectFailures;
			}

			uint32_t numConnectTimeouts() const {
				return m_numConnectTimeouts;
			}

			void wait() const {
				// note that a timed out connection is also counted as failed
				WAIT_FOR_VALUE_EXPR(m_numAttempts, m_numConnects + m_numConnectFailures);
			}

			void shutdown() {
				CATAPULT_LOG(debug)
						<< "Shutting down ClientService: "
						<< "connects " << m_numConnects
						<< ", failures " << m_numConnectFailures;

				for (const auto& pConnection : m_connections)
					pConnection->abort();

				m_ioContext.stop();
				m_threads.join_all();
				CATAPULT_LOG(debug) << "ClientService shut down";
			}

		private:
			enum class ConnectionStatus { Unset, Success, Error, Timeout };

			class SpawnContext : public std::enable_shared_from_this<SpawnContext> {
			private:
				using ConnectionHandler = consumer<ConnectionStatus>;

			public:
				SpawnContext(boost::asio::io_context& ioContext, test::ClientSocket::ConnectOptions connectOptions, size_t id)
						: m_pSocket(test::CreateClientSocket(ioContext))
						, m_connectOptions(connectOptions)
						, m_id(id)
						, m_deadline(ioContext)
						, m_status(ConnectionStatus::Unset)
				{}

			public:
				void abort() {
					m_pSocket->abort();
				}

			public:
				void setDeadline(size_t millis, const ConnectionHandler& handler) {
					m_deadline.expires_from_now(std::chrono::milliseconds(millis));
					m_deadline.async_wait([pThis = shared_from_this(), handler](const auto& ec) {
						pThis->handleTimeout(ec, handler);
					});
				}

			private:
				void handleTimeout(const boost::system::error_code& ec, const ConnectionHandler& handler) {
					if (setStatus(ConnectionStatus::Timeout)) {
						CATAPULT_LOG(debug) << "test thread " << m_id << " timed out " << ec.message();
						handler(ConnectionStatus::Timeout);

						m_pSocket->shutdown();
					}
				}

			public:
				void connect(const ConnectionHandler& handler) {
					m_pSocket->connect(m_connectOptions).then([pThis = shared_from_this(), handler](auto&& connectFuture) {
						try {
							connectFuture.get();
							pThis->handleConnect(boost::system::error_code(), handler);
						} catch (const boost::system::system_error& ex) {
							pThis->handleConnect(ex.code(), handler);
						}
					});
				}

			private:
				void handleConnect(const boost::system::error_code& ec, const ConnectionHandler& connectionHandler) {
					bool hasNewStatus = false;
					if (!ec) {
						// all non-normal connect options are errors, so don't indicate success
						if (test::ClientSocket::ConnectOptions::Normal != m_connectOptions)
							return;

						hasNewStatus = setStatus(ConnectionStatus::Success);
						CATAPULT_LOG(trace) << "test thread " << m_id << " connected";
					} else {
						hasNewStatus = setStatus(ConnectionStatus::Error);
						CATAPULT_LOG(debug) << "test thread " << m_id << " errored " << ec.message();
					}

					if (hasNewStatus)
						connectionHandler(m_status);
				}

				bool setStatus(ConnectionStatus status) {
					auto expected = ConnectionStatus::Unset;
					return m_status.compare_exchange_strong(expected, status);
				}

			private:
				std::shared_ptr<test::ClientSocket> m_pSocket;
				test::ClientSocket::ConnectOptions m_connectOptions;
				size_t m_id;
				boost::asio::steady_timer m_deadline;
				std::atomic<ConnectionStatus> m_status;
			};

		private:
			void spawnConnectionAttempts(uint32_t numAttempts, test::ClientSocket::ConnectOptions connectOptions, size_t timeoutMillis) {
				for (auto i = 0u; i < numAttempts; ++i) {
					auto pContext = std::make_shared<SpawnContext>(m_ioContext, connectOptions, i);
					m_connections.push_back(pContext);
					boost::asio::post(m_ioContext, [this, timeoutMillis, pContext{std::move(pContext)}]() {
						auto connectionHandler = [this](auto status) {
							switch (status) {
							case ConnectionStatus::Success:
								++m_numConnects;
								break;

							case ConnectionStatus::Timeout:
								// increment both timeouts and failures for timeout
								++m_numConnectTimeouts;
								++m_numConnectFailures;
								break;

							default:
								++m_numConnectFailures;
								break;
							}
						};

						// set a deadline on the connect operation
						pContext->setDeadline(timeoutMillis, connectionHandler);
						pContext->connect(connectionHandler);
					});
				}
			}

		private:
			boost::asio::io_context m_ioContext;
			boost::thread_group m_threads;
			uint32_t m_numAttempts;
			std::atomic<uint32_t> m_numConnects;
			std::atomic<uint32_t> m_numConnectFailures;
			std::atomic<uint32_t> m_numConnectTimeouts;
			std::vector<std::shared_ptr<SpawnContext>> m_connections;
		};

		// endregion

		// region AcceptServer

		class AcceptServer {
		public:
			explicit AcceptServer(const test::WaitFunction& wait)
					: m_pPool(test::CreateStartedIoThreadPool(1))
					, m_shouldWait(1)
					, m_numAcceptCallbacks(0) {
				m_pSettings = std::make_unique<AsyncTcpServerSettings>(CreateSettings([&, wait](const auto& acceptedSocketInfo) {
					++m_numAcceptCallbacks;
					wait(m_pPool->ioContext(), [this, acceptedSocketInfo]() { return 0 != m_shouldWait; });
				}));
			}

			~AcceptServer() {
				unblock();
			}

		public:
			AsyncTcpServerSettings& settings() {
				return *m_pSettings;
			}

			void waitForAccepts(uint32_t numAccepts) {
				WAIT_FOR_VALUE(numAccepts, m_numAcceptCallbacks);
			}

			void unblock() {
				m_shouldWait = 0;
			}

			void init() {
				auto pServer = CreateLocalHostAsyncTcpServer(*m_pSettings);
				m_ppServer = std::make_unique<PoolServerPair>(std::move(pServer));
			}

		public:
			const AsyncTcpServer& asyncServer() const {
				return **m_ppServer;
			}

		private:
			std::unique_ptr<thread::IoThreadPool> m_pPool;
			std::unique_ptr<AsyncTcpServerSettings> m_pSettings;
			std::atomic<uint32_t> m_shouldWait;
			std::atomic<uint32_t> m_numAcceptCallbacks;
			std::unique_ptr<PoolServerPair> m_ppServer;
		};

		class BlockingAcceptServer : public AcceptServer {
		public:
			BlockingAcceptServer() : AcceptServer(test::CreateSyncWaitFunction(Wait_Duration_Millis))
			{}
		};

		class NonBlockingAcceptServer : public AcceptServer {
		public:
			NonBlockingAcceptServer() : AcceptServer(test::CreateAsyncWaitFunction(Wait_Duration_Millis))
			{}
		};

		// endregion
	}

	// region basic start and shutdown

	TEST(TEST_CLASS, ServerCreatesSinglePendingAccept) {
		// Act: set up a multithreaded server
		auto pServer = CreateLocalHostAsyncTcpServer(Empty_Accept_Handler);

		// Assert: one pending accept has been spawned and no connections have been made
		EXPECT_EQ(1u, pServer->numPendingAccepts());
		EXPECT_EQ(0u, pServer->numLifetimeConnections());
		EXPECT_EQ(0u, pServer->numCurrentConnections());
	}

	TEST(TEST_CLASS, CannotCreateTwoServersOnSamePort) {
		// Arrange: create a server that should occupy the server port
		auto pServer = CreateLocalHostAsyncTcpServer(Empty_Accept_Handler, true);

		// Act + Assert: attempt to create another server on the same port
		EXPECT_THROW(CreateLocalHostAsyncTcpServer(Empty_Accept_Handler, false), boost::exception);
	}

	TEST(TEST_CLASS, ServerShutdownDestroysAllPendingAccepts) {
		// Arrange: set up a multithreaded server
		auto pServer = CreateLocalHostAsyncTcpServer(Empty_Accept_Handler);

		// Act: stop the server
		pServer.stopAll();

		// Assert: all pending accepts have been stopped
		EXPECT_EQ(0u, pServer->numPendingAccepts());
		EXPECT_EQ(0u, pServer->numLifetimeConnections());
		EXPECT_EQ(0u, pServer->numCurrentConnections());
	}

	// endregion

	// region other shutdown tests

	TEST(TEST_CLASS, ServerShutdownIsIdempotent) {
		// Arrange: set up a multithreaded server
		auto pServer = CreateLocalHostAsyncTcpServer(Empty_Accept_Handler);

		// Act: stop the server
		for (auto i = 0; i < 3; ++i)
			pServer->shutdown();
		pServer.stopAll();

		// Assert: all pending accepts have been stopped
		EXPECT_EQ(0u, pServer->numPendingAccepts());
		EXPECT_EQ(0u, pServer->numLifetimeConnections());
		EXPECT_EQ(0u, pServer->numCurrentConnections());
	}

	TEST(TEST_CLASS, ServerCannotAcceptNewClientsAfterShutdown) {
		// Arrange: set up a multithreaded server
		auto pServer = CreateLocalHostAsyncTcpServer(Empty_Accept_Handler);

		// - stop the server
		pServer->shutdown();

		// Act: connect to the server
		boost::system::error_code connectEc;
		boost::asio::io_context ioContext;
		ionet::NetworkSocket socket(ioContext);
		socket.async_connect(test::CreateLocalHostEndpoint(), [&connectEc](const auto& ec) {
			connectEc = ec;
		});
		ioContext.run();

		// Assert: a connection error was returned
		EXPECT_EQ(boost::system::errc::connection_refused, connectEc);
	}

	TEST(TEST_CLASS, ServerShutdownDoesNotAbortThreads) {
		// Arrange: set up a multithreaded server
		std::atomic_bool isAccepted(false);
		std::atomic<uint32_t> numWaits(0);
		std::atomic<uint32_t> maxWaits(10000);
		auto acceptHandler = [&](const auto&) {
			isAccepted = true;
			while (numWaits < maxWaits) {
				test::Sleep(Wait_Duration_Millis);
				++numWaits;
			}
		};
		auto pServer = CreateLocalHostAsyncTcpServer(CreateSettings(acceptHandler));

		// - connect to the server
		ClientService clientService(1, 1);
		WAIT_FOR(isAccepted);

		// Act: stop the server
		uint32_t preShutdownWaits = numWaits;
		maxWaits = numWaits + 10;
		pServer.stopAll();

		// Assert: the entire strand was allowed to complete and was not aborted
		CATAPULT_LOG(debug)
				<< "preShutdownWaits " << preShutdownWaits
				<< " numWaits " << numWaits
				<< " maxWaits " << maxWaits;
		EXPECT_LE(10u, maxWaits - preShutdownWaits);
		EXPECT_EQ(maxWaits, numWaits);
	}

	// endregion

	// region basic connections

	TEST(TEST_CLASS, ServerForwardsAppropriateAcceptedSocketInfoOnSuccess) {
		// Arrange: set up a multithreaded server
		ionet::PacketSocketInfo* pAcceptedSocketInfoRaw;
		std::atomic_bool isAccepted(false);
		auto acceptHandler = [&pAcceptedSocketInfoRaw, &isAccepted](const auto& acceptedSocketInfo) {
			*pAcceptedSocketInfoRaw = acceptedSocketInfo;
			isAccepted = true;
		};

		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto pServer = CreateLocalHostAsyncTcpServer(CreateSettings(acceptHandler, publicKey));

		// - for correct shutdown, the captured accepted socket needs to be destroyed before pServer
		auto pAcceptedSocketInfo = std::make_unique<ionet::PacketSocketInfo>();
		pAcceptedSocketInfoRaw = pAcceptedSocketInfo.get();

		// Act: connect to the server
		ClientService clientService(1, 1);
		WAIT_FOR(isAccepted);

		// Assert: check the accept info (loopback address is used in tests)
		const auto& acceptedSocketInfo = *pAcceptedSocketInfo;
		EXPECT_TRUE(!!acceptedSocketInfo);
		EXPECT_TRUE(!!acceptedSocketInfo.socket());
		EXPECT_EQ("127.0.0.1", acceptedSocketInfo.host());
		EXPECT_EQ(publicKey, acceptedSocketInfo.publicKey());
	}

	TEST(TEST_CLASS, ServerCanServeMoreRequestsThanWorkerThreads) {
		// Arrange: set up a multithreaded server
		std::atomic<uint32_t> numCallbacks(0);
		auto pServer = CreateLocalHostAsyncTcpServer(CreateSettings([&numCallbacks](const auto&) { ++numCallbacks; }));

		// Act: queue more connects than threads to the server on a single thread
		ClientService clientService(2 * Num_Default_Threads, 1);

		// - wait for the server to execute all accept handlers and then stop the server
		WAIT_FOR_VALUE(2 * Num_Default_Threads, numCallbacks);
		pServer.stopAll();

		// Assert: the server should have executed 2X > X accept handlers and all should have completed
		EXPECT_EQ(2 * Num_Default_Threads, pServer->numLifetimeConnections());
		EXPECT_EQ(0u, pServer->numCurrentConnections());
	}

	TEST(TEST_CLASS, ServerWorkerThreadsCannotServiceAdditionalRequestsWhenHandlersWaitBlocking) {
		// Arrange: set up a multithreaded server
		BlockingAcceptServer server;
		server.init();

		// Act: queue one connect per server thread and wait for them to get in the handler (and block)
		ClientService clientService(Num_Default_Threads, 1);
		server.waitForAccepts(Num_Default_Threads);

		// - queue an additional per connect server thread (there should not be a free thread to accept them)
		ClientService clientService2(Num_Default_Threads, 1);

		// - wait a bit to see if the state changes to something unwanted
		test::Pause();

		// Assert: one connection per thread should have been accepted and the rest should be outstanding
		EXPECT_EQ(Num_Default_Threads, server.asyncServer().numLifetimeConnections());
		EXPECT_EQ(Num_Default_Threads, server.asyncServer().numCurrentConnections());
		EXPECT_EQ(1u, server.asyncServer().numPendingAccepts());
	}

	TEST(TEST_CLASS, ServerWorkerThreadsCanServiceAdditionalRequestsWhenHandlersWaitNonBlocking) {
		// Arrange: set up a multithreaded server
		NonBlockingAcceptServer server;
		server.init();

		// Act: queue two connects per server thread
		ClientService clientService(2 * Num_Default_Threads, 1);

		// - wait for the server to accept all connects
		server.waitForAccepts(2 * Num_Default_Threads);

		// - wait a bit to see if the state changes to something unwanted
		test::Pause();

		// Assert: two connections per thread should have been accepted and all should be outstanding
		EXPECT_EQ(2 * Num_Default_Threads, server.asyncServer().numLifetimeConnections());
		EXPECT_EQ(2 * Num_Default_Threads, server.asyncServer().numCurrentConnections());
		EXPECT_EQ(1u, server.asyncServer().numPendingAccepts());
	}

	TEST(TEST_CLASS, ServerHonorsMaxActiveConnectionsForNonBlockingOperations) {
		// Arrange: set up a multithreaded server
		NonBlockingAcceptServer server;
		server.settings().MaxActiveConnections = 5;
		server.init();

		// Act: queue eight connects to the server on a single thread
		ClientService clientService(8, 1);

		// - wait for the server to get five connects
		server.waitForAccepts(5);

		// - wait a bit to see if the state changes to something unwanted
		test::Pause();

		// Assert: five connections should have been accepted and all should be outstanding
		EXPECT_EQ(5u, server.asyncServer().numLifetimeConnections());
		EXPECT_EQ(5u, server.asyncServer().numCurrentConnections());
		EXPECT_EQ(0u, server.asyncServer().numPendingAccepts());
	}

	TEST(TEST_CLASS, ServerAddsNewAcceptWorkItemsAsConnectionsAreCompleted) {
		// Arrange: set up a multithreaded server
		NonBlockingAcceptServer server;
		server.settings().MaxActiveConnections = 5;
		server.init();

		// Act: queue eight connects to the server on a single thread, unblock them, let them finish
		ClientService clientService(8, 1);
		server.waitForAccepts(5);
		CATAPULT_LOG(debug) << "Five threads entered callback, unblocking";
		server.unblock();
		server.waitForAccepts(8);
		WAIT_FOR_ZERO_EXPR(server.asyncServer().numCurrentConnections());

		// Assert: all eight connections should have completed with one pending accept remaining
		EXPECT_EQ(8u, server.asyncServer().numLifetimeConnections());
		EXPECT_EQ(0u, server.asyncServer().numCurrentConnections());
		EXPECT_EQ(1u, server.asyncServer().numPendingAccepts());
	}

	TEST(TEST_CLASS, ServerAcceptorsAreNotKilledBySocketConnectFailures) {
		// Arrange: set up a multithreaded server
		BlockingAcceptServer server;
		server.init();

		// Sanity:
		EXPECT_EQ(1u, server.asyncServer().numPendingAccepts());
		EXPECT_EQ(0u, server.asyncServer().numLifetimeConnections());
		EXPECT_EQ(0u, server.asyncServer().numCurrentConnections());

		// Act: make connections that fail handshake
		ClientService clientService1(2, 1, test::ClientSocket::ConnectOptions::Skip_Handshake);
		clientService1.wait();

		WAIT_FOR_ONE_EXPR(server.asyncServer().numPendingAccepts());

		// Sanity:
		EXPECT_EQ(1u, server.asyncServer().numPendingAccepts());
		EXPECT_EQ(0u, server.asyncServer().numLifetimeConnections());
		EXPECT_EQ(0u, server.asyncServer().numCurrentConnections());

		// Act: make connections that pass handshake
		ClientService clientService2(2, 1);
		clientService2.wait();

		WAIT_FOR_VALUE_EXPR(2u, server.asyncServer().numLifetimeConnections());
		WAIT_FOR_ONE_EXPR(server.asyncServer().numPendingAccepts());

		// Assert: no workers should have been killed and there should still be a pending accept
		//         (additionally, all client connections should be async blocked in the accept handler)
		EXPECT_EQ(1u, server.asyncServer().numPendingAccepts());
		EXPECT_EQ(2u, server.asyncServer().numLifetimeConnections());
		EXPECT_EQ(2u, server.asyncServer().numCurrentConnections());
	}

	// endregion

	// region MaxPendingConnections

	TEST(TEST_CLASS, ServerAllowsManyConnections) {
		test::RunNonDeterministicTest("server allows many connections", []() {
			// Arrange: set up a multithreaded server and 100 max connections and block in the accept handler
			BlockingAcceptServer server;
			server.settings().MaxPendingConnections = 100;
			server.init();

			// Act: queue 100 connects to the server on a single thread
			ClientService clientService(100, 1);

			// - wait for all connections to be queued
			clientService.wait();

			// - wait for all threads to have work
			WAIT_FOR_VALUE_EXPR(Num_Default_Threads, server.asyncServer().numLifetimeConnections());

			if (clientService.numConnectTimeouts() > 100 - Num_Default_Threads) {
				CATAPULT_LOG(debug) << clientService.numConnectTimeouts() << " connection(s) timed out";
				return false;
			}

			// Assert: all connections should have been made and one should be executing on each server thread
			//         (notice that all connections greater than Num_Default_Threads connect but don't complete handshake)
			EXPECT_EQ(Num_Default_Threads, clientService.numConnects());
			EXPECT_EQ(100u - Num_Default_Threads, clientService.numConnectTimeouts());
			EXPECT_EQ(Num_Default_Threads, server.asyncServer().numLifetimeConnections());
			EXPECT_EQ(Num_Default_Threads, server.asyncServer().numCurrentConnections());
			return true;
		});
	}

	namespace {
		bool RunServerEnforcesMaximumOnNumberOfConnectionsIteration() {
			// Arrange: set up a multithreaded server with a max connections limit and block in the accept handler
			constexpr uint32_t Max_Test_Connections = 25;

			BlockingAcceptServer server;
			server.settings().MaxPendingConnections = static_cast<uint16_t>(Num_Default_Threads);
			server.init();

			// - (phase 1) queue connects to the server that will fill up all threads
			ClientService clientServicePhase1(Num_Default_Threads, 1);
			clientServicePhase1.wait();

			// Sanity: all connections succeeded
			if (0 != clientServicePhase1.numConnectFailures())
				return false;

			EXPECT_EQ(Num_Default_Threads, clientServicePhase1.numConnects());
			EXPECT_EQ(0u, clientServicePhase1.numConnectFailures());
			EXPECT_EQ(0u, clientServicePhase1.numConnectTimeouts());

			// Act: (phase 2) queue connects that fill up the listener queue and additional connections
			ClientService clientServicePhase2(Max_Test_Connections, 1);
			clientServicePhase2.wait();

			// Assert: all additional connections timed out because the connect handshake couldn't be completed
			EXPECT_EQ(0u, clientServicePhase2.numConnects());
			EXPECT_EQ(Max_Test_Connections, clientServicePhase2.numConnectFailures());
			EXPECT_EQ(Max_Test_Connections, clientServicePhase2.numConnectTimeouts());

			// - there is (still) one connection per thread
			EXPECT_EQ(Num_Default_Threads, server.asyncServer().numLifetimeConnections());
			EXPECT_EQ(Num_Default_Threads, server.asyncServer().numCurrentConnections());
			return true;
		}
	}

	TEST(TEST_CLASS, ServerEnforcesMaximumOnNumberOfConnections) {
		// Assert: non-deterministic because it is possible for some expected successful connections to fail / timeout
		test::RunNonDeterministicTest("Max number of connections", RunServerEnforcesMaximumOnNumberOfConnectionsIteration);
	}

	// endregion
}}
