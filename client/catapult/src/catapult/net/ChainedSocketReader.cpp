#include "ChainedSocketReader.h"
#include "AsyncTcpServer.h"
#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/ionet/SocketReader.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace net {

	namespace {
		std::unique_ptr<ionet::SocketReader> CreateReader(
				const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext,
				const ionet::ServerPacketHandlers& serverHandlers) {
			return CreateSocketReader(
					pAcceptContext->socket(),
					CreateBufferedPacketIo(pAcceptContext->socket(), boost::asio::strand(pAcceptContext->service())),
					serverHandlers);
		}

		class DefaultChainedSocketReader
				: public ChainedSocketReader
				, public std::enable_shared_from_this<DefaultChainedSocketReader> {
		public:
			DefaultChainedSocketReader(
					const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext,
					const ionet::ServerPacketHandlers& serverHandlers,
					const ChainedSocketReader::CompletionHandler& completionHandler)
					: m_pAcceptContext(pAcceptContext)
					, m_serverHandlers(serverHandlers)
					, m_completionHandler(completionHandler)
					, m_pReader(CreateReader(pAcceptContext, serverHandlers))
			{}

		public:
			void start() override {
				m_pReader->read([pThis = shared_from_this()](auto code) { pThis->read(code); });
			}

			void stop() override {
				m_pAcceptContext->socket()->close();
			}

		private:
			void read(const ionet::SocketOperationCode& code) {
				switch (code) {
				case ionet::SocketOperationCode::Success:
					return;

				case ionet::SocketOperationCode::Insufficient_Data:
					// Insufficient_Data signals the definitive end of a (successful) batch operation,
					// whereas Success can be returned multiple times
					return start();

				default:
					CATAPULT_LOG(warning) << "read completed with error: " << code;
					return m_completionHandler(code);
				}
			}

		private:
			std::shared_ptr<AsyncTcpServerAcceptContext> m_pAcceptContext;
			ionet::ServerPacketHandlers m_serverHandlers;
			ChainedSocketReader::CompletionHandler m_completionHandler;
			std::unique_ptr<ionet::SocketReader> m_pReader;
		};
	}

	std::shared_ptr<ChainedSocketReader> CreateChainedSocketReader(
			const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext,
			const ionet::ServerPacketHandlers& serverHandlers) {
		return CreateChainedSocketReader(pAcceptContext, serverHandlers, [](auto) {});
	}

	std::shared_ptr<ChainedSocketReader> CreateChainedSocketReader(
			const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext,
			const ionet::ServerPacketHandlers& serverHandlers,
			const ChainedSocketReader::CompletionHandler& completionHandler) {
		return std::make_shared<DefaultChainedSocketReader>(pAcceptContext, serverHandlers, completionHandler);
	}
}}
