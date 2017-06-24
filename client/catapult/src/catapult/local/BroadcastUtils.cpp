#include "BroadcastUtils.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockUtils.h"

namespace catapult { namespace local {

	ionet::PacketPayload CreateBroadcastPayload(const std::shared_ptr<const model::Block>& pBlock) {
		return ionet::PacketPayload::FromEntity(ionet::PacketType::Push_Block, pBlock);
	}

	ionet::PacketPayload CreateBroadcastPayload(const std::vector<model::TransactionInfo>& infos) {
		model::Transactions transactions(infos.size());
		for (auto i = 0u; i < infos.size(); ++i)
			transactions[i] = infos[i].pEntity;

		return ionet::PacketPayload::FromEntities(ionet::PacketType::Push_Transactions, transactions);
	}
}}
