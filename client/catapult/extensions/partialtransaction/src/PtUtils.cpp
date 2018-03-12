#include "PtUtils.h"
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace partialtransaction {

	std::unique_ptr<model::Transaction> StitchAggregate(const model::WeakCosignedTransactionInfo& transactionInfo) {
		uint32_t size = transactionInfo.transaction().Size
				+ sizeof(model::Cosignature) * static_cast<uint32_t>(transactionInfo.cosignatures().size());
		auto pTransactionWithCosignatures = utils::MakeUniqueWithSize<model::AggregateTransaction>(size);

		// copy transaction data
		memcpy(pTransactionWithCosignatures.get(), &transactionInfo.transaction(), transactionInfo.transaction().Size);
		pTransactionWithCosignatures->Size = size;

		// copy cosignatures
		auto* pCosignature = pTransactionWithCosignatures->CosignaturesPtr();
		for (const auto& cosignature : transactionInfo.cosignatures())
			*pCosignature++ = cosignature;

		return std::move(pTransactionWithCosignatures);
	}

	namespace {
		model::TransactionRange CopyIntoRange(const model::WeakCosignedTransactionInfo& transactionInfo) {
			auto pStitchedTransaction = StitchAggregate(transactionInfo);
			return model::TransactionRange::FromEntity(std::move(pStitchedTransaction));
		}
	}

	void SplitCosignedTransactionInfos(
			CosignedTransactionInfos&& transactionInfos,
			const consumer<model::TransactionRange&&>& transactionRangeConsumer,
			const consumer<model::DetachedCosignature&&>& cosignatureConsumer) {
		std::vector<model::TransactionRange> transactionRanges;
		for (const auto& transactionInfo : transactionInfos) {
			if (transactionInfo.pTransaction) {
				transactionRanges.push_back(CopyIntoRange({ transactionInfo.pTransaction.get(), &transactionInfo.Cosignatures }));
				continue;
			}

			for (const auto& cosignature : transactionInfo.Cosignatures)
				cosignatureConsumer(model::DetachedCosignature(cosignature.Signer, cosignature.Signature, transactionInfo.EntityHash));
		}

		if (!transactionRanges.empty())
			transactionRangeConsumer(model::TransactionRange::MergeRanges(std::move(transactionRanges)));
	}
}}
