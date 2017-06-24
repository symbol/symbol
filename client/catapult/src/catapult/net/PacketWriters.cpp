#include "PacketWriters.h"
#include "AsyncTcpServer.h"
#include "ClientConnector.h"
#include "ServerConnector.h"
#include "VerifyPeer.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/thread/TimedCallback.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/ModificationSafeIterableContainer.h"
#include "catapult/utils/SpinLock.h"
#include <list>
#include <mutex>
#include <unordered_set>

namespace catapult { namespace net {

	namespace {
		using SocketPointer = std::shared_ptr<ionet::PacketSocket>;

		struct WriterState {
		public:
			constexpr WriterState() : IsAvailable(true)
			{}

		public:
			bool IsAvailable;
			std::shared_ptr<ionet::Node> pNode;
			SocketPointer pSocket;
			std::shared_ptr<ionet::PacketIo> pBufferedIo;
			std::shared_ptr<AsyncTcpServerAcceptContext> pAcceptContext;
		};

		struct NodeHasher {
			size_t operator()(const ionet::Node& node) const {
				return utils::ArrayHasher<Key>()(node.Identity.PublicKey);
			}
		};

		bool IsStateAvailable(const WriterState& state) {
			return state.IsAvailable;
		}

		// expected sequences
		// accept : insert -> remove
		// connect: prepareConnect -> abortConnect
		// connect: prepareConnect -> insert -> remove
		class WriterContainer {
		private:
			using Writers = utils::ModificationSafeIterableContainer<std::list<WriterState>>;
			using ExclusiveLock = std::lock_guard<utils::SpinLock>;

		public:
			size_t size() const {
				ExclusiveLock guard(m_lock);
				return m_writers.size();
			}

			size_t availableSize() const {
				ExclusiveLock guard(m_lock);
				return static_cast<size_t>(std::count_if(m_writers.cbegin(), m_writers.cend(), IsStateAvailable));
			}

			template<typename THandler>
			void forEach(THandler handler) {
				ExclusiveLock guard(m_lock);
				for (const auto& state : m_writers) {
					if (!state.IsAvailable)
						continue;

					handler(state);
				}
			}

			bool pickOne(WriterState& state) {
				ExclusiveLock guard(m_lock);
				auto pState = m_writers.nextIf(IsStateAvailable);
				if (!pState)
					return false;

				pState->IsAvailable = false;
				state = *pState;
				return true;
			}

			void makeAvailable(const SocketPointer& pSocket) {
				ExclusiveLock guard(m_lock);
				auto iter = findStateBySocket(pSocket);
				if (m_writers.end() == iter)
					return;

				iter->IsAvailable = true;
			}

			bool prepareConnect(const ionet::Node& node) {
				ExclusiveLock guard(m_lock);
				if (!m_nodes.insert(node).second) {
					CATAPULT_LOG(trace) << "bypassing connection to already connected peer " << node;
					return false;
				}

				return true;
			}

			void insert(const WriterState& state) {
				ExclusiveLock guard(m_lock);
				m_writers.push_back(state);
			}

			void abortConnect(const ionet::Node& node) {
				ExclusiveLock guard(m_lock);
				CATAPULT_LOG(debug) << "aborting connection to: " << node;
				m_nodes.erase(node);
			}

			void remove(const SocketPointer& pSocket) {
				ExclusiveLock guard(m_lock);
				auto iter = findStateBySocket(pSocket);
				if (m_writers.end() == iter) {
					CATAPULT_LOG(warning) << "ignoring request to remove unknown socket";
					return;
				}

				m_nodes.erase(*iter->pNode);
				m_writers.erase(iter);
			}

			void clear() {
				ExclusiveLock guard(m_lock);
				m_nodes.clear();
				m_writers.clear();
			}

		private:
			Writers::iterator findStateBySocket(const SocketPointer& pSocket) {
				return std::find_if(m_writers.begin(), m_writers.end(), [&pSocket](const auto& state) {
					return state.pSocket == pSocket;
				});
			}

