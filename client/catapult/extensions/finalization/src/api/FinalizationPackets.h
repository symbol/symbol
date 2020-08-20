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

namespace catapult { namespace api {

#pragma pack(push, 1)

	/// Finalization statistics response.
	struct FinalizationStatisticsResponse : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = ionet::PacketType::Finalization_Statistics;

		/// Finalization point.
		FinalizationPoint Point;

		/// Finalization height.
		catapult::Height Height;

		/// Finalization hash.
		Hash256 Hash;
	};

	/// Request packet for a proof at a finalization point.
	struct ProofAtPointRequest : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = ionet::PacketType::Finalization_Proof_At_Point;

		/// Requested finalization proof point.
		FinalizationPoint Point;
	};

	/// Request packet for a proof at a finalization height.
	struct ProofAtHeightRequest : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = ionet::PacketType::Finalization_Proof_At_Height;

		/// Requested finalization proof height.
		catapult::Height Height;
	};

#pragma pack(pop)
}}
