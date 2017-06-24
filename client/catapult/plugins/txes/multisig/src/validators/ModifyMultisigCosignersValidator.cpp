#include "Validators.h"
#include "catapult/utils/HashSet.h"
#include <unordered_set>

namespace catapult { namespace validators {

	using Notification = model::ModifyMultisigCosignersNotification;

	namespace {
		constexpr bool IsValidModificationType(model::CosignatoryModificationType type) {
			return model::CosignatoryModificationType::Add == type || model::CosignatoryModificationType::Del == type;
		}
	}

	stateless::NotificationValidatorPointerT<Notification> CreateModifyMultisigCosignersValidator() {
		return std::make_unique<stateless::FunctionalNotificationValidatorT<Notification>>(
				"ModifyMultisigCosignersValidator",
				[](const auto& notification) {
					utils::KeyPointerSet addedAccounts;
					utils::KeyPointerSet removedAccounts;

					auto pModifications = notification.ModificationsPtr;
					for (auto i = 0u; i < notification.ModificationsCount; ++i) {
						if (!IsValidModificationType(pModifications[i].ModificationType))
							return Failure_Multisig_Modify_Unsupported_Modification_Type;

						auto& accounts = model::CosignatoryModificationType::Add == pModifications[i].ModificationType
								? addedAccounts
								: removedAccounts;
						const auto& oppositeAccounts = &accounts == &addedAccounts ? removedAccounts : addedAccounts;

						auto& key = pModifications[i].CosignatoryPublicKey;
						if (oppositeAccounts.end() != oppositeAccounts.find(&key))
							return Failure_Multisig_Modify_Account_In_Both_Sets;

						accounts.insert(&key);
					}

					if (1 < removedAccounts.size())
						return Failure_Multisig_Modify_Multiple_Deletes;

					if (notification.ModificationsCount != addedAccounts.size() + removedAccounts.size())
						return Failure_Multisig_Modify_Redundant_Modifications;

					return ValidationResult::Success;
				});
	}
}}
