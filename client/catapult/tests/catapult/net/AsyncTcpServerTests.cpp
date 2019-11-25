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
#include "tests/test/net/SocketTestUtils.h"
#include <boost/asio/steady_timer.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <thread>

namespace catapult { namespace net {

#define TEST_CLASS AsyncTcpServerTests

	namespace {
		const uint32_t Num_Default_Threads = test::GetNumDefaultPoolThreads();
		const uint32_t Default_Max_Active_Connections = 2 * Num_Default_Threads + 1;

		class PoolServerPair {
		public:
			PoolServerPair(
					const std::shared_ptr<thread::IoThreadPool>& pPool,
					const boost::asio::ip::tcp::endpoint& endpoint,
					const AsyncTcpServerSettings& settings)
					: m_pPool(pPool) {
				m_pServer = CreateAsyncTcpServer(pPool, endpoint, settings);
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
			std::shared_ptr<thread::IoThreadPool> m_pPool;
			std::shared_ptr<AsyncTcpServer> m_pServer;

		public:
			PoolServerPair(PoolServerPair&& rhs) = default;
		};

		AsyncTcpServerSettings CreateSettings(const AcceptHandler& acceptHandler) {
			auto settings = AsyncTcpServerSettings(acceptHandler);
			settings.MaxActiveConnections = Default_Max_Active_Connections;
			return settings;
		}

		class ClientService {
		public:
			ClientService(uint32_t numAttempts, uint32_t numThreads, size_t timeoutMillis = 50)
					: m_numAttempts(numAttempts)
					, m_numConnects(0)
					, m_numConnectFailures(0)
					, m_numConnectTimeouts(0) {
				spawnConnectionAttempts(numAttempts, timeoutMillis);
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
				m_ioContext.stop();
				m_threads.join_all();
				CATAPULT_LOG(debug) << "ClientService shut down";
			}

		private:
			enum class ConnectionStatus { Unset, Success, Error, Timeout };

			// region SpawnContext

			class SpawnContext : public std::enable_shared_from_this<SpawnContext> {
			private:
				using ConnectionHandler = consumer<ConnectionStatus>;

			public:
				SpawnContext(boost::asio::io_context& ioContext, size_t id)
						: m_socket(ioContext)
						, m_id(id)
						, m_deadline(ioContext)
						, m_status(ConnectionStatus::Unset)
				{}

			public:
				void setDeadline(size_t millis) {
					m_deadline.expires_from_now(std::chrono::milliseconds(millis));
					m_deadline.async_wait([pThis = shared_from_this()](const auto& ec) {
						pThis->handleTimeout(ec);
					});
				}

			private:
				void handleTimeout(const boost::system::error_code& ec) {
					if (setStatus(ConnectionStatus::Timeout)) {
						CATAPULT_LOG(debug) << "test thread " << m_id << " timed out " << ec.message();
						m_socket.close();
					}
				}

			public:
				void connect(const ConnectionHandler& handler) {
					m_socket.async_connect(test::CreateLocalHostEndpoint(), [pThis = shared_from_this(), handler](const auto& ec) {
						pThis->handleConnect(ec, handler);
					});
				}

			private:
				void handleConnect(const boost::system::error_code& ec, const ConnectionHandler& connectionHandler) {
					if (!ec) {
						setStatus(ConnectionStatus::Success);
						CATAPULT_LOG(trace) << "test thread " << m_id << " connected";
					} else {
						setStatus(ConnectionStatus::Error);
						CATAPULT_LOG(debug) << "test thread " << m_id << " errored " << ec.message();
					}

					connectionHandler(m_status);
				}

				bool setStatus(ConnectionStatus status) {
					auto expected = ConnectionStatus::Unset;
					return m_status.compare_exchange_strong(expected, status);
				}

			private:
				ionet::socket m_socket;
				size_t m_id;
				boost::asio::steady_timer m_deadline;
				std::atomic<ConnectionStatus> m_status;
			};

			// endregion

