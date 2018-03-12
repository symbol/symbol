#include "Validators.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicSupplyChangeNotification;

	namespace {
		constexpr bool IsValidDirection(model::MosaicSupplyChangeDirection direction) {
			return direction <= model::MosaicSupplyChangeDirection::Increase;
		}
	}

	DEFINE_STATELESS_VALIDATOR(MosaicSupplyChange, [](const auto& notification) {
		if (!IsValidDirection(notification.Direction))
			return Failure_Mosaic_Invalid_Supply_Change_Direction;

		return Amount() == notification.Delta
				? Failure_Mosaic_Invalid_Supply_Change_Amount
				: ValidationResult::Success;
	});
}}
