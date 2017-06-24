#pragma once
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/ionet/SocketOperationCode.h"
#include <functional>
#include <memory>

namespace catapult { namespace net { class AsyncTcpServerAcceptContext; } }

namespace catapult { namespace net {

	/// A reader that chains reads from a socket (it initiates the next read upon the successful completion
	/// of the current read).
	class ChainedSocketReader {
	public:
		virtual ~ChainedSocketReader() {}

	public:
		/// Callback that is called when the read chain is broken.
		using CompletionHandler = std::function<void (ionet::SocketOperationCode)>;

	public:
		/// Starts reading.
		virtual void start() = 0;

		/// Stops reading.
		virtual void stop() = 0;
	};

	/// Creates a chained socket reader around \a pAcceptContext and \a serverHandlers with a default completion
	/// handler.
	std::shared_ptr<ChainedSocketReader> CreateChainedSocketReader(
			const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext,
			const ionet::ServerPacketHandlers& serverHandlers);

	/// Creates a chained socket reader around \a pAcceptContext and \a serverHandlers with a custom completion
	/// handler (\a completionHandler).
	std::shared_ptr<ChainedSocketReader> CreateChainedSocketReader(
			const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext,
			const ionet::ServerPacketHandlers& serverHandlers,
			const ChainedSocketReader::CompletionHandler& completionHandler);
}}
