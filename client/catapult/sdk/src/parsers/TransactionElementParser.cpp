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

#include "TransactionElementParser.h"
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/PacketPayloadParser.h"

namespace catapult { namespace parsers {

	bool TryParseTransactionElements(
			const ionet::Packet& packet,
			const predicate<const model::Transaction&>& isValid,
			std::vector<model::TransactionElement>& elements) {
		constexpr auto Header_Size = sizeof(ionet::PacketHeader) + sizeof(uint32_t);
		if (packet.Size < Header_Size) {
			CATAPULT_LOG(debug) << "packet does not contain full header (" << packet.Size << ", header size " << Header_Size << ")";
			return false;
		}

		auto hashesSize = reinterpret_cast<const uint32_t&>(*packet.Data());
		if (hashesSize > packet.Size - Header_Size) {
			CATAPULT_LOG(debug) << "packet does not contain all reported hashes (" << packet.Size << ", hashes size " << hashesSize << ")";
			return false;
		}

		const auto* pHashesStart = packet.Data() + sizeof(uint32_t);
		auto numHashes = ionet::CountFixedSizeStructures<Hash256>({ pHashesStart, hashesSize });
		if (0 == numHashes) {
			CATAPULT_LOG(debug) << "packet data is invalid because no hashes were extracted (" << packet.Size << ")";
			return false;
		}

		const auto* pEntitiesStart = pHashesStart + hashesSize;
		auto entitiesSize = packet.Size - Header_Size - hashesSize;
		auto transactionOffsets = ionet::ExtractEntityOffsets<model::Transaction>({ pEntitiesStart, entitiesSize }, isValid);
		if (2 * transactionOffsets.size() != numHashes) {
			CATAPULT_LOG(debug)
					<< "packet has an unexpected number of entities (" << transactionOffsets.size()
					<< ") and hashes (" << numHashes << ")";
			return false;
		}

		const auto* pHash = reinterpret_cast<const Hash256*>(packet.Data() + sizeof(uint32_t));
		for (auto offset : transactionOffsets) {
			elements.emplace_back(reinterpret_cast<const model::Transaction&>(pEntitiesStart[offset]));
			elements.back().EntityHash = *pHash++;
			elements.back().MerkleComponentHash = *pHash++;
		}

		return true;
	}
}}
