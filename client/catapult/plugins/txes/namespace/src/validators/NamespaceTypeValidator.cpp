#include "Validators.h"

namespace catapult { namespace validators {

	using Notification = model::NamespaceNotification;

	namespace {
		constexpr bool IsValidNamespaceType(model::NamespaceType namespaceType) {
			return namespaceType <= model::NamespaceType::Child;
		}
	}

	stateless::NotificationValidatorPointerT<Notification> CreateNamespaceTypeValidator() {
		return std::make_unique<stateless::FunctionalNotificationValidatorT<Notification>>(
				"NamespaceTypeValidator",
				[](const auto& notification) {
			return IsValidNamespaceType(notification.NamespaceType)
					? ValidationResult::Success
					: Failure_Namespace_Invalid_Namespace_Type;
		});
	}
}}
