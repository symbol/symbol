#include "Validators.h"

namespace catapult { namespace validators {

	using Notification = model::NamespaceNotification;

	namespace {
		constexpr bool IsValidNamespaceType(model::NamespaceType namespaceType) {
			return namespaceType <= model::NamespaceType::Child;
		}
	}

	DEFINE_STATELESS_VALIDATOR(NamespaceType, [](const auto& notification) {
		return IsValidNamespaceType(notification.NamespaceType)
				? ValidationResult::Success
				: Failure_Namespace_Invalid_Namespace_Type;
	});
}}
