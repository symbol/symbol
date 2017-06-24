#pragma once
#include "catapult/ionet/PacketPayload.h"
#include "catapult/model/EntityInfo.h"
#include <vector>

namespace catapult { namespace local {

	/// Creates a payload around \a pBlock for broadcasting.
	ionet::PacketPayload CreateBroadcastPayload(const std::shared_ptr<const model::Block>& pBlock);

	/// Creates a payload around \a infos for broadcasting.
	ionet::PacketPayload CreateBroadcastPayload(const std::vector<model::TransactionInfo>& infos);
}}
