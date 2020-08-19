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

#pragma once
#include "catapult/ionet/Packet.h"
#include "catapult/types.h"

namespace catapult { namespace api {

#pragma pack(push, 1)

	/// Chain statistics response.
	struct ChainStatisticsResponse : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = ionet::PacketType::Chain_Statistics;

		/// Chain height.
		catapult::Height Height;

		/// Finalized chain height.
		catapult::Height FinalizedHeight;

		/// High part of the score.
		uint64_t ScoreHigh;

		/// Low part of the score.
		uint64_t ScoreLow;
	};

	/// Packet containing header information and a height.
	template<ionet::PacketType PacketType>
	struct HeightPacket : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = PacketType;

		/// Requested block height.
		catapult::Height Height;
	};

	/// Pull block request.
	using PullBlockRequest = HeightPacket<ionet::PacketType::Pull_Block>;

	/// Block hashes request.
	struct BlockHashesRequest : public HeightPacket<ionet::PacketType::Block_Hashes> {
		/// Requested number of hashes.
		uint32_t NumHashes;
	};

	/// Pull blocks request.
	struct PullBlocksRequest : public HeightPacket<ionet::PacketType::Pull_Blocks> {
		/// Requested number of blocks.
		uint32_t NumBlocks;

		/// Requested response size (in bytes).
		uint32_t NumResponseBytes;
	};

#pragma pack(pop)
}}
