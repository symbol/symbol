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

#include "PtUtils.h"
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/preprocessor.h"

namespace catapult { namespace partialtransaction {

	std::unique_ptr<model::Transaction> StitchAggregate(const model::WeakCosignedTransactionInfo& transactionInfo) {
		auto numCosignatures = static_cast<uint32_t>(transactionInfo.cosignatures().size());
		uint32_t size = transactionInfo.transaction().Size + SizeOf32<model::Cosignature>() * numCosignatures;
		auto pTransactionWithCosignatures = utils::MakeUniqueWithSize<model::AggregateTransaction>(size);

		// copy transaction data
		auto transactionSize = transactionInfo.transaction().Size;
		std::memcpy(static_cast<void*>(pTransactionWithCosignatures.get()), &transactionInfo.transaction(), transactionSize);
		pTransactionWithCosignatures->Size = size;

		// copy cosignatures
		auto* pCosignature = pTransactionWithCosignatures->CosignaturesPtr();
		for (const auto& cosignature : transactionInfo.cosignatures())
			*pCosignature++ = cosignature;

		return PORTABLE_MOVE(pTransactionWithCosignatures);
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

			for (const auto& cosignature : transactionInfo.Cosignatures) {
				cosignatureConsumer(model::DetachedCosignature(
						cosignature.SignerPublicKey,
						cosignature.Signature,
						transactionInfo.EntityHash));
			}
		}

		if (!transactionRanges.empty())
			transactionRangeConsumer(model::TransactionRange::MergeRanges(std::move(transactionRanges)));
	}
}}
