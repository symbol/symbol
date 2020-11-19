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

#include "PacketIoTestUtils.h"
#include "EntityTestUtils.h"
#include "PacketTestUtils.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	ionet::PacketIo::ReadCallback CreateReadCaptureCallback(PacketIoReadCallbackParams& capture) {
		return [&capture](auto code, const auto* pReadPacket) {
			capture.ReadCode = code;
			capture.IsPacketValid = !!pReadPacket;
			if (capture.IsPacketValid)
				capture.ReadPacketBytes = CopyPacketToBuffer(*pReadPacket);
		};
	}

	ionet::PacketIo::ReadCallback CreateReadCaptureCallback(std::vector<PacketIoReadCallbackParams>& captures) {
		return [&captures](auto code, const auto* pReadPacket) {
			PacketIoReadCallbackParams capture;
			CreateReadCaptureCallback(capture)(code, pReadPacket);
			captures.push_back(capture);
		};
	}

	namespace {
		template<typename TTraits>
		void AssertCanRoundtripPackets(mocks::MockPacketIo& mockIo, TTraits ioTraits) {
			// Arrange:
			mockIo.queueWrite(ionet::SocketOperationCode::Success);

			// Act: write a payload
			auto entities = std::vector<std::shared_ptr<model::VerifiableEntity>>{
				CreateRandomEntityWithSize<>(126),
				CreateRandomEntityWithSize<>(212)
			};
			auto payload = ionet::PacketPayloadFactory::FromEntities(ionet::PacketType::Push_Transactions, entities);

			ionet::SocketOperationCode writeCode;
			ioTraits.write(payload, [&writeCode](auto code) {
				writeCode = code;
			});

			// Assert: write succeeded
			ASSERT_EQ(ionet::SocketOperationCode::Success, writeCode);

			// Arrange: prepare a read of the written data
			mockIo.queueRead(ionet::SocketOperationCode::Success, [](const auto* pWrittenPacket) {
				auto pWrittenPacketCopy = utils::MakeSharedWithSize<ionet::Packet>(pWrittenPacket->Size);
				std::memcpy(static_cast<void*>(pWrittenPacketCopy.get()), pWrittenPacket, pWrittenPacket->Size);
				return pWrittenPacketCopy;
			});

			// Act: read it back
			bool isPacketValid;
			ionet::SocketOperationCode readCode;
			std::vector<uint8_t> readPacketBytes;
			ioTraits.read([&isPacketValid, &readCode, &readPacketBytes](auto code, const auto* pReadPacket) {
				readCode = code;
				isPacketValid = !!pReadPacket;
				if (isPacketValid)
					readPacketBytes = CopyPacketToBuffer(*pReadPacket);
			});

			// Assert:
			ASSERT_EQ(ionet::SocketOperationCode::Success, readCode);

			const auto& readPacket = reinterpret_cast<const ionet::Packet&>(readPacketBytes[0]);
			ASSERT_EQ(sizeof(ionet::PacketHeader) + 126 + 212, readPacket.Size);
			EXPECT_EQ(ionet::PacketType::Push_Transactions, readPacket.Type);

			EXPECT_EQ_MEMORY(payload.buffers()[0].pData, readPacket.Data(), payload.buffers()[0].Size);
			EXPECT_EQ_MEMORY(payload.buffers()[1].pData, readPacket.Data() + 126, payload.buffers()[1].Size);
		}

		struct BasicIoTraits {
		public:
			explicit BasicIoTraits(ionet::PacketIo& io) : m_io(io)
			{}

		public:
			void write(const ionet::PacketPayload& payload, const ionet::PacketIo::WriteCallback& callback) {
				m_io.write(payload, callback);
			}

			void read(const ionet::PacketIo::ReadCallback& callback) {
				m_io.read(callback);
			}

		private:
			ionet::PacketIo& m_io;
		};

		struct ReaderIoTraits : public BasicIoTraits {
		public:
			ReaderIoTraits(ionet::PacketIo& io, ionet::BatchPacketReader& reader)
					: BasicIoTraits(io)
					, m_reader(reader)
			{}

		public:
			void read(const ionet::PacketIo::ReadCallback& callback) {
				m_reader.readMultiple(callback);
			}

		private:
			ionet::BatchPacketReader& m_reader;
		};
	}

	void AssertCanRoundtripPackets(mocks::MockPacketIo& mockIo, ionet::PacketIo& io) {
		BasicIoTraits ioTraits(io);
		AssertCanRoundtripPackets(mockIo, ioTraits);
	}

	void AssertCanRoundtripPackets(mocks::MockPacketIo& mockIo, ionet::PacketIo& io, ionet::BatchPacketReader& reader) {
		ReaderIoTraits ioTraits(io, reader);
		AssertCanRoundtripPackets(mockIo, ioTraits);
	}
}}
