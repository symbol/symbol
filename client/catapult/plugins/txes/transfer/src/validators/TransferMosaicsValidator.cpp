#include "Validators.h"

namespace catapult { namespace validators {

	using Notification = model::TransferMosaicsNotification;

	stateless::NotificationValidatorPointerT<Notification> CreateTransferMosaicsValidator() {
		return std::make_unique<stateless::FunctionalNotificationValidatorT<Notification>>(
				"TransferMosaicsValidator",
				[](const auto& notification) {
					// check strict ordering of mosaics
					if (1 >= notification.MosaicsCount)
						return ValidationResult::Success;

					auto pMosaics = notification.MosaicsPtr;
					auto lastMosaicId = pMosaics[0].MosaicId;
					for (auto i = 1u; i < notification.MosaicsCount; ++i) {
						auto currentMosaicId = pMosaics[i].MosaicId;
						if (lastMosaicId >= currentMosaicId)
							return Failure_Transfer_Out_Of_Order_Mosaics;

						lastMosaicId = currentMosaicId;
					}

					return ValidationResult::Success;
				});
	}
}}
