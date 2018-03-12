#include "Validators.h"
#include "src/cache/NamespaceCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::ChildNamespaceNotification;

	DECLARE_STATEFUL_VALIDATOR(RootNamespaceMaxChildren, Notification)(uint16_t maxChildren) {
		return MAKE_STATEFUL_VALIDATOR(RootNamespaceMaxChildren, ([maxChildren](
				const auto& notification,
				const ValidatorContext& context) {
			const auto& cache = context.Cache.sub<cache::NamespaceCache>();
			const auto& parentEntry = cache.get(notification.ParentId);
			return maxChildren <= parentEntry.root().size() ? Failure_Namespace_Max_Children_Exceeded : ValidationResult::Success;
		}));
	}
}}
