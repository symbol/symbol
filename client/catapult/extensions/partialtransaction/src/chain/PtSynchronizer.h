#pragma once
#include "partialtransaction/src/PtTypes.h"
#include "catapult/chain/RemoteNodeSynchronizer.h"
#include "catapult/model/CosignedTransactionInfo.h"

namespace catapult { namespace api { class RemotePtApi; } }

namespace catapult { namespace chain {

	/// Creates a partial transactions synchronizer around the specified short hash pairs supplier (\a shortHashPairsSupplier)
	/// and partial transaction infos consumer (\a transactionInfosConsumer).
	RemoteNodeSynchronizer<api::RemotePtApi> CreatePtSynchronizer(
			const partialtransaction::ShortHashPairsSupplier& shortHashPairsSupplier,
			const partialtransaction::CosignedTransactionInfosConsumer& transactionInfosConsumer);
}}
