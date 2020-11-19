/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "PacketHandlers.h"
#include "SocketOperationCode.h"
#include "catapult/model/NodeIdentity.h"
#include <memory>

namespace catapult {
	namespace ionet {
		class BatchPacketReader;
		class PacketIo;
	}
}

namespace catapult { namespace ionet {

	/// Reads and consumes packets from a socket.
	class SocketReader {
	public:
		using ReadCallback = consumer<SocketOperationCode>;

	public:
		virtual ~SocketReader() = default;

	public:
		/// Reads and consumes one or more packets and calls \a callback on completion.
		virtual void read(const ReadCallback& callback) = 0;
	};

	/// Creates a socket packet reader around \a pReader, \a pWriter and \a handlers given a reader \a identity.
	std::unique_ptr<SocketReader> CreateSocketReader(
			const std::shared_ptr<BatchPacketReader>& pReader,
			const std::shared_ptr<PacketIo>& pWriter,
			const ServerPacketHandlers& handlers,
			const model::NodeIdentity& identity);
}}
