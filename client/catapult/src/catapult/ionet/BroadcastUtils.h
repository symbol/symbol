#pragma once
#include "PacketPayload.h"
#include "catapult/model/Cosignature.h"
#include "catapult/model/EntityInfo.h"
#include <vector>

namespace catapult { namespace ionet {

	/// Creates a payload around \a pBlock for broadcasting.
	PacketPayload CreateBroadcastPayload(const std::shared_ptr<const model::Block>& pBlock);

	/// Creates a payload around \a transactionInfos for broadcasting.
	PacketPayload CreateBroadcastPayload(const std::vector<model::TransactionInfo>& transactionInfos);

	/// Creates a payload around \a transactionInfos for broadcasting using \a packetType.
	PacketPayload CreateBroadcastPayload(const std::vector<model::TransactionInfo>& transactionInfos, PacketType packetType);

	/// Creates a payload around \a cosignatures for broadcasting.
	PacketPayload CreateBroadcastPayload(const std::vector<model::DetachedCosignature>& cosignatures);
}}
