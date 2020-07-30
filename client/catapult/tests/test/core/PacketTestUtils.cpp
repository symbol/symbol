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

#include "PacketTestUtils.h"
#include "BlockTestUtils.h"
#include "mocks/MockTransaction.h"
#include "catapult/model/Block.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	void SetPacketAt(ionet::ByteBuffer& buffer, size_t offset, uint32_t size) {
		auto pPacket = reinterpret_cast<ionet::Packet*>(&buffer[offset]);
		pPacket->Size = size;
		pPacket->Type = Default_Packet_Type;
	}

	ionet::ByteBuffer GenerateRandomPacketBuffer(uint32_t packetSize) {
		return GenerateRandomPacketBuffer(packetSize, { packetSize });
	}

	ionet::ByteBuffer GenerateRandomPacketBuffer(size_t bufferSize, const std::vector<uint32_t>& packetSizes) {
		auto buffer = GenerateRandomVector(bufferSize);

		size_t offset = 0;
		for (auto packetSize : packetSizes) {
			SetPacketAt(buffer, offset, packetSize);
			offset += packetSize;
		}

		return buffer;
	}

	std::vector<ionet::ByteBuffer> GenerateRandomPacketBuffers(const std::vector<uint32_t>& packetSizes) {
		std::vector<ionet::ByteBuffer> buffers;
		for (auto packetSize : packetSizes) {
			auto buffer = GenerateRandomPacketBuffer(packetSize);
			buffers.push_back(buffer);
		}

		return buffers;
	}

	ionet::ByteBuffer CopyPacketToBuffer(const ionet::Packet& packet) {
		ionet::ByteBuffer packetBytes(packet.Size);
		auto pPacketBuffer = reinterpret_cast<const uint8_t*>(&packet);
		std::copy(pPacketBuffer, pPacketBuffer + packet.Size, packetBytes.begin());
		return packetBytes;
	}

	void AddCopyBuffersHandler(ionet::ServerPacketHandlers& serverHandlers, std::vector<ionet::ByteBuffer>& receivedBuffers) {
		serverHandlers.registerHandler(Default_Packet_Type, [&receivedBuffers](const auto& packet, const auto&) {
			receivedBuffers.push_back(CopyPacketToBuffer(packet));
		});
	}

	std::unique_ptr<ionet::Packet> BufferToPacket(const ionet::ByteBuffer& buffer) {
		auto pPacket = utils::MakeUniqueWithSize<ionet::Packet>(buffer.size());
		std::memcpy(static_cast<void*>(pPacket.get()), buffer.data(), buffer.size());
		return pPacket;
	}

	ionet::PacketPayload BufferToPacketPayload(const ionet::ByteBuffer& buffer) {
		return ionet::PacketPayload(utils::UniqueToShared(BufferToPacket(buffer)));
	}

	std::shared_ptr<ionet::Packet> CreateRandomPacket(uint32_t payloadSize, ionet::PacketType packetType) {
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
		pPacket->Type = packetType;

		// fill payload with random data
		if (0 < payloadSize)
			test::FillWithRandomData({ pPacket->Data(), payloadSize });

		return pPacket;
	}

	namespace {
		template<typename TEntity>
		void SetVerifiableEntityAt(ionet::ByteBuffer& buffer, size_t offset, size_t size, model::EntityType entityType) {
			auto entitySize = static_cast<uint32_t>(size);
			auto remainingBufferSize = buffer.size() - offset;

			if (sizeof(TEntity) > remainingBufferSize)
				throw std::runtime_error("cannot fit the data in provided buffer");

			auto& entity = reinterpret_cast<model::VerifiableEntity&>(buffer[offset]);
			entity.Size = entitySize;
			entity.Type = entityType;
		}
	}

	void SetTransactionAt(ionet::ByteBuffer& buffer, size_t offset, size_t size) {
		SetVerifiableEntityAt<mocks::MockTransaction>(buffer, offset, size, mocks::MockTransaction::Entity_Type);

		auto& transaction = reinterpret_cast<mocks::MockTransaction&>(buffer[offset]);

		// assume extra size is part of data
		transaction.Data.Size = size >= sizeof(mocks::MockTransaction)
				? static_cast<uint16_t>(size - sizeof(mocks::MockTransaction))
				: 0;
	}

	void SetBlockAt(ionet::ByteBuffer& buffer, size_t offset) {
		SetBlockAt(buffer, offset, sizeof(model::BlockHeader));
	}

	void SetBlockAt(ionet::ByteBuffer& buffer, size_t offset, size_t size) {
		SetVerifiableEntityAt<model::Block>(buffer, offset, size, model::Entity_Type_Block);
	}

	ionet::Packet& SetPushBlockPacketInBuffer(ionet::ByteBuffer& buffer) {
		// fill the buffer with random data
		FillWithRandomData(buffer);

		// set the packet at the start of the buffer
		auto& packet = reinterpret_cast<ionet::Packet&>(buffer[0]);
		packet.Size = static_cast<uint32_t>(buffer.size());
		packet.Type = ionet::PacketType::Push_Block;

		// set the block after the packet if Size and Type fit in the buffer
		uint32_t entitySize = packet.Size - SizeOf32<ionet::Packet>();
		SetBlockAt(buffer, sizeof(ionet::Packet), entitySize);
		return packet;
	}

	namespace {
		template<typename TEntity>
		std::shared_ptr<ionet::Packet> GeneratePushEntityPacket(ionet::PacketType type, const TEntity& entity) {
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(entity.Size);
			pPacket->Type = type;

			std::memcpy(static_cast<void*>(pPacket.get() + 1), &entity, entity.Size);
			return pPacket;
		}
	}

	std::shared_ptr<ionet::Packet> GenerateRandomBlockPacket() {
		return GeneratePushEntityPacket(ionet::PacketType::Push_Block, *GenerateEmptyRandomBlock());
	}

	std::shared_ptr<ionet::Packet> GenerateRandomTransactionPacket() {
		return GeneratePushEntityPacket(ionet::PacketType::Push_Transactions, *GenerateRandomTransaction());
	}
}}
