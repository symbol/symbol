#include "BroadcastUtils.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockUtils.h"

namespace catapult { namespace ionet {

	PacketPayload CreateBroadcastPayload(const std::shared_ptr<const model::Block>& pBlock) {
		return PacketPayload::FromEntity(PacketType::Push_Block, pBlock);
	}

	PacketPayload CreateBroadcastPayload(const std::vector<model::TransactionInfo>& transactionInfos) {
		return CreateBroadcastPayload(transactionInfos, PacketType::Push_Transactions);
	}

	PacketPayload CreateBroadcastPayload(const std::vector<model::TransactionInfo>& transactionInfos, PacketType packetType) {
		model::Transactions transactions(transactionInfos.size());
		for (auto i = 0u; i < transactionInfos.size(); ++i)
			transactions[i] = transactionInfos[i].pEntity;

		return PacketPayload::FromEntities(packetType, transactions);
	}

	PacketPayload CreateBroadcastPayload(const std::vector<model::DetachedCosignature>& cosignatures) {
		PacketPayloadBuilder builder(PacketType::Push_Detached_Cosignatures);
		for (const auto& cosignature : cosignatures)
			builder.appendValue(cosignature);

		return builder.build();
	}
}}
