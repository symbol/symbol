#include "Validators.h"
#include "catapult/crypto/Signer.h"

namespace catapult { namespace validators {

	using Notification = model::SignatureNotification;

	stateless::NotificationValidatorPointerT<Notification> CreateSignatureValidator() {
		return std::make_unique<stateless::FunctionalNotificationValidatorT<Notification>>(
				"SignatureValidator",
				[](const auto& notification) {
					return crypto::Verify(notification.Signer, notification.Data, notification.Signature)
							? ValidationResult::Success
							: Failure_Core_Signature_Not_Verifiable;
				});
	}
}}
