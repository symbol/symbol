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

#include "SecureSignedPacketSocketDecorator.h"
#include "PacketSocketDecorator.h"
#include "SecureSignedPacketIo.h"
#include "catapult/utils/FileSize.h"

namespace catapult { namespace ionet {

	namespace {
		class SecureSignedWrapFactory {
		public:
			SecureSignedWrapFactory(const crypto::KeyPair& sourceKeyPair, const Key& remoteKey, uint32_t maxPacketDataSize)
					: m_sourceKeyPair(sourceKeyPair)
					, m_remoteKey(remoteKey)
					, m_maxPacketDataSize(maxPacketDataSize)
			{}

		public:
			auto wrapIo(const std::shared_ptr<PacketIo>& pIo) const {
				return CreateSecureSignedPacketIo(pIo, m_sourceKeyPair, m_remoteKey, m_maxPacketDataSize);
			}

			auto wrapReader(const std::shared_ptr<BatchPacketReader>& pReader) const {
				return CreateSecureSignedBatchPacketReader(pReader, m_remoteKey);
			}

		private:
			const crypto::KeyPair& m_sourceKeyPair;
			Key m_remoteKey;
			uint32_t m_maxPacketDataSize;
		};
	}

	std::shared_ptr<PacketSocket> AddSecureSigned(
			const std::shared_ptr<PacketSocket>& pSocket,
			ConnectionSecurityMode securityMode,
			const crypto::KeyPair& sourceKeyPair,
			const Key& remoteKey,
			utils::FileSize maxPacketDataSize) {
		if (!HasFlag(ConnectionSecurityMode::Signed, securityMode))
			return pSocket;

		SecureSignedWrapFactory wrapFactory(sourceKeyPair, remoteKey, maxPacketDataSize.bytes32());
		return std::make_shared<PacketSocketDecorator<SecureSignedWrapFactory>>(pSocket, wrapFactory);
	}
}}
