#include "Validators.h"
#include "src/model/IdGenerator.h"
#include "src/model/NameChecker.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicNameNotification;

	DECLARE_STATELESS_VALIDATOR(MosaicName, Notification)(uint8_t maxNameSize) {
		return MAKE_STATELESS_VALIDATOR(MosaicName, [maxNameSize](const auto& notification) {
			if (MosaicId() == notification.MosaicId)
				return Failure_Mosaic_Name_Reserved;

			if (maxNameSize < notification.NameSize || !model::IsValidName(notification.NamePtr, notification.NameSize))
				return Failure_Mosaic_Invalid_Name;

			auto name = utils::RawString(reinterpret_cast<const char*>(notification.NamePtr), notification.NameSize);
			if (notification.MosaicId != model::GenerateMosaicId(notification.ParentId, name))
				return Failure_Mosaic_Name_Id_Mismatch;

			return ValidationResult::Success;
		});
	}
}}
