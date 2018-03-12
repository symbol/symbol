#include "Validators.h"
#include "catapult/utils/ArraySet.h"

namespace catapult { namespace validators {

	using Notification = model::AggregateCosignaturesNotification;

	DECLARE_STATELESS_VALIDATOR(BasicAggregateCosignatures, Notification)(uint32_t maxTransactions, uint8_t maxCosignatures) {
		return MAKE_STATELESS_VALIDATOR(BasicAggregateCosignatures, ([maxTransactions, maxCosignatures](const auto& notification) {
			if (0 == notification.TransactionsCount)
				return Failure_Aggregate_No_Transactions;

			if (maxTransactions < notification.TransactionsCount)
				return Failure_Aggregate_Too_Many_Transactions;

			if (maxCosignatures < notification.CosignaturesCount + 1)
				return Failure_Aggregate_Too_Many_Cosignatures;

			utils::KeyPointerSet cosigners;
			cosigners.insert(&notification.Signer);
			const auto* pCosignature = notification.CosignaturesPtr;
			for (auto i = 0u; i < notification.CosignaturesCount; ++i) {
				if (!cosigners.insert(&pCosignature->Signer).second)
					return Failure_Aggregate_Redundant_Cosignatures;

				++pCosignature;
			}

			return ValidationResult::Success;
		}));
	}
}}
