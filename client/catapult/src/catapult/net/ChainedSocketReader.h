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

#pragma once
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/ionet/SocketOperationCode.h"
#include <functional>
#include <memory>

namespace catapult {
	namespace ionet {
		class PacketSocket;
		struct ReaderIdentity;
	}
}

namespace catapult { namespace net {

	/// A reader that chains reads from a socket (it initiates the next read upon the successful completion
	/// of the current read).
	class ChainedSocketReader {
	public:
		virtual ~ChainedSocketReader() = default;

	public:
		/// Callback that is called when the read chain is broken.
		using CompletionHandler = consumer<ionet::SocketOperationCode>;

	public:
		/// Starts reading.
		virtual void start() = 0;

		/// Stops reading.
		virtual void stop() = 0;
	};

	/// Creates a chained socket reader around \a pPacketSocket and \a serverHandlers with a default completion
	/// handler given an \a identity.
	std::shared_ptr<ChainedSocketReader> CreateChainedSocketReader(
			const std::shared_ptr<ionet::PacketSocket>& pPacketSocket,
			const ionet::ServerPacketHandlers& serverHandlers,
			const ionet::ReaderIdentity& identity);

	/// Creates a chained socket reader around \a pPacketSocket and \a serverHandlers with a custom completion
	/// handler (\a completionHandler) given an \a identity.
	std::shared_ptr<ChainedSocketReader> CreateChainedSocketReader(
			const std::shared_ptr<ionet::PacketSocket>& pPacketSocket,
			const ionet::ServerPacketHandlers& serverHandlers,
			const ionet::ReaderIdentity& identity,
			const ChainedSocketReader::CompletionHandler& completionHandler);
}}
