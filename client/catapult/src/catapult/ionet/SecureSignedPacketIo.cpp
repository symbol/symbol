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

#include "SecureSignedPacketIo.h"
#include "BatchPacketReader.h"
#include "PacketIo.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/Signer.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace ionet {

	namespace {
		struct SecurePacketHeader : public ionet::Packet {
			static constexpr PacketType Packet_Type = PacketType::Secure_Signed;

			catapult::Signature Signature;
		};

		Hash256 CalculatePayloadHash(const PacketPayload& payload) {
			// sign full payload, including header
			crypto::Sha3_256_Builder hashBuilder;
			hashBuilder.update({ reinterpret_cast<const uint8_t*>(&payload.header()), sizeof(PacketHeader) });
			for (const auto& buffer : payload.buffers())
				hashBuilder.update(buffer);

			Hash256 payloadHash;
			hashBuilder.final(payloadHash);
			return payloadHash;
		}

		class VerifyingReadCallback {
		public:
			VerifyingReadCallback(const Key& remoteKey, PacketIo::ReadCallback callback)
					: m_remoteKey(remoteKey)
					, m_callback(callback)
			{}

		public:
			void operator()(SocketOperationCode code, const Packet* pPacket) {
				if (SocketOperationCode::Success != code)
					return m_callback(code, nullptr);

				// cannot use CoercePacket because Size is variable
				auto minPacketSize = sizeof(SecurePacketHeader) + sizeof(PacketHeader);
				if (pPacket->Type != SecurePacketHeader::Packet_Type || minPacketSize > pPacket->Size)
					return m_callback(SocketOperationCode::Malformed_Data, nullptr);

				auto& securePacketHeader = static_cast<const SecurePacketHeader&>(*pPacket);
				auto& childPacket = static_cast<const Packet&>(*(&securePacketHeader + 1));
				if (securePacketHeader.Size - sizeof(SecurePacketHeader) != childPacket.Size)
					return m_callback(SocketOperationCode::Malformed_Data, nullptr);

				Hash256 childPacketHash;
				crypto::Sha3_256({ reinterpret_cast<const uint8_t*>(&childPacket), childPacket.Size }, childPacketHash);

				if (!crypto::Verify(m_remoteKey, childPacketHash, securePacketHeader.Signature)) {
					CATAPULT_LOG(warning) << "packet from " << utils::HexFormat(m_remoteKey) << " has invalid signature";
					return m_callback(SocketOperationCode::Security_Error, nullptr);
				}

				m_callback(code, &childPacket);
			}

		private:
			const Key& m_remoteKey;
			PacketIo::ReadCallback m_callback;
		};

		class SecureSignedPacketIo
				: public PacketIo
				, public std::enable_shared_from_this<SecureSignedPacketIo> {
		public:
			SecureSignedPacketIo(
					const std::shared_ptr<PacketIo>& pIo,
					const crypto::KeyPair& sourceKeyPair,
					const Key& remoteKey,
					uint32_t maxSignedPacketDataSize)
					: m_pIo(pIo)
					, m_sourceKeyPair(sourceKeyPair)
					, m_remoteKey(remoteKey)
					, m_maxSignedPacketDataSize(maxSignedPacketDataSize)
			{}

		public:
			void write(const PacketPayload& payload, const WriteCallback& callback) override {
				if (!IsPacketDataSizeValid(payload.header(), m_maxSignedPacketDataSize)) {
					CATAPULT_LOG(warning) << "bypassing write of malformed " << payload.header();
					callback(SocketOperationCode::Malformed_Data);
					return;
				}

				auto payloadHash = CalculatePayloadHash(payload);
				auto pSecurePacketHeader = CreateSharedPacket<SecurePacketHeader>(0);
				crypto::Sign(m_sourceKeyPair, payloadHash, pSecurePacketHeader->Signature);

				m_pIo->write(PacketPayload::Merge(pSecurePacketHeader, payload), callback);
			}

			void read(const ReadCallback& callback) override {
				m_pIo->read([pThis = shared_from_this(), callback](auto code, const auto* pPacket) {
					VerifyingReadCallback(pThis->m_remoteKey, callback)(code, pPacket);
				});
			}

		private:
			std::shared_ptr<PacketIo> m_pIo;
			const crypto::KeyPair& m_sourceKeyPair;
			Key m_remoteKey;
			uint32_t m_maxSignedPacketDataSize;
		};
	}

	std::shared_ptr<PacketIo> CreateSecureSignedPacketIo(
			const std::shared_ptr<PacketIo>& pIo,
			const crypto::KeyPair& sourceKeyPair,
			const Key& remoteKey,
			uint32_t maxSignedPacketDataSize) {
		return std::make_shared<SecureSignedPacketIo>(pIo, sourceKeyPair, remoteKey, maxSignedPacketDataSize);
	}

	namespace {
		class SecureSignedBatchPacketReader
				: public BatchPacketReader
				, public std::enable_shared_from_this<SecureSignedBatchPacketReader> {
		public:
			SecureSignedBatchPacketReader(const std::shared_ptr<BatchPacketReader>& pReader, const Key& remoteKey)
					: m_pReader(pReader)
					, m_remoteKey(remoteKey)
			{}

		public:
			void readMultiple(const PacketIo::ReadCallback& callback) override {
				m_pReader->readMultiple([pThis = shared_from_this(), callback](auto code, const auto* pPacket) {
					VerifyingReadCallback(pThis->m_remoteKey, callback)(code, pPacket);
				});
			}

		private:
			std::shared_ptr<BatchPacketReader> m_pReader;
			Key m_remoteKey;
		};
	}

	std::shared_ptr<BatchPacketReader> CreateSecureSignedBatchPacketReader(
			const std::shared_ptr<BatchPacketReader>& pReader,
			const Key& remoteKey) {
		return std::make_shared<SecureSignedBatchPacketReader>(pReader, remoteKey);
	}
}}
