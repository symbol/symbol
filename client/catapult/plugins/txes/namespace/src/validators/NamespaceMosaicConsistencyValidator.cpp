#include "Validators.h"
#include "src/cache/NamespaceCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicDefinitionNotification;

	stateful::NotificationValidatorPointerT<Notification> CreateNamespaceMosaicConsistencyValidator() {
		return std::make_unique<stateful::FunctionalNotificationValidatorT<Notification>>(
				"NamespaceMosaicConsistencyValidator",
				[](const auto& notification, const ValidatorContext& context) {
					const auto& cache = context.Cache.sub<cache::NamespaceCache>();

					if (!cache.contains(notification.ParentId))
						return Failure_Namespace_Parent_Unknown;

					const auto& parentEntry = cache.get(notification.ParentId);
					const auto& root = parentEntry.root();
					if (!root.lifetime().isActive(context.Height))
						return Failure_Namespace_Expired;

					if (root.owner() != notification.Signer)
						return Failure_Namespace_Owner_Conflict;

					return ValidationResult::Success;
				});
	}
}}
