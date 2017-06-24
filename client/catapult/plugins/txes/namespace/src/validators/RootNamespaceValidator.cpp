#include "Validators.h"
#include "src/model/IdGenerator.h"
#include "catapult/utils/Hashers.h"

namespace catapult { namespace validators {

	using Notification = model::RootNamespaceNotification;

	stateless::NotificationValidatorPointerT<Notification> CreateRootNamespaceValidator(
			ArtifactDuration maxDuration,
			const std::unordered_set<std::string>& reservedRootNamespaceNames) {
		std::unordered_set<NamespaceId, utils::BaseValueHasher<NamespaceId>> reservedRootIds;
		for (const auto& name : reservedRootNamespaceNames)
			reservedRootIds.emplace(model::GenerateNamespaceId(Namespace_Base_Id, name));

		return std::make_unique<stateless::FunctionalNotificationValidatorT<Notification>>(
				"RootNamespaceValidator",
				[maxDuration, reservedRootIds](const auto& notification) {
			// note that zero duration is acceptable because it is eternal
			if (maxDuration < notification.Duration)
				return Failure_Namespace_Invalid_Duration;

			if (reservedRootIds.cend() != reservedRootIds.find(notification.NamespaceId))
				return Failure_Namespace_Root_Name_Reserved;

			return ValidationResult::Success;
		});
	}
}}
