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
			using ChainedSocketReaderFactory = std::function<std::shared_ptr<ChainedSocketReader> (const PacketSocketPointer&)>;

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
				auto pReader = readerFactory(pSocket);
				pReader->start();
				insertedReaderIter->second.pReader = pReader;
				return true;
			}

			bool close(const Key& identityKey) {
				utils::SpinLockGuard guard(m_lock);

				bool anyClosed = false;
				for (auto i = 0u; i < m_maxConnectionsPerIdentity; ++i)
					anyClosed = close(identityKey, i) || anyClosed;

				return anyClosed;
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

			bool close(const Key& identityKey, uint32_t id) {
				auto iter = m_readers.find(std::make_pair(identityKey, id));
				if (m_readers.end() == iter)
					return false;

				CATAPULT_LOG(debug) << "closing connection to " << utils::HexFormat(identityKey) << " - " << id;
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
					const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
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
				auto acceptCallback = [pThis = shared_from_this(), socketInfo, callback](auto result, const auto& identityKey) {
					if (PeerConnectResult::Accepted == result) {
						if (!pThis->addReader(identityKey, socketInfo)) {
							result = PeerConnectResult::Already_Connected;
						} else {
							CATAPULT_LOG(debug)
									<< "accepted connection from '" << socketInfo.host()
									<< "' as " << utils::HexFormat(identityKey);
						}
					}

					return callback(result);
				};
				m_pClientConnector->accept(socketInfo.socket(), acceptCallback);
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
				return m_readers.insert(identityKey, socketInfo.socket(), [this, &identity](const auto& pSocket) {
					return this->createReader(pSocket, identity);
				});
			}

			std::shared_ptr<ChainedSocketReader> createReader(const PacketSocketPointer& pSocket, const ionet::ReaderIdentity& identity) {
				const auto& identityKey = identity.PublicKey;
				return CreateChainedSocketReader(pSocket, m_handlers, identity, [pThis = shared_from_this(), identityKey](auto) {
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
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const ionet::ServerPacketHandlers& handlers,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings,
			uint32_t maxConnectionsPerIdentity) {
		return std::make_shared<DefaultPacketReaders>(pPool, handlers, keyPair, settings, maxConnectionsPerIdentity);
	}
}}
