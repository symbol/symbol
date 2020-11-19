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
#include "IoTypes.h"
#include "Packet.h"
#include <stddef.h>

namespace catapult { namespace ionet {

#define PACKET_EXTRACT_RESULT_LIST \
	/* Packet was succesfully extracted. */ \
	ENUM_VALUE(Success) \
	\
	/* Packet was not extracted due to insufficient data. */ \
	ENUM_VALUE(Insufficient_Data) \
	\
	/* Packet was not extracted due to a packet error. */ \
	ENUM_VALUE(Packet_Error)

#define ENUM_VALUE(LABEL) LABEL,
	/// Possible results from PacketExtractor::tryExtractNextPacket.
	enum class PacketExtractResult {
		PACKET_EXTRACT_RESULT_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, PacketExtractResult value);

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
