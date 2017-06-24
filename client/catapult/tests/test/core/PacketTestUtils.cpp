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
		std::unique_ptr<ionet::Packet> pPacket(reinterpret_cast<ionet::Packet*>(::operator new (buffer.size())));
		std::memcpy(pPacket.get(), buffer.data(), buffer.size());
		return pPacket;
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
		SetBlockAt(buffer, offset, sizeof(model::Block));
	}

	void SetBlockAt(ionet::ByteBuffer& buffer, size_t offset, size_t size) {
		SetVerifiableEntityAt<model::Block>(buffer, offset, size, model::EntityType::Block);
	}

	ionet::Packet& SetPushBlockPacketInBuffer(ionet::ByteBuffer& buffer) {
		// fill the buffer with random data
		FillWithRandomData(buffer);

		// set the packet at the start of the buffer
		auto& packet = reinterpret_cast<ionet::Packet&>(*buffer.data());
		packet.Size = static_cast<uint32_t>(buffer.size());
		packet.Type = ionet::PacketType::Push_Block;

		// set the block after the packet if Size and Type fit in the buffer
		uint32_t entitySize = packet.Size - sizeof(ionet::Packet);
		SetBlockAt(buffer, sizeof(ionet::Packet), entitySize);
		return packet;
	}
}}
