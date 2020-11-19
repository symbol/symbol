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
#include "catapult/ionet/IoTypes.h"
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/PacketHandlers.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include <memory>
#include <vector>

namespace catapult { namespace test {

/// Asserts that the data pointed to by \a SEND_BUFFER at offset \a SEND_BUFFER_OFFSET
/// with size \a SEND_BUFFER_SIZE is equal to \a RECEIVED_BUFFER.
#define EXPECT_EQUAL_BUFFERS(SEND_BUFFER, SEND_BUFFER_OFFSET, SEND_BUFFER_SIZE, RECEIVED_BUFFER) \
	EXPECT_EQ(SEND_BUFFER_SIZE, RECEIVED_BUFFER.size()); \
	if (SEND_BUFFER_SIZE == RECEIVED_BUFFER.size()) \
		EXPECT_EQ_MEMORY(&SEND_BUFFER[SEND_BUFFER_OFFSET], &RECEIVED_BUFFER[0], SEND_BUFFER_SIZE)

	/// Default packet type used in tests. This type is guaranteed to not conflict any with known packet types.
	constexpr ionet::PacketType Default_Packet_Type = ionet::PacketType::Undefined;

	/// Writes a packet header in \a buffer at \a offset for a packet with \a size and a default type.
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

	/// Copies \a buffer into a dynamically allocated PacketPayload.
	ionet::PacketPayload BufferToPacketPayload(const ionet::ByteBuffer& buffer);

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

	/// Generates a random push block packet.
	std::shared_ptr<ionet::Packet> GenerateRandomBlockPacket();

	/// Generates a random push transaction packet.
	std::shared_ptr<ionet::Packet> GenerateRandomTransactionPacket();

	/// Coerces a packet \a buffer to an entity of the specified type.
	template<typename TEntity>
	const TEntity& CoercePacketToEntity(const ionet::ByteBuffer& buffer) {
		return *reinterpret_cast<const TEntity*>(buffer.data() + sizeof(ionet::PacketHeader));
	}

	/// Performs a default size check of \a entity.
	template<typename TEntity>
	bool DefaultSizeCheck(const TEntity& entity) {
		auto registry = mocks::CreateDefaultTransactionRegistry();
		return IsSizeValid(entity, registry);
	}
}}
