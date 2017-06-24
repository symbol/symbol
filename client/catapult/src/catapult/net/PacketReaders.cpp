#include "PacketReaders.h"
#include "ChainedSocketReader.h"
#include "ClientConnector.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/WeakContainer.h"

namespace catapult { namespace net {

	namespace {
		class DefaultPacketReaders
				: public PacketReaders
				, public std::enable_shared_from_this<DefaultPacketReaders> {
		public:
			explicit DefaultPacketReaders(
					const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
					const ionet::ServerPacketHandlers& handlers,
					const crypto::KeyPair& keyPair,
					const ConnectionSettings& settings)
					: m_handlers(handlers)
					, m_pClientConnector(CreateClientConnector(pPool, keyPair, settings))
					, m_readers([](auto& reader) { reader.stop(); })
			{}

		public:
			size_t numActiveConnections() const override {
				return m_pClientConnector->numActiveConnections();
			}

			size_t numActiveReaders() const override {
				return m_readers.size();
			}

		public:
			void accept(
					const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext,
					const AcceptCallback& callback) override {
				auto acceptCallback = [pThis = shared_from_this(), pAcceptContext, callback](auto result, const auto&) {
					if (PeerConnectResult::Accepted == result)
						pThis->addReader(pAcceptContext);

					return callback(result);
				};
				m_pClientConnector->accept(pAcceptContext, acceptCallback);
			}

			void shutdown() override {
				CATAPULT_LOG(info) << "closing all connections in PacketReaders";
				m_pClientConnector->shutdown();
				m_readers.clear();
			}

		private:
			void addReader(const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext) {
				auto pReader = CreateChainedSocketReader(pAcceptContext, m_handlers);
				pReader->start();
				m_readers.insert(pReader);
			}

		private:
			ionet::ServerPacketHandlers m_handlers;
			std::shared_ptr<ClientConnector> m_pClientConnector;
			utils::WeakContainer<ChainedSocketReader> m_readers;
		};
	}

	std::shared_ptr<PacketReaders> CreatePacketReaders(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const ionet::ServerPacketHandlers& handlers,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings) {
		return std::make_shared<DefaultPacketReaders>(pPool, handlers, keyPair, settings);
	}
}}
