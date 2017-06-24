#include "Validators.h"
#include "src/model/IdGenerator.h"
#include "src/model/NameChecker.h"

namespace catapult { namespace validators {

	using Notification = model::NamespaceNameNotification;

	stateless::NotificationValidatorPointerT<Notification> CreateNamespaceNameValidator(uint8_t maxNameSize) {
		return std::make_unique<stateless::FunctionalNotificationValidatorT<Notification>>(
				"NamespaceNameValidator",
				[maxNameSize](const auto& notification) {
			if (maxNameSize < notification.NameSize || !model::IsValidName(notification.NamePtr, notification.NameSize))
				return Failure_Namespace_Invalid_Name;

			auto name = utils::RawString(reinterpret_cast<const char*>(notification.NamePtr), notification.NameSize);
			if (notification.NamespaceId != model::GenerateNamespaceId(notification.ParentId, name))
				return Failure_Namespace_Name_Id_Mismatch;

			return ValidationResult::Success;
		});
	}
}}
