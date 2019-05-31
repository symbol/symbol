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

namespace catapult { namespace net {

	namespace {
		using PacketSocketPointer = std::shared_ptr<ionet::PacketSocket>;

		struct ReaderState {
		public:
			Key PublicKey;
			std::shared_ptr<ionet::PacketIo> pBufferedIo;
			std::weak_ptr<ChainedSocketReader> pReader;
		};

		class ReaderContainer {
		private:
			using ChainedSocketReaderFactory = std::function<std::shared_ptr<ChainedSocketReader> (const PacketSocketPointer&, uint32_t)>;

		public:
			explicit ReaderContainer(uint32_t maxConnectionsPerIdentity) : m_maxConnectionsPerIdentity(maxConnectionsPerIdentity)
			{}

		public:
			size_t size() const {
				utils::SpinLockGuard guard(m_lock);
				return m_readers.size();
			}

			utils::KeySet identities() const {
				utils::SpinLockGuard guard(m_lock);
				utils::KeySet activeIdentities;
				for (const auto& pair : m_readers)
					activeIdentities.emplace(pair.second.PublicKey);

				return activeIdentities;
			}

			bool insert(const Key& identityKey, const PacketSocketPointer& pSocket, const ChainedSocketReaderFactory& readerFactory) {
				ReaderState state;
				state.PublicKey = identityKey;
				state.pBufferedIo = pSocket->buffered();

				utils::SpinLockGuard guard(m_lock);

				auto insertedReaderIter = m_readers.end();
				for (auto i = 0u; i < m_maxConnectionsPerIdentity; ++i) {
					auto emplaceResult = m_readers.emplace(std::make_pair(state.PublicKey, i), state);
					if (emplaceResult.second) {
						insertedReaderIter = emplaceResult.first;
						break;
					}
				}

				if (m_readers.end() == insertedReaderIter) {
					// all available connections for the current identity are used up
					pSocket->close();
					return false;
				}

				// the reader takes ownership of the socket
				auto pReader = readerFactory(pSocket, insertedReaderIter->first.second);
				pReader->start();
				insertedReaderIter->second.pReader = pReader;
				return true;
			}

			bool close(const Key& identityKey) {
				utils::SpinLockGuard guard(m_lock);

				bool anyClosed = false;
				for (auto i = 0u; i < m_maxConnectionsPerIdentity; ++i)
					anyClosed = closeSingle(identityKey, i) || anyClosed;

				return anyClosed;
			}

			bool close(const Key& identityKey, uint32_t id) {
				utils::SpinLockGuard guard(m_lock);
				return closeSingle(identityKey, id);
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

			bool closeSingle(const Key& identityKey, uint32_t id) {
				auto iter = m_readers.find(std::make_pair(identityKey, id));
				if (m_readers.end() == iter)
					return false;

				CATAPULT_LOG(debug) << "closing connection to " << identityKey << " - " << id;
				auto state = iter->second;
				m_readers.erase(iter);
				stop(state);
				return true;
			}

		private:
			struct KeyIdPairHasher {
				size_t operator()(const std::pair<Key, uint32_t>& value) const {
					return utils::ArrayHasher<Key>()(value.first);
				}
			};

			using Readers = std::unordered_map<std::pair<Key, uint32_t>, ReaderState, KeyIdPairHasher>;

		private:
			uint32_t m_maxConnectionsPerIdentity;
			Readers m_readers;
			mutable utils::SpinLock m_lock;
		};

		class DefaultPacketReaders : public PacketReaders, public std::enable_shared_from_this<DefaultPacketReaders> {
		public:
			explicit DefaultPacketReaders(
					const std::shared_ptr<thread::IoThreadPool>& pPool,
					const ionet::ServerPacketHandlers& handlers,
					const crypto::KeyPair& keyPair,
					const ConnectionSettings& settings,
					uint32_t maxConnectionsPerIdentity)
					: m_handlers(handlers)
					, m_pClientConnector(CreateClientConnector(pPool, keyPair, settings))
					, m_readers(maxConnectionsPerIdentity)
			{}

		public:
			size_t numActiveConnections() const override {
				return m_pClientConnector->numActiveConnections();
			}

			size_t numActiveReaders() const override {
				return m_readers.size();
			}

			utils::KeySet identities() const override {
				return m_readers.identities();
			}

		public:
			void accept(const ionet::AcceptedPacketSocketInfo& socketInfo, const AcceptCallback& callback) override {
				m_pClientConnector->accept(socketInfo.socket(), [pThis = shared_from_this(), host = socketInfo.host(), callback](
						auto connectCode,
						const auto& pVerifiedSocket,
						const auto& identityKey) {
					ionet::AcceptedPacketSocketInfo verifiedSocketInfo(host, pVerifiedSocket);
					if (PeerConnectCode::Accepted == connectCode) {
						if (!pThis->addReader(identityKey, verifiedSocketInfo))
							connectCode = PeerConnectCode::Already_Connected;
						else
							CATAPULT_LOG(debug) << "accepted connection from '" << verifiedSocketInfo.host() << "' as " << identityKey;
					}

					return callback({ connectCode, identityKey });
				});
			}

			bool closeOne(const Key& identityKey) override {
				return m_readers.close(identityKey);
			}

			void shutdown() override {
				CATAPULT_LOG(info) << "closing all connections in PacketReaders";
				m_pClientConnector->shutdown();
				m_readers.clear();
			}

		private:
			bool addReader(const Key& identityKey, const ionet::AcceptedPacketSocketInfo& socketInfo) {
				auto identity = ionet::ReaderIdentity{ identityKey, socketInfo.host() };
				return m_readers.insert(identityKey, socketInfo.socket(), [this, &identity](const auto& pSocket, auto id) {
					return this->createReader(pSocket, identity, id);
				});
			}

			std::shared_ptr<ChainedSocketReader> createReader(
					const PacketSocketPointer& pSocket,
					const ionet::ReaderIdentity& identity,
					uint32_t id) {
				const auto& identityKey = identity.PublicKey;
				return CreateChainedSocketReader(pSocket, m_handlers, identity, [pThis = shared_from_this(), identityKey, id](auto code) {
					// if the socket is closed cleanly, just remove the closed socket
					// if the socket errored, remove all sockets with the same identity
					if (ionet::SocketOperationCode::Closed == code)
						pThis->m_readers.close(identityKey, id);
					else
						pThis->m_readers.close(identityKey);
				});
			}

		private:
			ionet::ServerPacketHandlers m_handlers;
			std::shared_ptr<ClientConnector> m_pClientConnector;
			ReaderContainer m_readers;
		};
	}

	std::shared_ptr<PacketReaders> CreatePacketReaders(
			const std::shared_ptr<thread::IoThreadPool>& pPool,
			const ionet::ServerPacketHandlers& handlers,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings,
			uint32_t maxConnectionsPerIdentity) {
		return std::make_shared<DefaultPacketReaders>(pPool, handlers, keyPair, settings, maxConnectionsPerIdentity);
	}
}}
