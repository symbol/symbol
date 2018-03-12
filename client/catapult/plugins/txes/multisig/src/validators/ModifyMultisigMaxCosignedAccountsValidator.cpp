#include "Validators.h"
#include "src/cache/MultisigCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::ModifyMultisigNewCosignerNotification;

	DECLARE_STATEFUL_VALIDATOR(ModifyMultisigMaxCosignedAccounts, Notification)(uint8_t maxCosignedAccountsPerAccount) {
		return MAKE_STATEFUL_VALIDATOR(ModifyMultisigMaxCosignedAccounts, [maxCosignedAccountsPerAccount](
				const auto& notification,
				const ValidatorContext& context) {
			const auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
			if (!multisigCache.contains(notification.CosignatoryKey))
				return ValidationResult::Success;

			const auto& cosignatoryEntry = multisigCache.get(notification.CosignatoryKey);
			return cosignatoryEntry.multisigAccounts().size() >= maxCosignedAccountsPerAccount
					? Failure_Multisig_Modify_Max_Cosigned_Accounts
					: ValidationResult::Success;
		});
	}
}}
