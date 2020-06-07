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

#include "PacketWriters.h"
#include "ClientConnector.h"
#include "ServerConnector.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/IoThreadPool.h"
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
			ionet::Node Node;
			SocketPointer pSocket;
			std::shared_ptr<ionet::PacketIo> pBufferedIo;
		};

		struct WriterStateWithAvailability : public WriterState {
		public:
			WriterStateWithAvailability() : IsAvailable(true)
			{}

			explicit WriterStateWithAvailability(const WriterState& state) : WriterStateWithAvailability() {
				Node = state.Node;
				pSocket = state.pSocket;
				pBufferedIo = state.pBufferedIo;
			}

		public:
			// atomic because addUnexpectedDataHook reads IsAvailable outside of (WriterContainer) lock context
			std::atomic_bool IsAvailable;
		};

		bool IsStateAvailable(const std::shared_ptr<WriterStateWithAvailability>& pState) {
			return pState->IsAvailable;
		}

		// expected sequences
		// connect: prepareConnect -> abortConnect
		// connect: prepareConnect -> insert -> remove
		class WriterContainer {
		private:
			using Writers = utils::ModificationSafeIterableContainer<std::list<std::shared_ptr<WriterStateWithAvailability>>>;

		public:
			explicit WriterContainer(model::NodeIdentityEqualityStrategy equalityStrategy)
					: m_equalityStrategy(equalityStrategy)
					, m_nodeIdentities(CreateNodeIdentitySet(equalityStrategy))
					, m_outgoingNodeIdentities(CreateNodeIdentitySet(equalityStrategy))
			{}

		public:
			size_t size() const {
				utils::SpinLockGuard guard(m_lock);
				return m_writers.size();
			}

			size_t numOutgoingConnections() const {
				utils::SpinLockGuard guard(m_lock);
				return m_outgoingNodeIdentities.size();
			}

			size_t availableSize() const {
				utils::SpinLockGuard guard(m_lock);
				return static_cast<size_t>(std::count_if(m_writers.cbegin(), m_writers.cend(), IsStateAvailable));
			}

			model::NodeIdentitySet identities() const {
				utils::SpinLockGuard guard(m_lock);
				return m_nodeIdentities;
			}

			template<typename THandler>
			void forEach(THandler handler) {
				utils::SpinLockGuard guard(m_lock);
				for (const auto& pState : m_writers) {
					if (!pState->IsAvailable)
						continue;

					handler(*pState);
				}
			}

			bool pickOne(WriterState& state) {
				utils::SpinLockGuard guard(m_lock);
				auto* ppState = m_writers.nextIf(IsStateAvailable);
				if (!ppState)
					return false;

				(*ppState)->IsAvailable = false;
				state = *(*ppState);
				return true;
			}

			void makeAvailable(const SocketPointer& pSocket) {
				utils::SpinLockGuard guard(m_lock);
				auto iter = findStateBySocket(pSocket);
				if (m_writers.end() == iter)
					return;

				(*iter)->IsAvailable = true;
				addUnexpectedDataHook(*iter);
			}

			bool prepareConnect(const ionet::Node& node) {
				utils::SpinLockGuard guard(m_lock);
				if (!m_outgoingNodeIdentities.insert(node.identity()).second) {
					CATAPULT_LOG(debug) << "bypassing connection to already connected peer " << node;
					return false;
				}

				return true;
			}

			void insert(const WriterState& state) {
				utils::SpinLockGuard guard(m_lock);

				m_nodeIdentities.emplace(state.Node.identity());

				auto pNewState = std::make_shared<WriterStateWithAvailability>(state);
				m_writers.push_back(pNewState);
				addUnexpectedDataHook(pNewState);
			}

			void abortConnect(const ionet::Node& node) {
				utils::SpinLockGuard guard(m_lock);
				CATAPULT_LOG(debug) << "aborting connection to: " << node;
				m_outgoingNodeIdentities.erase(node.identity());
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

			bool close(const model::NodeIdentity& identity) {
				utils::SpinLockGuard guard(m_lock);
				auto iter = findStateByIdentity(identity);
				if (m_writers.end() == iter)
					return false;

				CATAPULT_LOG(debug) << "closing connection to " << identity;
				(*iter)->pSocket->close();
				remove(iter);
				return true;
			}

			void clear() {
				utils::SpinLockGuard guard(m_lock);
				m_nodeIdentities.clear();
				m_outgoingNodeIdentities.clear();
				m_writers.clear();
			}

		private:
			Writers::iterator findStateBySocket(const SocketPointer& pSocket) {
				return std::find_if(m_writers.begin(), m_writers.end(), [&pSocket](const auto& pState) {
					return pState->pSocket == pSocket;
				});
			}

			Writers::iterator findStateByIdentity(const model::NodeIdentity& identity) {
				auto equality = model::NodeIdentityEquality(m_equalityStrategy);
				return std::find_if(m_writers.begin(), m_writers.end(), [&identity, equality](const auto& pState) {
					return equality(pState->Node.identity(), identity);
				});
			}

			void addUnexpectedDataHook(const std::shared_ptr<WriterStateWithAvailability>& pState) {
				pState->pSocket->waitForData([pState]() {
					if (!pState->IsAvailable)
						return;

					CATAPULT_LOG(warning) << "closing connection to " << pState->Node << " due to unexpected data";
					pState->pSocket->close();
				});
			}

			void remove(Writers::iterator iter) {
				const auto& identity = (*iter)->Node.identity();
				m_nodeIdentities.erase(identity);
				m_outgoingNodeIdentities.erase(identity);
				m_writers.erase(iter);
			}

		private:
			model::NodeIdentityEqualityStrategy m_equalityStrategy;
			model::NodeIdentitySet m_nodeIdentities; // identities of active writers (both connected AND accepted)
			model::NodeIdentitySet m_outgoingNodeIdentities; // identities of connecting or connected writers
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

			~ErrorHandlingPacketIo() override {
				m_completionCallback(true);
			}

		public:
			void write(const ionet::PacketPayload& payload, const WriteCallback& callback) override {
				m_pPacketIo->write(payload, [callback, errorCallback = m_errorCallback](auto code) {
					CheckError(code, errorCallback, "write");
					callback(code);
				});
			}

			void read(const ReadCallback& callback) override {
				m_pPacketIo->read([callback, errorCallback = m_errorCallback](auto code, const auto* pPacket) {
					CheckError(code, errorCallback, "read");
					callback(code, pPacket);
				});
			}

		private:
			static void CheckError(ionet::SocketOperationCode code, const ErrorCallback& handler, const char* operation) {
				if (ionet::SocketOperationCode::Success == code)
					return;

				CATAPULT_LOG(warning) << "calling error handler due to " << operation << " error (" << code << ")";
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
			DefaultPacketWriters(thread::IoThreadPool& pool, const Key& serverPublicKey, const ConnectionSettings& settings)
					: m_ioContext(pool.ioContext())
					, m_pClientConnector(CreateClientConnector(pool, serverPublicKey, settings, "writers"))
					, m_pServerConnector(CreateServerConnector(pool, serverPublicKey, settings, "writers"))
					, m_writers(settings.NodeIdentityEqualityStrategy)
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

			model::NodeIdentitySet identities() const override {
				return m_writers.identities();
			}

		public:
			void broadcast(const ionet::PacketPayload& payload) override {
				m_writers.forEach([pThis = shared_from_this(), payload](const auto& state) {
					state.pBufferedIo->write(payload, [pThis, pSocket = state.pSocket](auto code) {
						if (ionet::SocketOperationCode::Success == code)
							return;

						CATAPULT_LOG(warning) << "closing socket due to broadcast write error (" << code << ")";
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

				auto pTimedCompletionHandler = thread::MakeTimedCallback(m_ioContext, completionHandler, false);
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
					return callback(PeerConnectCode::Already_Connected);

				m_pServerConnector->connect(node, [pThis = shared_from_this(), node, callback](
						auto connectCode,
						const auto& verifiedSocketInfo) {
					// abort the connection if it failed
					if (PeerConnectCode::Accepted != connectCode)
						pThis->m_writers.abortConnect(node);
					else
						pThis->addWriter(node, verifiedSocketInfo.socket());

					// PacketWritersTests require socket
					callback({ connectCode, node.identity(), verifiedSocketInfo.socket() });
				});
			}

		private:
			void addWriter(const ionet::Node& node, const SocketPointer& pSocket) {
				WriterState state;
				state.Node = node;
				state.pSocket = pSocket;
				state.pBufferedIo = pSocket->buffered();
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
			bool closeOne(const model::NodeIdentity& identity) override {
				CATAPULT_LOG(debug) << "closing all connections from " << identity;
				return m_writers.close(identity);
			}

			void shutdown() override {
				CATAPULT_LOG(info) << "closing all connections in PacketWriters";
				m_pClientConnector->shutdown();
				m_pServerConnector->shutdown();
				m_writers.clear();
			}

		private:
			boost::asio::io_context& m_ioContext;
			std::shared_ptr<ClientConnector> m_pClientConnector;
			std::shared_ptr<ServerConnector> m_pServerConnector;
			WriterContainer m_writers;
		};
	}

	std::shared_ptr<PacketWriters> CreatePacketWriters(
			thread::IoThreadPool& pool,
			const Key& serverPublicKey,
			const ConnectionSettings& settings) {
		return std::make_shared<DefaultPacketWriters>(pool, serverPublicKey, settings);
	}
}}
