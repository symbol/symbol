#include "PacketWriters.h"
#include "ClientConnector.h"
#include "ServerConnector.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/thread/TimedCallback.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/ModificationSafeIterableContainer.h"
#include "catapult/utils/SpinLock.h"
#include "catapult/utils/ThrottleLogger.h"
#include <list>

namespace catapult { namespace net {

	namespace {
		using SocketPointer = std::shared_ptr<ionet::PacketSocket>;

		struct WriterState {
		public:
			WriterState() : IsAvailable(true)
			{}

		public:
			bool IsAvailable;
			ionet::Node Node;
			SocketPointer pSocket;
			std::shared_ptr<ionet::PacketIo> pBufferedIo;
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

		public:
			size_t size() const {
				utils::SpinLockGuard guard(m_lock);
				return m_writers.size();
			}

			size_t numOutgoingConnections() const {
				utils::SpinLockGuard guard(m_lock);
				return m_outgoingNodeIdentityKeys.size();
			}

			size_t availableSize() const {
				utils::SpinLockGuard guard(m_lock);
				return static_cast<size_t>(std::count_if(m_writers.cbegin(), m_writers.cend(), IsStateAvailable));
			}

			utils::KeySet identities() const {
				utils::SpinLockGuard guard(m_lock);
				return m_nodeIdentityKeys;
			}

			template<typename THandler>
			void forEach(THandler handler) {
				utils::SpinLockGuard guard(m_lock);
				for (const auto& state : m_writers) {
					if (!state.IsAvailable)
						continue;

					handler(state);
				}
			}

			bool pickOne(WriterState& state) {
				utils::SpinLockGuard guard(m_lock);
				auto* pState = m_writers.nextIf(IsStateAvailable);
				if (!pState)
					return false;

				pState->IsAvailable = false;
				state = *pState;
				return true;
			}

			void makeAvailable(const SocketPointer& pSocket) {
				utils::SpinLockGuard guard(m_lock);
				auto iter = findStateBySocket(pSocket);
				if (m_writers.end() == iter)
					return;

				iter->IsAvailable = true;
			}

			bool prepareConnect(const ionet::Node& node) {
				utils::SpinLockGuard guard(m_lock);
				if (!m_outgoingNodeIdentityKeys.insert(node.identityKey()).second) {
					CATAPULT_LOG(debug) << "bypassing connection to already connected peer " << node;
					return false;
				}

				return true;
			}

			bool insert(const WriterState& state) {
				utils::SpinLockGuard guard(m_lock);

				// if the state is for an already connected node, ignore it
				// 1. required for filtering accepted connections
				// 2. prepareConnect proactively filters connections
				// 3. failsafe for mixed connect + accept use cases (currently unused, so not optimized)
				if (!m_nodeIdentityKeys.emplace(state.Node.identityKey()).second) {
					CATAPULT_LOG(debug) << "ignoring connection to already connected peer " << state.Node;
					return false;
				}

				m_writers.push_back(state);
				return true;
			}

			void abortConnect(const ionet::Node& node) {
				utils::SpinLockGuard guard(m_lock);
				CATAPULT_LOG(debug) << "aborting connection to: " << node;
				m_outgoingNodeIdentityKeys.erase(node.identityKey());
			}

			void remove(const SocketPointer& pSocket) {
				utils::SpinLockGuard guard(m_lock);
				auto iter = findStateBySocket(pSocket);
				if (m_writers.end() == iter) {
					CATAPULT_LOG(warning) << "ignoring request to remove unknown socket";
					return;
				}

				remove(iter);
			}

			bool close(const Key& identityKey) {
				utils::SpinLockGuard guard(m_lock);
				auto iter = findStateByKey(identityKey);
				if (m_writers.end() == iter)
					return false;

				CATAPULT_LOG(debug) << "closing connection to " << utils::HexFormat(identityKey);
				iter->pSocket->close();
				remove(iter);
				return true;
			}

			void clear() {
				utils::SpinLockGuard guard(m_lock);
				m_nodeIdentityKeys.clear();
				m_outgoingNodeIdentityKeys.clear();
				m_writers.clear();
			}

		private:
			Writers::iterator findStateBySocket(const SocketPointer& pSocket) {
				return std::find_if(m_writers.begin(), m_writers.end(), [&pSocket](const auto& state) {
					return state.pSocket == pSocket;
				});
			}

			Writers::iterator findStateByKey(const Key& identityKey) {
				return std::find_if(m_writers.begin(), m_writers.end(), [&identityKey](const auto& state) {
					return state.Node.identityKey() == identityKey;
				});
			}

			void remove(Writers::iterator iter) {
				const auto& identityKey = iter->Node.identityKey();
				m_nodeIdentityKeys.erase(identityKey);
				m_outgoingNodeIdentityKeys.erase(identityKey);
				m_writers.erase(iter);
			}

		private:
			utils::KeySet m_nodeIdentityKeys; // keys of active writers (both connected AND accepted)
			utils::KeySet m_outgoingNodeIdentityKeys; // keys of connecting or connected writers
			Writers m_writers;
			mutable utils::SpinLock m_lock;
		};

