#pragma once
#include "catapult/ionet/Packet.h"
#include "catapult/types.h"

namespace catapult { namespace api {

#pragma pack(push, 1)

	/// A chain info response.
	struct ChainInfoResponse : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = ionet::PacketType::Chain_Info;

		/// The chain height.
		catapult::Height Height;

		/// The high part of the score.
		uint64_t ScoreHigh;

		/// The low part of the score.
		uint64_t ScoreLow;
	};

	/// A packet containing header information and a height.
	template<ionet::PacketType PacketType>
	struct HeightPacket : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = PacketType;

		/// The requested block height.
		catapult::Height Height;
	};

	/// A pull block request.
	using PullBlockRequest = HeightPacket<ionet::PacketType::Pull_Block>;

	/// A block hashes request.
	using BlockHashesRequest = HeightPacket<ionet::PacketType::Block_Hashes>;

	/// A pull blocks request.
	struct PullBlocksRequest : public HeightPacket<ionet::PacketType::Pull_Blocks> {
		/// The requested number of blocks.
		uint32_t NumBlocks;

		/// The requested response size (in bytes).
		uint32_t NumResponseBytes;
	};

#pragma pack(pop)
}}
