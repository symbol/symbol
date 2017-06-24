#include "AsyncTcpServer.h"
#include "ConnectionSettings.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/utils/Logging.h"
#include <atomic>

namespace catapult { namespace net {

	namespace {
		/// Allow at most one pending accept at a time.
		const uint32_t Max_Pending_Accepts = 1;

		/// Wraps the socket returned by ionet::Accept and triggers a callback when it is destroyed.
		/// (When wrapped around a valid connection, the destruction callback is used to decrement the current
		/// connections counter).
		class DefaultAsyncTcpServerAcceptContext : public AsyncTcpServerAcceptContext {
		public:
			explicit DefaultAsyncTcpServerAcceptContext(
					boost::asio::io_service& service,
					const std::shared_ptr<ionet::PacketSocket>& pSocket,
					std::function<void (bool)> destructionCallback)
					: m_service(service)
					, m_pSocket(std::move(pSocket))
					, m_destructionCallback(destructionCallback)
			{}

			~DefaultAsyncTcpServerAcceptContext() {
				// don't allow the socket to stay open after the destruction of its associated context
				if (isValid())
					m_pSocket->close();

				m_destructionCallback(isValid());
			}

		public:
			boost::asio::io_service& service() override { return m_service; }
			std::shared_ptr<ionet::PacketSocket> socket() override { return m_pSocket; }

		public:
			bool isValid() { return nullptr != m_pSocket; }

		private:
			boost::asio::io_service& m_service;
			std::shared_ptr<ionet::PacketSocket> m_pSocket;
			std::function<void (bool)> m_destructionCallback;
		};

		void EnableAddressReuse(boost::asio::ip::tcp::acceptor& acceptor) {
			acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

#ifndef _WIN32
			using reuse_port = boost::asio::detail::socket_option::boolean<BOOST_ASIO_OS_DEF(SOL_SOCKET), SO_REUSEPORT>;
			acceptor.set_option(reuse_port(true));
#endif
		}

		void BindAcceptor(
				boost::asio::ip::tcp::acceptor& acceptor,
				const boost::asio::ip::tcp::endpoint& endpoint,
				bool allowAddressReuse) {
			acceptor.open(endpoint.protocol());

			if (allowAddressReuse) {
				CATAPULT_LOG(info) << "configuring AsyncTcpServer to reuse addresses";
				EnableAddressReuse(acceptor);
			}

			acceptor.bind(endpoint);
		}