		class ErrorHandlingPacketIo : public ionet::PacketIo {
		public:
			using ErrorCallback = action;
			using CompletionCallback = consumer<bool>;

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
			static void CheckError(ionet::SocketOperationCode code, const ErrorCallback& handler, const char* operation) {
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
				// use m_writers.numOutgoingConnections() instead of m_pServerConnector->numActiveConnections() because the latter
				// does not count pending connections (before socket CONNECT succeeds)
				return m_writers.numOutgoingConnections() + m_pClientConnector->numActiveConnections();
			}

			size_t numActiveWriters() const override {
				return m_writers.size();
			}

			size_t numAvailableWriters() const override {
				return m_writers.availableSize();
			}

			utils::KeySet identities() const override {
				return m_writers.identities();
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
					CATAPULT_LOG_THROTTLE(warning, 60'000) << "no packet io available for checkout";
					return ionet::NodePacketIoPair();
				}

				// important - capture pSocket by value in order to prevent it from being removed out from under the
				// error handling packet io, also capture this for the same reason
				auto errorHandler = [pThis = shared_from_this(), pSocket = state.pSocket, node = state.Node]() {
					CATAPULT_LOG(warning) << "error handler triggered for " << node;
					pThis->removeWriter(pSocket);
				};

				auto pPacketIo = std::make_shared<ErrorHandlingPacketIo>(
						state.pBufferedIo,
						errorHandler,
						createTimedCompletionHandler(state.pSocket, ioDuration, errorHandler));

				CATAPULT_LOG(trace) << "checked out an io for " << ioDuration;
				return ionet::NodePacketIoPair(state.Node, pPacketIo);
			}

		private:
			ErrorHandlingPacketIo::CompletionCallback createTimedCompletionHandler(
					const SocketPointer& pSocket,
					const utils::TimeSpan& ioDuration,
					const ErrorHandlingPacketIo::ErrorCallback& errorHandler) {
				ErrorHandlingPacketIo::CompletionCallback completionHandler = [pThis = shared_from_this(), pSocket](auto isCompleted) {
					CATAPULT_LOG(trace) << "completed pickOne operation, success? " << isCompleted;
					pThis->makeWriterAvailable(pSocket);
				};

				auto pTimedCompletionHandler = thread::MakeTimedCallback(m_pPool->service(), completionHandler, false);
				pTimedCompletionHandler->setTimeout(ioDuration);
				pTimedCompletionHandler->setTimeoutHandler([errorHandler]() {
					CATAPULT_LOG(warning) << "calling error handler due to timeout";
					errorHandler();
				});

				return [pTimedCompletionHandler](auto isCompleted) {
					pTimedCompletionHandler->callback(isCompleted);
				};
			}

		public:
			void connect(const ionet::Node& node, const ConnectCallback& callback) override {
				if (!m_writers.prepareConnect(node))
					return callback(PeerConnectResult::Already_Connected);

				auto connectCallback = [pThis = shared_from_this(), node, callback](auto result, const auto& pSocket) {
					// abort the connection if it failed or is redundant
					if (PeerConnectResult::Accepted != result || !pThis->addWriter(node, pSocket)) {
						pThis->m_writers.abortConnect(node);

						if (PeerConnectResult::Accepted == result)
							result = PeerConnectResult::Already_Connected;
					}

					callback(result);
				};
				m_pServerConnector->connect(node, connectCallback);
			}

			void accept(const std::shared_ptr<ionet::PacketSocket>& pPacketSocket, const ConnectCallback& callback) override {
				auto acceptCallback = [pThis = shared_from_this(), pPacketSocket, callback](auto result, const auto& remoteKey) {
					if (PeerConnectResult::Accepted == result) {
						if (!pThis->addWriter(remoteKey, pPacketSocket))
							result = PeerConnectResult::Already_Connected;
					}

					callback(result);
				};
				m_pClientConnector->accept(pPacketSocket, acceptCallback);
			}

		private:
			bool addWriter(const Key& key, const SocketPointer& pSocket) {
				// this is for supporting eventsource extension where api writers register to receive pushed data
				// endpoint and metadata are unimportant because only key-based filtering is required
				auto node = ionet::Node(key, ionet::NodeEndpoint(), ionet::NodeMetadata(m_networkIdentifier));
				return addWriter(node, pSocket);
			}

			bool addWriter(const ionet::Node& node, const SocketPointer& pSocket) {
				WriterState state;
				state.Node = node;
				state.pSocket = pSocket;
				state.pBufferedIo = pSocket->buffered();
				return m_writers.insert(state);
			}

			void makeWriterAvailable(const SocketPointer& pSocket) {
				m_writers.makeAvailable(pSocket);
			}

			void removeWriter(const SocketPointer& pSocket) {
				pSocket->close();
				m_writers.remove(pSocket);
			}

		public:
			bool closeOne(const Key& identityKey) override {
				return m_writers.close(identityKey);
			}

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
