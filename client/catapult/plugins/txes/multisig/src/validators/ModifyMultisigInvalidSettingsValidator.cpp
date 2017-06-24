#include "Validators.h"
#include "src/cache/MultisigCache.h"
#include "catapult/utils/Functional.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/RawBuffer.h"
#include "catapult/validators/ValidatorContext.h"
#include <limits>

namespace catapult { namespace validators {

	using Notification = model::ModifyMultisigSettingsNotification;

	namespace {
		bool IsValid(int value, uint8_t max) {
			return value > 0 && value <= max;
		}
	}

	stateful::NotificationValidatorPointerT<Notification> CreateModifyMultisigInvalidSettingsValidator(uint8_t maxCosignersPerAccount) {
		return std::make_unique<stateful::FunctionalNotificationValidatorT<Notification>>(
				"ModifyMultisigInvalidSettingsValidator",
				[maxCosignersPerAccount](const auto& notification, const ValidatorContext& context) {
					const auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
					if (!multisigCache.contains(notification.Signer))
						return Failure_Multisig_Modify_Unknown_Multisig_Account;

					const auto& multisigEntry = multisigCache.get(notification.Signer);
					int newMinRemoval = multisigEntry.minRemoval() + notification.MinRemovalDelta;
					int newMinApproval = multisigEntry.minApproval() + notification.MinApprovalDelta;
					if (!IsValid(newMinRemoval, maxCosignersPerAccount) || !IsValid(newMinApproval, maxCosignersPerAccount))
						return Failure_Multisig_Modify_Min_Setting_Out_Of_Range;

					int maxValue = static_cast<int>(multisigEntry.cosignatories().size());
					if (newMinRemoval > maxValue || newMinApproval > maxValue)
						return Failure_Multisig_Modify_Min_Setting_Larger_Than_Num_Cosignatories;

					return ValidationResult::Success;
				});
	}
}}
