#include "Validators.h"
#include "catapult/model/Block.h"
#include "catapult/utils/BaseValue.h"
#include "catapult/utils/NetworkTime.h"
#include "catapult/types.h"

namespace catapult { namespace validators {

	using Notification = model::BlockNotification;

	stateless::NotificationValidatorPointerT<Notification> CreateNonFutureBlockValidator(const utils::TimeSpan& maxBlockFutureTime) {
		return std::make_unique<stateless::FunctionalNotificationValidatorT<Notification>>(
				"NonFutureBlockValidator",
				[maxBlockFutureTime](const auto& notification) {
					auto currentTime = utils::NetworkTime();
					auto isTooFarAhead = currentTime + maxBlockFutureTime < notification.Timestamp;

					return isTooFarAhead
							? Failure_Core_Timestamp_Too_Far_In_Future
							: ValidationResult::Success;
				});
	}
}}
