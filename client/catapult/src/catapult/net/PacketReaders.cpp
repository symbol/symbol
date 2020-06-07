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

#include "PacketReaders.h"
#include "ChainedSocketReader.h"
#include "ClientConnector.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/ionet/SocketReader.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/SpinLock.h"
#include <unordered_map>

namespace catapult { namespace net {

	namespace {
		using PacketSocketPointer = std::shared_ptr<ionet::PacketSocket>;

		struct ReaderState {
		public:
			model::NodeIdentity Identity;
			std::shared_ptr<ionet::PacketIo> pBufferedIo;
			std::weak_ptr<ChainedSocketReader> pReader;
		};

		class ReaderContainer {
		private:
			using ChainedSocketReaderFactory = std::function<std::shared_ptr<ChainedSocketReader> (const PacketSocketPointer&, uint32_t)>;

		public:
			ReaderContainer(uint32_t maxConnectionsPerIdentity, model::NodeIdentityEqualityStrategy equalityStrategy)
					: m_maxConnectionsPerIdentity(maxConnectionsPerIdentity)
					, m_equalityStrategy(equalityStrategy)
					, m_readers(0, IdentityIdPairHasher(equalityStrategy), IdentityIdPairEquality(equalityStrategy))
			{}

		public:
			size_t size() const {
				utils::SpinLockGuard guard(m_lock);
				return m_readers.size();
			}

			model::NodeIdentitySet identities() const {
				auto activeIdentities = model::CreateNodeIdentitySet(m_equalityStrategy);

				utils::SpinLockGuard guard(m_lock);
				for (const auto& pair : m_readers)
					activeIdentities.emplace(pair.second.Identity);

				return activeIdentities;
			}

		public:
			struct InsertOperation {
			public:
				explicit InsertOperation(bool isPending)
						: IsPending(isPending)
						, Abort([]() {})
				{}

			public:
				bool IsPending;
				action Commit;
				action Abort;
			};

		public:
			InsertOperation insert(
					const model::NodeIdentity& identity,
					const PacketSocketPointer& pSocket,
					const ChainedSocketReaderFactory& readerFactory) {
				ReaderState state;
				state.Identity = identity;
				state.pBufferedIo = pSocket->buffered();

				utils::SpinLockGuard guard(m_lock);

				std::pair<model::NodeIdentity, uint32_t> qualifiedReaderId;
				auto insertedReaderIter = m_readers.end();
				for (auto i = 0u; i < m_maxConnectionsPerIdentity; ++i) {
					qualifiedReaderId = std::make_pair(state.Identity, i);
					auto emplaceResult = m_readers.emplace(qualifiedReaderId, state);
					if (emplaceResult.second) {
						insertedReaderIter = emplaceResult.first;
						break;
					}
				}

				if (m_readers.end() == insertedReaderIter) {
					// all available connections for the current identity are used up
					CATAPULT_LOG(warning) << "rejecting incoming connection from " << identity << " (max connections in use)";
					pSocket->close();
					return InsertOperation(false);
				}

				// the reader takes ownership of the socket
				auto pReader = readerFactory(pSocket, insertedReaderIter->first.second);
				insertedReaderIter->second.pReader = pReader;

				InsertOperation operation(true);
				operation.Commit = [pReader]() {
					pReader->start();
				};
				operation.Abort = [this, qualifiedReaderId]() {
					CATAPULT_LOG(warning) << "aborting incoming connection from " << qualifiedReaderId.first;

					utils::SpinLockGuard guard2(m_lock);
					closeSingle(qualifiedReaderId.first, qualifiedReaderId.second);
				};

				return operation;
			}

			bool close(const model::NodeIdentity& identity) {
				utils::SpinLockGuard guard(m_lock);

				bool anyClosed = false;
				for (auto i = 0u; i < m_maxConnectionsPerIdentity; ++i)
					anyClosed = closeSingle(identity, i) || anyClosed;

				return anyClosed;
			}

			bool close(const model::NodeIdentity& identity, uint32_t id) {
				utils::SpinLockGuard guard(m_lock);
				return closeSingle(identity, id);
			}

			void clear() {
				utils::SpinLockGuard guard(m_lock);
				for (auto& pair : m_readers)
					stop(pair.second);

				m_readers.clear();
			}

		private:
			void stop(const ReaderState& state) {
				auto pReader = state.pReader.lock();
				if (!pReader)
					return;

				pReader->stop();
			}

			// closeSingle needs to be called with lock
			bool closeSingle(const model::NodeIdentity& identity, uint32_t id) {
				auto iter = m_readers.find(std::make_pair(identity, id));
				if (m_readers.end() == iter)
					return false;

				CATAPULT_LOG(debug) << "closing connection to " << identity << " - " << id;
				auto state = iter->second;
				m_readers.erase(iter);
				stop(state);
				return true;
			}

		private:
			using IdentityIdPair = std::pair<model::NodeIdentity, uint32_t>;

