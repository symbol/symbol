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
#include "IoTypes.h"
#include "catapult/types.h"

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace ionet {
		class BatchPacketReader;
		class PacketIo;
	}
}

namespace catapult { namespace ionet {

	/// Adds secure signing to all packets read from and written to \a pIo.
	/// - All written packets are wrapped in a signature packet, signed by \a sourceKeyPair and must have
	///   a max packet data size of \a maxSignedPacketDataSize.
	/// - All read packets are validated to be signed by \a remoteKey.
	std::shared_ptr<PacketIo> CreateSecureSignedPacketIo(
			const std::shared_ptr<PacketIo>& pIo,
			const crypto::KeyPair& sourceKeyPair,
			const Key& remoteKey,
			uint32_t maxSignedPacketDataSize);

	/// Adds secure signing to all packets read from \a pReader.
	/// - All read packets are validated to be signed by \a remoteKey.
	std::shared_ptr<BatchPacketReader> CreateSecureSignedBatchPacketReader(
			const std::shared_ptr<BatchPacketReader>& pReader,
			const Key& remoteKey);
}}
