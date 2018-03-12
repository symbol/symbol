#include "Validators.h"
#include "src/cache/MultisigCache.h"
#include "catapult/utils/Functional.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/RawBuffer.h"
#include "catapult/validators/ValidatorContext.h"
#include <limits>

namespace catapult { namespace validators {

	using Notification = model::ModifyMultisigSettingsNotification;

	DEFINE_STATEFUL_VALIDATOR(ModifyMultisigInvalidSettings, [](const auto& notification, const ValidatorContext& context) {
		const auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
		if (!multisigCache.contains(notification.Signer)) {
			// since the ModifyMultisigInvalidCosignersValidator and the ModifyMultisigCosignersObserver ran before
			// this validator, the only scenario in which the multisig account cannot be found in the multisig cache
			// is that the observer removed the last cosigner reverting the multisig account to a normal accounts

			// MinRemovalDelta and MinApprovalDelta are both expected to be -1 in this case
			if (-1 != notification.MinRemovalDelta || -1 != notification.MinApprovalDelta)
				return Failure_Multisig_Modify_Min_Setting_Out_Of_Range;

			return ValidationResult::Success;
		}

		const auto& multisigEntry = multisigCache.get(notification.Signer);
		int newMinRemoval = multisigEntry.minRemoval() + notification.MinRemovalDelta;
		int newMinApproval = multisigEntry.minApproval() + notification.MinApprovalDelta;
		if (1 > newMinRemoval || 1 > newMinApproval)
			return Failure_Multisig_Modify_Min_Setting_Out_Of_Range;

		int maxValue = static_cast<int>(multisigEntry.cosignatories().size());
		if (newMinRemoval > maxValue || newMinApproval > maxValue)
			return Failure_Multisig_Modify_Min_Setting_Larger_Than_Num_Cosignatories;

		return ValidationResult::Success;
	});
}}
