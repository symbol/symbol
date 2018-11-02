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

#include "SecurePacketSocketDecorator.h"
#include "PacketSocket.h"
#include "SecureSignedPacketIo.h"
#include "catapult/utils/FileSize.h"

namespace catapult { namespace ionet {

	namespace {
		class SecureSignedPacketSocket : public PacketSocket {
		public:
			SecureSignedPacketSocket(
					const std::shared_ptr<PacketSocket>& pSocket,
					const crypto::KeyPair& sourceKeyPair,
					const Key& remoteKey,
					uint32_t maxPacketDataSize)
					: m_pSocket(pSocket)
					, m_sourceKeyPair(sourceKeyPair)
					, m_remoteKey(remoteKey)
					, m_maxPacketDataSize(maxPacketDataSize)
					, m_pIo(createSecureSignedPacketIo(m_pSocket))
					, m_pReader(CreateSecureSignedBatchPacketReader(m_pSocket, m_remoteKey))
			{}

		public:
			void read(const ReadCallback& callback) override {
				m_pIo->read(callback);
			}

			void write(const PacketPayload& payload, const WriteCallback& callback) override {
				m_pIo->write(payload, callback);
			}

			void readMultiple(const ReadCallback& callback) override {
				m_pReader->readMultiple(callback);
			}

		public:
			void stats(const StatsCallback& callback) override {
				m_pSocket->stats(callback);
			}

			void close() override{
				m_pSocket->close();
			}

			std::shared_ptr<PacketIo> buffered() override {
				return createSecureSignedPacketIo(m_pSocket->buffered());
			}

		private:
			std::shared_ptr<PacketIo> createSecureSignedPacketIo(const std::shared_ptr<PacketIo>& pSocket) {
				return CreateSecureSignedPacketIo(pSocket, m_sourceKeyPair, m_remoteKey, m_maxPacketDataSize);
			}

		private:
			std::shared_ptr<PacketSocket> m_pSocket;
			const crypto::KeyPair& m_sourceKeyPair;
			Key m_remoteKey;
			uint32_t m_maxPacketDataSize;
			std::shared_ptr<PacketIo> m_pIo;
			std::shared_ptr<BatchPacketReader> m_pReader;
		};
	}

	std::shared_ptr<PacketSocket> Secure(
			const std::shared_ptr<PacketSocket>& pSocket,
			ConnectionSecurityMode securityMode,
			const crypto::KeyPair& sourceKeyPair,
			const Key& remoteKey,
			utils::FileSize maxPacketDataSize) {
		return HasFlag(ConnectionSecurityMode::Signed, securityMode)
				? std::make_shared<SecureSignedPacketSocket>(pSocket, sourceKeyPair, remoteKey, maxPacketDataSize.bytes32())
				: pSocket;
	}
}}
