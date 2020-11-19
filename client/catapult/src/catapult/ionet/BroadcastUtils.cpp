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

#include "BroadcastUtils.h"
#include "PacketPayloadFactory.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockUtils.h"

namespace catapult { namespace ionet {

	PacketPayload CreateBroadcastPayload(const std::shared_ptr<const model::Block>& pBlock) {
		return PacketPayloadFactory::FromEntity(PacketType::Push_Block, pBlock);
	}

	PacketPayload CreateBroadcastPayload(const std::vector<model::TransactionInfo>& transactionInfos) {
		return CreateBroadcastPayload(transactionInfos, PacketType::Push_Transactions);
	}

	PacketPayload CreateBroadcastPayload(const std::vector<model::TransactionInfo>& transactionInfos, PacketType packetType) {
		PacketPayloadBuilder builder(packetType);
		for (const auto& transactionInfo : transactionInfos)
			builder.appendEntity(transactionInfo.pEntity);

		return builder.build();
	}

	PacketPayload CreateBroadcastPayload(const std::vector<model::DetachedCosignature>& cosignatures) {
		PacketPayloadBuilder builder(PacketType::Push_Detached_Cosignatures);
		builder.appendValues(cosignatures);
		return builder.build();
	}
}}
