#include "Validators.h"
#include "catapult/model/NetworkInfo.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::SignatureNotification;

	stateful::NotificationValidatorPointerT<Notification> CreateNemesisSinkValidator() {
		return std::make_unique<stateful::FunctionalNotificationValidatorT<Notification>>(
				"NemesisSinkValidator",
				[](const auto& notification, const auto& context) {
					auto isBlockHeightOne = context.Height == Height(1);
					auto isNemesisPublicKey = notification.Signer == context.Network.PublicKey;
					return isBlockHeightOne || !isNemesisPublicKey
							? ValidationResult::Success
							: Failure_Core_Nemesis_Account_Signed_After_Nemesis_Block;
				});
	}
}}