		private:
			std::unordered_set<ionet::Node, NodeHasher> m_nodes;
			Writers m_writers;
			mutable utils::SpinLock m_lock;
		};

		class ErrorHandlingPacketIo : public ionet::PacketIo {
		public:
			using ErrorCallback = std::function<void ()>;
			using CompletionCallback = std::function<void (bool)>;

		public:
			ErrorHandlingPacketIo(
					const std::shared_ptr<ionet::PacketIo>& pPacketIo,
					const ErrorCallback& errorCallback,
					const CompletionCallback& completionCallback)
					: m_pPacketIo(pPacketIo)
					, m_errorCallback(errorCallback)
					, m_completionCallback(completionCallback)
			{}

			~ErrorHandlingPacketIo() {
				m_completionCallback(true);
			}

		public:
			void read(const ReadCallback& callback) override {
				m_pPacketIo->read([callback, errorCallback = m_errorCallback](auto code, const auto* pPacket) {
					CheckError(code, errorCallback, "read");
					callback(code, pPacket);
				});
			}

			void write(const ionet::PacketPayload& payload, const WriteCallback& callback) override {
				m_pPacketIo->write(payload, [callback, errorCallback = m_errorCallback](auto code) {
					CheckError(code, errorCallback, "write");
					callback(code);
				});
			}

		private:
			static void CheckError(
					const ionet::SocketOperationCode& code,
					const ErrorCallback& handler,
					const char* operation) {
				if (ionet::SocketOperationCode::Success == code)
					return;

				CATAPULT_LOG(warning) << "calling error handler due to " << operation << " error";
				handler();
			}

		private:
			std::shared_ptr<ionet::PacketIo> m_pPacketIo;
			// note that m_errorCallback is captured by value in the read / write callbacks to ensure that it
			// is always available even if the containing ErrorHandlingPacketIo is destroyed
			ErrorCallback m_errorCallback;
			CompletionCallback m_completionCallback;
		};

