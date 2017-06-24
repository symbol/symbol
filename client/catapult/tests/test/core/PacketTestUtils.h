#pragma once
#include "catapult/ionet/IoTypes.h"
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/PacketHandlers.h"
#include <memory>
#include <vector>

namespace catapult { namespace test {

/// Asserts that the data pointed to by \a SEND_BUFFER at offset \a SEND_BUFFER_OFFSET
/// with size \a SEND_BUFFER_SIZE is equal to \a RECEIVED_BUFFER.
#define EXPECT_EQUAL_BUFFERS(SEND_BUFFER, SEND_BUFFER_OFFSET, SEND_BUFFER_SIZE, RECEIVED_BUFFER) \
	EXPECT_EQ(SEND_BUFFER_SIZE, RECEIVED_BUFFER.size()); \
	EXPECT_EQ( \
			test::ToHexString(&SEND_BUFFER[SEND_BUFFER_OFFSET], SEND_BUFFER_SIZE), \
			test::ToHexString(RECEIVED_BUFFER));

	/// The default packet type used in tests. This type is guaranteed to not conflict any with known packet types.
	constexpr ionet::PacketType Default_Packet_Type = ionet::PacketType::Undefined;

	/// Writes a packet header in \a buffer at \a offset for a packet with size \a size and a default type.
	void SetPacketAt(ionet::ByteBuffer& buffer, size_t offset, uint32_t size);

	/// Generates a random packet buffer of size \a packetSize containing a default packet of size
	/// \a packetSize at offset \c 0.
	ionet::ByteBuffer GenerateRandomPacketBuffer(uint32_t packetSize);

	/// Generates a random packet buffer of size \a bufferSize containing packets of sizes \a packetSizes.
	ionet::ByteBuffer GenerateRandomPacketBuffer(size_t bufferSize, const std::vector<uint32_t>& packetSizes);

	/// Generates random packet buffers for packet (and buffer) sizes in \a packetSizes.
	std::vector<ionet::ByteBuffer> GenerateRandomPacketBuffers(const std::vector<uint32_t>& packetSizes);

	/// Copies the full contents of \a packet to a buffer.
	ionet::ByteBuffer CopyPacketToBuffer(const ionet::Packet& packet);

	/// Adds a handler for the default packet type to \a serverHandlers that copies all handled packets to
	/// \a receivedBuffers.
	void AddCopyBuffersHandler(ionet::ServerPacketHandlers& serverHandlers, std::vector<ionet::ByteBuffer>& receivedBuffers);

	/// Copies \a buffer into a dynamically allocated Packet.
	std::unique_ptr<ionet::Packet> BufferToPacket(const ionet::ByteBuffer& buffer);

	/// Generates a packet with a random payload of \a payloadSize bytes and type \a packetType.
	std::shared_ptr<ionet::Packet> CreateRandomPacket(uint32_t payloadSize, ionet::PacketType packetType);

	/// Registers \a handler with \a handlers for the default packet type.
	template<typename TPacketHandlers>
	void RegisterDefaultHandler(TPacketHandlers& handlers, const typename TPacketHandlers::PacketHandler& handler) {
		handlers.registerHandler(Default_Packet_Type, handler);
	}

	/// Sets a transaction in \a buffer at \a offset with a custom \a size.
	void SetTransactionAt(ionet::ByteBuffer& buffer, size_t offset, size_t size);

	/// Sets a block in \a buffer at \a offset with the default block size.
	void SetBlockAt(ionet::ByteBuffer& buffer, size_t offset);

	/// Sets a block in \a buffer at \a offset with a custom \a size.
	void SetBlockAt(ionet::ByteBuffer& buffer, size_t offset, size_t size);

	/// Sets a push block packet in \a buffer.
	ionet::Packet& SetPushBlockPacketInBuffer(ionet::ByteBuffer& buffer);
}}
