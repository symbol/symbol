#include "Validators.h"
#include "src/cache/NamespaceCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::ChildNamespaceNotification;

	DEFINE_STATEFUL_VALIDATOR(ChildNamespaceAvailability, [](const auto& notification, const ValidatorContext& context) {
		const auto& cache = context.Cache.sub<cache::NamespaceCache>();
		auto height = context.Height;

		if (cache.contains(notification.NamespaceId))
			return Failure_Namespace_Already_Exists;

		if (!cache.contains(notification.ParentId))
			return Failure_Namespace_Parent_Unknown;

		const auto& parentEntry = cache.get(notification.ParentId);
		const auto& parentPath = parentEntry.ns().path();
		if (parentPath.size() == parentPath.capacity())
			return Failure_Namespace_Too_Deep;

		const auto& root = parentEntry.root();
		if (!root.lifetime().isActive(height))
			return Failure_Namespace_Expired;

		if (root.owner() != notification.Signer)
			return Failure_Namespace_Owner_Conflict;

		return ValidationResult::Success;
	});
}}
