#include "Validators.h"
#include "catapult/utils/HashSet.h"
#include <unordered_map>

namespace catapult { namespace validators {

	using Notification = model::AggregateCosignaturesNotification;
	// Key and Hash256 have same underlying type
	using KeyPointerFlagMap = std::unordered_map<const Hash256*, bool, utils::Hash256PointerHasher, utils::Hash256PointerEquality>;

	namespace {
		const model::EmbeddedEntity* AdvanceNext(const model::EmbeddedEntity* pTransaction) {
			return reinterpret_cast<const model::EmbeddedEntity*>(reinterpret_cast<const uint8_t*>(pTransaction) + pTransaction->Size);
		}
	}

	stateless::NotificationValidatorPointerT<Notification> CreateStrictAggregateCosignaturesValidator() {
		return std::make_unique<stateless::FunctionalNotificationValidatorT<Notification>>(
				"StrictAggregateCosignaturesValidator",
				[](const auto& notification) {
					// collect all cosigners (initially set used flag to false)
					KeyPointerFlagMap cosigners;
					cosigners.emplace(&notification.Signer, false);
					const auto* pCosignature = notification.CosignaturesPtr;
					for (auto i = 0u; i < notification.CosignaturesCount; ++i) {
						cosigners.emplace(&pCosignature->Signer, false);
						++pCosignature;
					}

					// check all transaction signers and mark cosigners as used
					const auto* pTransaction = notification.TransactionsPtr;
					for (auto i = 0u; i < notification.TransactionsCount; ++i) {
						auto iter = cosigners.find(&pTransaction->Signer);
						if (cosigners.cend() == iter)
							return Failure_Aggregate_Missing_Cosigners;

						iter->second = true;
						pTransaction = AdvanceNext(pTransaction);
					}

					// only return success if all cosigners are used
					return std::all_of(cosigners.cbegin(), cosigners.cend(), [](const auto& pair) { return pair.second; })
							? ValidationResult::Success
							: Failure_Aggregate_Ineligible_Cosigners;
				});
	}
}}
