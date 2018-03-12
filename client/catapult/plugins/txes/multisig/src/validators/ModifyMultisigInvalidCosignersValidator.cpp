#include "Validators.h"
#include "src/cache/MultisigCache.h"
#include "catapult/utils/Hashers.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::ModifyMultisigCosignersNotification;

	DEFINE_STATEFUL_VALIDATOR(ModifyMultisigInvalidCosigners, [](const auto& notification, const ValidatorContext& context) {
		const auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
		const auto* pModifications = notification.ModificationsPtr;

		if (!multisigCache.contains(notification.Signer)) {
			for (auto i = 0u; i < notification.ModificationsCount; ++i) {
				if (model::CosignatoryModificationType::Del == pModifications[i].ModificationType)
					return Failure_Multisig_Modify_Unknown_Multisig_Account;
			}

			return ValidationResult::Success;
		}

		const auto& multisigEntry = multisigCache.get(notification.Signer);
		for (auto i = 0u; i < notification.ModificationsCount; ++i) {
			auto isCosignatory = multisigEntry.hasCosignatory(pModifications[i].CosignatoryPublicKey);
			auto isEnablingCosignatory = model::CosignatoryModificationType::Add == pModifications[i].ModificationType;

			if (!isEnablingCosignatory && !isCosignatory)
				return Failure_Multisig_Modify_Not_A_Cosigner;

			if (isEnablingCosignatory && isCosignatory)
				return Failure_Multisig_Modify_Already_A_Cosigner;
		}

		return ValidationResult::Success;
	});
}}