			class IdentityIdPairEquality {
			public:
				explicit IdentityIdPairEquality(model::NodeIdentityEqualityStrategy strategy) : m_equality(strategy)
				{}

			public:
				bool operator()(const IdentityIdPair& lhs, const IdentityIdPair& rhs) const {
					return m_equality(lhs.first, rhs.first) && lhs.second == rhs.second;
				}

			private:
				model::NodeIdentityEquality m_equality;
			};

			class IdentityIdPairHasher {
			public:
				explicit IdentityIdPairHasher(model::NodeIdentityEqualityStrategy strategy) : m_hasher(strategy)
				{}

			public:
				size_t operator()(const IdentityIdPair& pair) const {
					return m_hasher(pair.first);
				}

			private:
				model::NodeIdentityHasher m_hasher;
			};

			using Readers = std::unordered_map<IdentityIdPair, ReaderState, IdentityIdPairHasher, IdentityIdPairEquality>;

		private:
			uint32_t m_maxConnectionsPerIdentity;
			model::NodeIdentityEqualityStrategy m_equalityStrategy;
			Readers m_readers;
			mutable utils::SpinLock m_lock;
		};

		class DefaultPacketReaders : public PacketReaders, public std::enable_shared_from_this<DefaultPacketReaders> {
		public:
			DefaultPacketReaders(
					thread::IoThreadPool& pool,
					const ionet::ServerPacketHandlers& handlers,
					const Key& serverPublicKey,
					const ConnectionSettings& settings,
					uint32_t maxConnectionsPerIdentity)
					: m_handlers(handlers)
					, m_pClientConnector(CreateClientConnector(pool, serverPublicKey, settings, "readers"))
					, m_readers(maxConnectionsPerIdentity, settings.NodeIdentityEqualityStrategy)
			{}

		public:
			size_t numActiveConnections() const override {
				return m_pClientConnector->numActiveConnections();
			}

			size_t numActiveReaders() const override {
				return m_readers.size();
			}

			model::NodeIdentitySet identities() const override {
				return m_readers.identities();
			}

		public:
			void accept(const ionet::PacketSocketInfo& socketInfo, const AcceptCallback& callback) override {
				m_pClientConnector->accept(socketInfo, [pThis = shared_from_this(), host = socketInfo.host(), callback](
						auto connectCode,
						const auto& pVerifiedSocket,
						const auto& identityKey) {
					auto connectResult = PeerConnectResult{ connectCode, { identityKey, host } };
					if (PeerConnectCode::Accepted != connectCode) {
						callback(connectResult);
						return;
					}

					ionet::PacketSocketInfo verifiedSocketInfo(host, identityKey, pVerifiedSocket);
					auto operation = pThis->tryAddReader(identityKey, verifiedSocketInfo);
					if (operation.IsPending) {
						if (callback(connectResult)) {
							CATAPULT_LOG(debug) << "accepted connection from '" << verifiedSocketInfo.host() << "' as " << identityKey;
							operation.Commit();
							return;
						}
					} else {
						connectResult.Code = PeerConnectCode::Already_Connected;
						callback(connectResult);
					}

					operation.Abort();
				});
			}

			bool closeOne(const model::NodeIdentity& identity) override {
				CATAPULT_LOG(debug) << "closing all connections from " << identity;
				return m_readers.close(identity);
			}

			void shutdown() override {
				CATAPULT_LOG(info) << "closing all connections in PacketReaders";
				m_pClientConnector->shutdown();
				m_readers.clear();
			}

		private:
			ReaderContainer::InsertOperation tryAddReader(const Key& identityKey, const ionet::PacketSocketInfo& socketInfo) {
				auto identity = model::NodeIdentity{ identityKey, socketInfo.host() };
				return m_readers.insert(identity, socketInfo.socket(), [this, &identity](const auto& pSocket, auto id) {
					return this->createReader(pSocket, identity, id);
				});
			}

			std::shared_ptr<ChainedSocketReader> createReader(
					const PacketSocketPointer& pSocket,
					const model::NodeIdentity& identity,
					uint32_t id) {
				return CreateChainedSocketReader(pSocket, m_handlers, identity, [pThis = shared_from_this(), identity, id](auto code) {
					// if the socket is closed cleanly, just remove the closed socket
					// if the socket errored, remove all sockets with the same identity
					if (ionet::SocketOperationCode::Closed == code)
						pThis->m_readers.close(identity, id);
					else
						pThis->m_readers.close(identity);
				});
			}

		private:
			ionet::ServerPacketHandlers m_handlers;
			std::shared_ptr<ClientConnector> m_pClientConnector;
			ReaderContainer m_readers;
		};
	}

	std::shared_ptr<PacketReaders> CreatePacketReaders(
			thread::IoThreadPool& pool,
			const ionet::ServerPacketHandlers& handlers,
			const Key& serverPublicKey,
			const ConnectionSettings& settings,
			uint32_t maxConnectionsPerIdentity) {
		return std::make_shared<DefaultPacketReaders>(pool, handlers, serverPublicKey, settings, maxConnectionsPerIdentity);
	}
}}
