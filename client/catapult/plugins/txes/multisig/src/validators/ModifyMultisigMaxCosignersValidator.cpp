#include "Validators.h"
#include "src/cache/MultisigCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::ModifyMultisigCosignersNotification;

	DECLARE_STATEFUL_VALIDATOR(ModifyMultisigMaxCosigners, Notification)(uint8_t maxCosignersPerAccount) {
		return MAKE_STATEFUL_VALIDATOR(ModifyMultisigMaxCosigners, [maxCosignersPerAccount](
				const auto& notification,
				const ValidatorContext& context) {
			size_t numCosignatories = 0u;
			const auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
			if (multisigCache.contains(notification.Signer)) {
				const auto& multisigAccountEntry = multisigCache.get(notification.Signer);
				numCosignatories = multisigAccountEntry.cosignatories().size();
			}

			const auto* pCosignatoryModification = notification.ModificationsPtr;
			for (auto i = 0u; i < notification.ModificationsCount; ++i) {
				if (model::CosignatoryModificationType::Add == pCosignatoryModification->ModificationType)
					++numCosignatories;
				else
					--numCosignatories;

				++pCosignatoryModification;
			}

			return numCosignatories > maxCosignersPerAccount ? Failure_Multisig_Modify_Max_Cosigners : ValidationResult::Success;
		});
	}
}}