		class DefaultAsyncTcpServer
				: public AsyncTcpServer
				, public std::enable_shared_from_this<DefaultAsyncTcpServer> {
		public:
			DefaultAsyncTcpServer(
					const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
					const boost::asio::ip::tcp::endpoint& endpoint,
					const AsyncTcpServerSettings& settings)
					: m_pPool(pPool)
					, m_acceptorStrand(pPool->service())
					, m_acceptor(pPool->service())
					, m_settings(settings)
					, m_isStopped(false)
					, m_numPendingAccepts(0)
					, m_numCurrentConnections(0)
					, m_numLifetimeConnections(0) {
				BindAcceptor(m_acceptor, endpoint, m_settings.AllowAddressReuse);
				CATAPULT_LOG(info) << "AsyncTcpServer created around " << endpoint;
			}

			virtual ~DefaultAsyncTcpServer() override {
				shutdown();
			}

		public:
			uint32_t numPendingAccepts() const override { return m_numPendingAccepts; }
			uint32_t numCurrentConnections() const override { return m_numCurrentConnections; }
			uint32_t numLifetimeConnections() const override { return m_numLifetimeConnections; }

		public:
			void start() {
				m_acceptor.listen(m_settings.MaxConnections);
				tryStartAccept();

				CATAPULT_LOG(trace) << "AsyncTcpServer waiting for threads to enter pending accept state";
				while (m_numPendingAccepts < Max_Pending_Accepts) {}
				CATAPULT_LOG(info) << "AsyncTcpServer spawned " << m_numPendingAccepts << " pending accepts";
			}

			void shutdown() override {
				bool expectedIsStopped = false;
				if (!m_isStopped.compare_exchange_strong(expectedIsStopped, true))
					return;

				// close the acceptor to prevent new connections and block until the close actually happens
				CATAPULT_LOG(info) << "AsyncTcpServer stopping";
				m_acceptorStrand.dispatch([pThis = shared_from_this()]() {
					pThis->closeAcceptor();
				});

				while (0 != m_numPendingAccepts) {}
				CATAPULT_LOG(info) << "AsyncTcpServer stopped";
			}

		private:
			void closeAcceptor() {
				boost::system::error_code ignored_ec;
				m_acceptor.close(ignored_ec);
			}

			void handleAccept(const std::shared_ptr<ionet::PacketSocket>& pSocket) {
				// create an accept context and post additional handling to the strand
				auto pAcceptContext = createAcceptContext(pSocket);
				m_acceptorStrand.post([pThis = shared_from_this(), pAcceptContext]() {
					pThis->handleAcceptOnStrand(pAcceptContext);
				});
			}

			void handleAcceptOnStrand(const std::shared_ptr<DefaultAsyncTcpServerAcceptContext>& pAcceptContext) {
				// decrement the number of pending accepts
				--m_numPendingAccepts;

				// if accept had an error, try to start an accept and exit
				if (!pAcceptContext->isValid()) {
					tryStartAccept();
					return;
				}

				// if accept succeeded, increment connection counters and try to start an accept
				++m_numLifetimeConnections;
				++m_numCurrentConnections;
				tryStartAccept();

				// post the user callback on the threadpool (outside of the strand)
				m_pPool->service().post([userCallback = m_settings.Accept, pAcceptContext] {
					userCallback(pAcceptContext);
				});
			}

			std::shared_ptr<DefaultAsyncTcpServerAcceptContext> createAcceptContext(
					const std::shared_ptr<ionet::PacketSocket>& pSocket) {
				auto destructionHandler = [pThis = shared_from_this()](auto isValid) -> void {
					if (!isValid)
						return;

					// if a valid connection was wrapped, decrement the number of current connections
					// and attempt to start a new accept
					pThis->m_acceptorStrand.post([pThis] {
						pThis->handleContextDestructionOnStrand();
					});
				};
				return std::make_shared<DefaultAsyncTcpServerAcceptContext>(
						m_pPool->service(),
						pSocket,
						destructionHandler);
			}

			void handleContextDestructionOnStrand() {
				--m_numCurrentConnections;
				tryStartAccept();
			}

			// note that aside from start, which blocks, this function is always called from within a strand,
			// so no additional synchronization is necessary inside
			void tryStartAccept() {
				if (m_isStopped) {
					CATAPULT_LOG(trace) << "bypassing Accept because server is stopping";
					return;
				}

				uint32_t numActiveConnections = m_numPendingAccepts + m_numCurrentConnections;
				uint32_t numOpenConnectionSlots = m_settings.MaxActiveConnections - numActiveConnections;
				if (m_numPendingAccepts >= Max_Pending_Accepts || numOpenConnectionSlots <= 0) {
					CATAPULT_LOG(trace) << "bypassing Accept due to limit (numPendingAccepts="
							<< m_numPendingAccepts << ", numOpenConnectionSlots=" << numOpenConnectionSlots << ")";
					return;
				}

				// start a new accept
				++m_numPendingAccepts;
				ionet::Accept(
						m_acceptor,
						m_settings.PacketSocketOptions,
						m_settings.ConfigureSocket,
						[pThis = shared_from_this()](const auto& pSocket) { pThis->handleAccept(pSocket); });
			}

		private:
			std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
			boost::asio::strand m_acceptorStrand;
			boost::asio::ip::tcp::acceptor m_acceptor;

			const AsyncTcpServerSettings m_settings;
			std::atomic_bool m_isStopped;
			std::atomic<uint32_t> m_numPendingAccepts;
			std::atomic<uint32_t> m_numCurrentConnections;
			std::atomic<uint32_t> m_numLifetimeConnections;
		};
	}

	AsyncTcpServerSettings::AsyncTcpServerSettings(const AcceptHandler& accept) :
			Accept(accept),
			ConfigureSocket([](const auto&) {}),
			PacketSocketOptions(ConnectionSettings().toSocketOptions())
	{}

	std::shared_ptr<AsyncTcpServer> CreateAsyncTcpServer(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const boost::asio::ip::tcp::endpoint& endpoint,
			const AsyncTcpServerSettings& settings) {
		auto pServer = std::make_shared<DefaultAsyncTcpServer>(pPool, endpoint, settings);
		pServer->start();
		return std::move(pServer);
	}
}}
