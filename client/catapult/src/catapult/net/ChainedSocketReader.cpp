#include "ChainedSocketReader.h"
#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/ionet/SocketReader.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace net {

	namespace {
		class DefaultChainedSocketReader
				: public ChainedSocketReader
				, public std::enable_shared_from_this<DefaultChainedSocketReader> {
		public:
			DefaultChainedSocketReader(
					const std::shared_ptr<ionet::PacketSocket>& pPacketSocket,
					const ionet::ServerPacketHandlers& serverHandlers,
					const ionet::ReaderIdentity& identity,
					const ChainedSocketReader::CompletionHandler& completionHandler)
					: m_pPacketSocket(pPacketSocket)
					, m_identity(identity)
					, m_completionHandler(completionHandler)
					, m_pReader(CreateSocketReader(m_pPacketSocket, m_pPacketSocket->buffered(), serverHandlers, identity))
			{}

		public:
			void start() override {
				m_pReader->read([pThis = shared_from_this()](auto code) { pThis->read(code); });
			}

			void stop() override {
				m_pPacketSocket->close();
			}

		private:
			void read(ionet::SocketOperationCode code) {
				switch (code) {
				case ionet::SocketOperationCode::Success:
					return;

				case ionet::SocketOperationCode::Insufficient_Data:
					// Insufficient_Data signals the definitive end of a (successful) batch operation,
					// whereas Success can be returned multiple times
					return start();

				default:
					CATAPULT_LOG(warning) << m_identity << " read completed with error: " << code;
					return m_completionHandler(code);
				}
			}

		private:
			std::shared_ptr<ionet::PacketSocket> m_pPacketSocket;
			ionet::ReaderIdentity m_identity;
			ChainedSocketReader::CompletionHandler m_completionHandler;
			std::unique_ptr<ionet::SocketReader> m_pReader;
		};
	}

	std::shared_ptr<ChainedSocketReader> CreateChainedSocketReader(
			const std::shared_ptr<ionet::PacketSocket>& pPacketSocket,
			const ionet::ServerPacketHandlers& serverHandlers,
			const ionet::ReaderIdentity& identity) {
		return CreateChainedSocketReader(pPacketSocket, serverHandlers, identity, [](auto) {});
	}

	std::shared_ptr<ChainedSocketReader> CreateChainedSocketReader(
			const std::shared_ptr<ionet::PacketSocket>& pPacketSocket,
			const ionet::ServerPacketHandlers& serverHandlers,
			const ionet::ReaderIdentity& identity,
			const ChainedSocketReader::CompletionHandler& completionHandler) {
		return std::make_shared<DefaultChainedSocketReader>(pPacketSocket, serverHandlers, identity, completionHandler);
	}
}}
