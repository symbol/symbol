#pragma once
#include "PtTypes.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/model/WeakCosignedTransactionInfo.h"

namespace catapult { namespace partialtransaction {

	/// Stitches a weak cosigned transaction \a info into a full aggregate transaction.
	std::unique_ptr<model::Transaction> StitchAggregate(const model::WeakCosignedTransactionInfo& transactionInfo);

	/// Splits up cosigned transaction infos (\a transactionInfos) and forwards transactions as a single range
	/// to \a transactionRangeConsumer and cosignatures individually to \a cosignatureConsumer.
	void SplitCosignedTransactionInfos(
			CosignedTransactionInfos&& transactionInfos,
			const consumer<model::TransactionRange&&>& transactionRangeConsumer,
			const consumer<model::DetachedCosignature&&>& cosignatureConsumer);
}}