			void spawnConnectionAttempts(uint32_t numAttempts, size_t timeoutMillis) {
				for (auto i = 0u; i < numAttempts; ++i) {
					auto pContext = std::make_shared<SpawnContext>(m_ioContext, i);
					boost::asio::post(m_ioContext, [this, timeoutMillis, pContext{std::move(pContext)}]() {
						// set a 50ms deadline on the async_connect
						pContext->setDeadline(timeoutMillis);
						pContext->connect([this](auto status) {
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
							}
						});
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
		};

		template<typename T>
		auto CreateLocalHostAsyncTcpServer(const T& settings, bool allowAddressReuse = true) {
			// enable AllowAddressReuse in tests in order to avoid sporadic 'address already in use' errors
			auto testSettings = AsyncTcpServerSettings(settings);
			testSettings.AllowAddressReuse = allowAddressReuse;

			auto pPool = test::CreateStartedIoThreadPool();
			return PoolServerPair(std::move(pPool), test::CreateLocalHostEndpoint(), testSettings);
		}

		constexpr int Wait_Duration_Millis = 5;

		class AcceptServer {
		public:
			explicit AcceptServer(const test::WaitFunction& wait)
					: m_pPool(test::CreateStartedIoThreadPool(1))
					, m_shouldWait(1)
					, m_numAcceptCallbacks(0) {
				m_pSettings = std::make_unique<AsyncTcpServerSettings>([&, wait](const auto& acceptedSocketInfo) {
					++m_numAcceptCallbacks;
					wait(m_pPool->ioContext(), [this, acceptedSocketInfo]() { return 0 != m_shouldWait; });
				});
				m_pSettings->MaxActiveConnections = Default_Max_Active_Connections;
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

		const auto Empty_Accept_Handler = [](const auto&) {};
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
		ionet::socket socket(ioContext);
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

	TEST(TEST_CLASS, ServerForwardsAppropriateAcceptedSocketInfoOnSuccess) {
		// Arrange: set up a multithreaded server
		ionet::PacketSocketInfo* pAcceptedSocketInfoRaw;
		std::atomic_bool isAccepted(false);
		auto acceptHandler = [&pAcceptedSocketInfoRaw, &isAccepted](const auto& acceptedSocketInfo) {
			*pAcceptedSocketInfoRaw = acceptedSocketInfo;
			isAccepted = true;
		};
		auto pServer = CreateLocalHostAsyncTcpServer(CreateSettings(acceptHandler));

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
	}

	TEST(TEST_CLASS, ServerCanServeMoreRequestsThanWorkerThreads) {
		// Arrange: set up a multithreaded server
		std::atomic<uint32_t> numCallbacks(0);
		auto pServer = CreateLocalHostAsyncTcpServer([&numCallbacks](const auto&) { ++numCallbacks; });

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

			if (clientService.numConnectTimeouts() > 0) {
				CATAPULT_LOG(debug) << clientService.numConnectTimeouts() << " connection(s) timed out";
				return false;
			}

			// Assert: all connections should have been made and one should be executing on each server thread
			EXPECT_EQ(100u, clientService.numConnects());
			EXPECT_EQ(0u, clientService.numConnectFailures());
			EXPECT_EQ(Num_Default_Threads, server.asyncServer().numLifetimeConnections());
			EXPECT_EQ(Num_Default_Threads, server.asyncServer().numCurrentConnections());
			return true;
		});
	}

	namespace {
		bool RunServerEnforcesMaximumOnNumberOfConnectionsIteration() {
			// Arrange: set up a multithreaded server with a max connections limit and block in the accept handler
#ifdef _WIN32
			constexpr uint32_t Max_Test_Connections = 201;
#else
			constexpr uint32_t Max_Test_Connections = 10;
#endif
			constexpr uint32_t Listener_Queue_Slack = 4;

			BlockingAcceptServer server;
			server.settings().MaxPendingConnections = static_cast<uint16_t>(Max_Test_Connections);
			server.init();

			// Act: queue connects to the server (increase test connection timeout because this test makes a lot of connections)
			std::vector<std::shared_ptr<ClientService>> clientServices;
			for (auto numConnects : { Num_Default_Threads, Max_Test_Connections, 15u }) {
				auto timeoutMillis = std::max<size_t>(Num_Default_Threads * 50, 250);
				clientServices.push_back(std::make_shared<ClientService>(numConnects, 1, timeoutMillis));
				clientServices.back()->wait();
			}

			// Retry: if there are any unexpected failures
			auto numClient0Failures = clientServices[0]->numConnectFailures();
			auto numClient1Failures = clientServices[1]->numConnectFailures();
			if (0 != numClient0Failures || 0 != numClient1Failures) {
				CATAPULT_LOG(warning) << "Unexpected failures: C0 = " << numClient0Failures << ", C1 = " << numClient1Failures;
				return false;
			}

			// Assert:
			// - first Num_Default_Threads connects should be blocking server threads
			auto clientId = 0u;
			EXPECT_EQ(Num_Default_Threads, clientServices[clientId]->numConnects());
			EXPECT_EQ(0u, clientServices[clientId]->numConnectFailures());

			// - next Max_Test_Connections should be in the connection queue
			++clientId;
			EXPECT_EQ(Max_Test_Connections, clientServices[clientId]->numConnects());
			EXPECT_EQ(0u, clientServices[clientId]->numConnectFailures());

			// - most of the last 15 should have been rejected (depending on os)
			//   (on mac and windows all 15 are rejected, on linux up to 4 are not rejected)
			++clientId;
			EXPECT_GE(15u - Listener_Queue_Slack, clientServices[clientId]->numConnects());
			EXPECT_LE(0u + Listener_Queue_Slack, clientServices[clientId]->numConnectFailures());

			// - there is one connection per thread
			EXPECT_EQ(Num_Default_Threads, server.asyncServer().numLifetimeConnections());
			EXPECT_EQ(Num_Default_Threads, server.asyncServer().numCurrentConnections());
			return true;
		}
	}

	TEST(TEST_CLASS, ServerEnforcesMaximumOnNumberOfConnections) {
		// Assert: non-deterministic because it is possible for some expected successful connections to fail / timeout
		test::RunNonDeterministicTest("Max number of connections", RunServerEnforcesMaximumOnNumberOfConnectionsIteration);
	}

	TEST(TEST_CLASS, ServerCallsConfigureSocketHandlerForAllSockets) {
		// Arrange: set up a multithreaded server
		std::atomic<uint32_t> numAcceptCallbacks(0);
		std::atomic<uint32_t> numConfigureSocketCallbacks(0);
		auto settings = CreateSettings([&numAcceptCallbacks](const auto&) { ++numAcceptCallbacks; });
		settings.ConfigureSocket = [&numConfigureSocketCallbacks](const auto&) { ++numConfigureSocketCallbacks; };
		auto pServer = CreateLocalHostAsyncTcpServer(settings);

		// Act: queue four connects to the server on a single thread
		ClientService clientService(4, 1);

		// - wait for the server to get four accepts
		WAIT_FOR_VALUE(4u, numAcceptCallbacks);
		pServer.stopAll();

		// Assert: the server should have configured 5 sockets (one per request + one pending accept)
		EXPECT_EQ(5u, numConfigureSocketCallbacks);
		EXPECT_EQ(4u, numAcceptCallbacks);
	}

	TEST(TEST_CLASS, ServerAcceptorsAreNotKilledBySocketAcceptFailures) {
		// Arrange: set up a multithreaded server and cause the first two accepts to fail (already open)
		NonBlockingAcceptServer server;
		std::atomic<uint32_t> numConfigureSocketCallbacks(0);
		server.settings().ConfigureSocket = [&numConfigureSocketCallbacks](auto& socket) {
			uint32_t num = ++numConfigureSocketCallbacks;
			if (num <= 2) {
				CATAPULT_LOG(debug) << "emulating accept failure #" << num;
				socket.open(boost::asio::ip::tcp::v4());
			}
		};

		server.init();

		// Act: queue two requests per server thread
		ClientService clientService(2 * Num_Default_Threads, 1);
		server.waitForAccepts(2 * Num_Default_Threads);
		clientService.shutdown();

		// Assert: no workers should have been killed and there should still be a pending accept
		//         (additionally, all client connections should be async blocked in the accept handler)
		EXPECT_EQ(1u, server.asyncServer().numPendingAccepts());
		EXPECT_EQ(2 * Num_Default_Threads, server.asyncServer().numLifetimeConnections());
		EXPECT_EQ(2 * Num_Default_Threads, server.asyncServer().numCurrentConnections());
	}
}}