		class DefaultPacketWriters
				: public PacketWriters
				, public std::enable_shared_from_this<DefaultPacketWriters> {
		public:
			DefaultPacketWriters(
					const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
					const crypto::KeyPair& keyPair,
					const ConnectionSettings& settings)
					: m_pPool(pPool)
					, m_pClientConnector(CreateClientConnector(m_pPool, keyPair, settings))
					, m_pServerConnector(CreateServerConnector(m_pPool, keyPair, settings))
					, m_networkIdentifier(settings.NetworkIdentifier)
			{}

		public:
			size_t numActiveConnections() const override {
				return m_pServerConnector->numActiveConnections() + m_pClientConnector->numActiveConnections();
			}

			size_t numActiveWriters() const override {
				return m_writers.size();
			}

			size_t numAvailableWriters() const override {
				return m_writers.availableSize();
			}

		public:
			void broadcast(const ionet::PacketPayload& payload) override {
				m_writers.forEach([pThis = shared_from_this(), payload](const auto& state) {
					state.pBufferedIo->write(payload, [pThis, pSocket = state.pSocket](auto code) {
						if (ionet::SocketOperationCode::Success == code)
							return;

						CATAPULT_LOG(warning) << "closing socket due to broadcast write error";
						pThis->removeWriter(pSocket);
					});
				});
			}

			ionet::NodePacketIoPair pickOne(const utils::TimeSpan& ioDuration) override {
				WriterState state;
				if (!m_writers.pickOne(state)) {
					CATAPULT_LOG(warning) << "no packet io available for checkout";
					return ionet::NodePacketIoPair();
				}

				CATAPULT_LOG(trace) << "checked out an io for " << ioDuration;

				// important - capture pSocket by value in order to prevent it from being removed out from under the
				// error handling packet io, also capture this for the same reason
				auto errorHandler = [pThis = shared_from_this(), pSocket = state.pSocket]() {
					pThis->removeWriter(pSocket);
				};

				auto pPacketIo = std::make_shared<ErrorHandlingPacketIo>(
						state.pBufferedIo,
						errorHandler,
						createTimedCompletionHandler(state.pSocket, ioDuration, errorHandler));
				return ionet::NodePacketIoPair(*state.pNode, pPacketIo);
			}

		private:
			ErrorHandlingPacketIo::CompletionCallback createTimedCompletionHandler(
					const SocketPointer& pSocket,
					const utils::TimeSpan& ioDuration,
					const ErrorHandlingPacketIo::ErrorCallback& errorHandler) {
				ErrorHandlingPacketIo::CompletionCallback completionHandler = [pThis = shared_from_this(), pSocket](
						auto isCompleted) {
					CATAPULT_LOG(trace) << "completed pickOne operation, success? " << isCompleted;
					pThis->makeWriterAvailable(pSocket);
				};

				auto pTimedCompletionHandler = thread::MakeTimedCallback(m_pPool->service(), completionHandler, false);
				pTimedCompletionHandler->setTimeout(ioDuration);
				pTimedCompletionHandler->setTimeoutHandler([errorHandler]() {
					CATAPULT_LOG(warning) << "calling error handler due to timeout";
					errorHandler();
				});

				return [pTimedCompletionHandler](auto isCompleted) -> void {
					pTimedCompletionHandler->callback(isCompleted);
				};
			}

		public:
			void connect(const ionet::Node& node, const ConnectCallback& callback) override {
				if (!m_writers.prepareConnect(node))
					return callback(PeerConnectResult::Already_Connected);

				auto connectCallback = [pThis = shared_from_this(), node, callback](auto result, const auto& pSocket) {
					if (PeerConnectResult::Accepted == result)
						pThis->addWriter(std::make_shared<ionet::Node>(node), pSocket, nullptr);
					else
						pThis->m_writers.abortConnect(node);

					callback(result);
				};
				m_pServerConnector->connect(node, connectCallback);
			}

			void accept(
					const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext,
					const ConnectCallback& callback) override {
				auto acceptCallback = [pThis = shared_from_this(), pAcceptContext, callback](auto result, const auto& remoteKey) {
					if (PeerConnectResult::Accepted == result)
						pThis->addWriter(remoteKey, pAcceptContext->socket(), pAcceptContext);

					callback(result);
				};
				m_pClientConnector->accept(pAcceptContext, acceptCallback);
			}

		private:
			void addWriter(
					const Key& key,
					const SocketPointer& pSocket,
					const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext) {
				auto pNode = std::make_shared<ionet::Node>(
						ionet::NodeEndpoint(),
						ionet::NodeIdentity{ key, "" },
						m_networkIdentifier);
				addWriter(pNode, pSocket, pAcceptContext);
			}

			void addWriter(
					const std::shared_ptr<ionet::Node>& pNode,
					const SocketPointer& pSocket,
					const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext) {
				WriterState state;
				state.pNode = pNode;
				state.pSocket = pSocket;
				state.pAcceptContext = pAcceptContext;
				state.pBufferedIo = ionet::CreateBufferedPacketIo(pSocket, boost::asio::strand(m_pPool->service()));
				m_writers.insert(state);
			}

			void makeWriterAvailable(const SocketPointer& pSocket) {
				m_writers.makeAvailable(pSocket);
			}

			void removeWriter(const SocketPointer& pSocket) {
				pSocket->close();
				m_writers.remove(pSocket);
			}

		public:
			void shutdown() override {
				CATAPULT_LOG(info) << "closing all connections in PacketWriters";
				m_pClientConnector->shutdown();
				m_pServerConnector->shutdown();
				m_writers.clear();
			}

		private:
			std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
			std::shared_ptr<ClientConnector> m_pClientConnector;
			std::shared_ptr<ServerConnector> m_pServerConnector;
			model::NetworkIdentifier m_networkIdentifier;
			WriterContainer m_writers;
		};
	}

	std::shared_ptr<PacketWriters> CreatePacketWriters(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings) {
		return std::make_shared<DefaultPacketWriters>(pPool, keyPair, settings);
	}
}}
