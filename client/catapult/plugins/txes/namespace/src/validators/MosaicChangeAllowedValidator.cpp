#include "Validators.h"
#include "ActiveMosaicView.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicChangeNotification;

	stateful::NotificationValidatorPointerT<Notification> CreateMosaicChangeAllowedValidator() {
		return std::make_unique<stateful::FunctionalNotificationValidatorT<Notification>>(
				"MosaicChangeAllowedValidator",
				[](const auto& notification, const ValidatorContext& context) {
					auto view = ActiveMosaicView(context.Cache);

					const state::MosaicEntry* pEntry;
					return view.tryGet(notification.MosaicId, context.Height, notification.Signer, &pEntry);
				});
	}
}}
