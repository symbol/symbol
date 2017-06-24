#pragma once
#include "IoTypes.h"
#include "Packet.h"
#include <stddef.h>

namespace catapult { namespace ionet {

/// Possible results from PacketExtractor::tryExtractNextPacket.
#define PACKET_EXTRACT_RESULT_LIST \
	/* A packet was succesfully extracted. */ \
	ENUM_VALUE(Success) \
	\
	/* A packet was not extracted due to insufficient data. */ \
	ENUM_VALUE(Insufficient_Data) \
	\
	/* A packet was not extracted due to a packet error. */ \
	ENUM_VALUE(Packet_Error)

#define DECLARE_ENUM PacketExtractResult
#define ENUM_LIST PACKET_EXTRACT_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DECLARE_ENUM

	/// Helper for extracting a packet from a working buffer.
	class PacketExtractor {
	public:
		/// Creates a packet extractor for extracting a packet from \a data that allows a maximum packet data
		/// size of \a maxPacketDataSize.
		PacketExtractor(ByteBuffer& data, size_t maxPacketDataSize);

	public:
		/// Tries to extract the next packet into (\a pExtractedPacket).
		PacketExtractResult tryExtractNextPacket(const Packet*& pExtractedPacket);

		/// Marks all extracted packets as consumed and deletes their backing memory.
		void consume();

	private:
		ByteBuffer& m_data;
		size_t m_maxPacketDataSize;
		size_t m_consumedBytes;
	};
}}
