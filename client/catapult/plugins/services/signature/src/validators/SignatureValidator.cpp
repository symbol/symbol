#include "Validators.h"
#include "catapult/crypto/Signer.h"

namespace catapult { namespace validators {

	using Notification = model::SignatureNotification;

	DEFINE_STATELESS_VALIDATOR(Signature, [](const auto& notification) {
		return crypto::Verify(notification.Signer, notification.Data, notification.Signature)
				? ValidationResult::Success
				: Failure_Signature_Not_Verifiable;
	});
}}
