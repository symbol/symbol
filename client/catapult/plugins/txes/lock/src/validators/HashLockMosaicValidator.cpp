#include "Validators.h"
#include "catapult/constants.h"

namespace catapult { namespace validators {

	using Notification = model::HashLockMosaicNotification;

	DECLARE_STATELESS_VALIDATOR(HashLockMosaic, Notification)(Amount lockedFundsPerAggregate) {
		return MAKE_STATELESS_VALIDATOR(HashLockMosaic, ([lockedFundsPerAggregate](const auto& notification) {
			if (lockedFundsPerAggregate != notification.Mosaic.Amount)
				return Failure_Lock_Invalid_Mosaic_Amount;

			return Xem_Id != notification.Mosaic.MosaicId ? Failure_Lock_Invalid_Mosaic_Id : ValidationResult::Success;
		}));
	}
}}

