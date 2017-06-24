#pragma once
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/PacketHandlers.h"

namespace catapult { namespace test {

	/// Asserts that the specified \a payload is a thin wrapper around the specified \a packet.
	void AssertPacketPayload(const ionet::Packet& packet, const ionet::PacketPayload& payload);

	/// Asserts that \a payload has a header with size (\a expectedSize) and type (\a expectedType).
	void AssertPacketHeader(const ionet::PacketPayload& payload, size_t expectedSize, ionet::PacketType expectedType);

	/// Asserts that \a context does not have a response.
	void AssertNoResponse(const ionet::ServerPacketHandlerContext& context);

	/// Asserts that \a context has a response payload header with size (\a expectedSize) and type (\a expectedType).
	void AssertPacketHeader(
			const ionet::ServerPacketHandlerContext& context,
			size_t expectedSize,
			ionet::PacketType expectedType);

	/// Asserts that \a context has a response with a single buffer and returns the buffer.
	const uint8_t* GetSingleBufferData(const ionet::ServerPacketHandlerContext& context);
}}
