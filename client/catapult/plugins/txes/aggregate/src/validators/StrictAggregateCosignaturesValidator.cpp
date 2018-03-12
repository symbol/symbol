#include "Validators.h"
#include "catapult/utils/ArraySet.h"
#include <unordered_map>

namespace catapult { namespace validators {

	using Notification = model::AggregateCosignaturesNotification;

	namespace {
		const model::EmbeddedTransaction* AdvanceNext(const model::EmbeddedTransaction* pTransaction) {
			const auto* pTransactionData = reinterpret_cast<const uint8_t*>(pTransaction);
			return reinterpret_cast<const model::EmbeddedTransaction*>(pTransactionData + pTransaction->Size);
		}
	}

	DEFINE_STATELESS_VALIDATOR(StrictAggregateCosignatures, [](const auto& notification) {
		// collect all cosigners (initially set used flag to false)
		utils::ArrayPointerFlagMap<Key> cosigners;
		cosigners.emplace(&notification.Signer, false);
		const auto* pCosignature = notification.CosignaturesPtr;
		for (auto i = 0u; i < notification.CosignaturesCount; ++i) {
			cosigners.emplace(&pCosignature->Signer, false);
			++pCosignature;
		}

		// check all transaction signers and mark cosigners as used
		// notice that ineligible cosigners must dominate missing cosigners in order for cosigner aggregation to work
		auto hasMissingCosigners = false;
		const auto* pTransaction = notification.TransactionsPtr;
		for (auto i = 0u; i < notification.TransactionsCount; ++i) {
			auto iter = cosigners.find(&pTransaction->Signer);
			if (cosigners.cend() == iter)
				hasMissingCosigners = true;
			else
				iter->second = true;

			pTransaction = AdvanceNext(pTransaction);
		}

		// only return success if all cosigners are used
		return std::all_of(cosigners.cbegin(), cosigners.cend(), [](const auto& pair) { return pair.second; })
				? hasMissingCosigners ? Failure_Aggregate_Missing_Cosigners : ValidationResult::Success
				: Failure_Aggregate_Ineligible_Cosigners;
	});
}}
