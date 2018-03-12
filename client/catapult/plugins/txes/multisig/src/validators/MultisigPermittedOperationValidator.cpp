#include "Validators.h"
#include "src/cache/MultisigCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::TransactionNotification;

	DEFINE_STATEFUL_VALIDATOR(MultisigPermittedOperation, [](const auto& notification, const ValidatorContext& context) {
		const auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
		if (!multisigCache.contains(notification.Signer))
			return ValidationResult::Success;

		return multisigCache.get(notification.Signer).cosignatories().empty()
				? ValidationResult::Success
				: Failure_Multisig_Operation_Not_Permitted_By_Account;
	});
}}
