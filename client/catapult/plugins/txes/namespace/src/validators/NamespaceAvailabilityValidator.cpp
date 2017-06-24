#include "Validators.h"
#include "src/cache/NamespaceCache.h"
#include "src/model/NamespaceLifetimeConstraints.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::RootNamespaceNotification;

	namespace {
		constexpr bool IsEternal(const state::NamespaceLifetime& lifetime) {
			return Height(std::numeric_limits<Height::ValueType>::max()) == lifetime.End;
		}

		constexpr Height ToHeight(ArtifactDuration duration) {
			return Height(duration.unwrap());
		}
	}

	stateful::NotificationValidatorPointerT<Notification> CreateRootNamespaceAvailabilityValidator(
			const model::NamespaceLifetimeConstraints& constraints) {
		return std::make_unique<stateful::FunctionalNotificationValidatorT<Notification>>(
				"RootNamespaceAvailabilityValidator",
				[constraints](const auto& notification, const ValidatorContext& context) {
					const auto& cache = context.Cache.sub<cache::NamespaceCache>();
					auto height = context.Height;

					if (Height(1) != height && Eternal_Artifact_Duration == notification.Duration)
						return Failure_Namespace_Eternal_After_Nemesis_Block;

					if (!cache.contains(notification.NamespaceId))
						return ValidationResult::Success;

					const auto& root = cache.get(notification.NamespaceId).root();
					if (IsEternal(root.lifetime()) || Eternal_Artifact_Duration == notification.Duration)
						return Failure_Namespace_Invalid_Duration;

					// if grace period after expiration has passed, any signer can claim the namespace
					if (!constraints.IsWithinLifetimePlusDuration(root.lifetime().End, height))
						return ValidationResult::Success;

					auto newLifetimeEnd = root.lifetime().End + ToHeight(notification.Duration);
					auto maxLifetimeEnd = height + ToHeight(constraints.MaxNamespaceDuration);
					if (newLifetimeEnd > maxLifetimeEnd)
						return Failure_Namespace_Invalid_Duration;

					return root.owner() == notification.Signer ? ValidationResult::Success : Failure_Namespace_Owner_Conflict;
				});
	}

	stateful::NotificationValidatorPointerT<model::ChildNamespaceNotification> CreateChildNamespaceAvailabilityValidator() {
		return std::make_unique<stateful::FunctionalNotificationValidatorT<model::ChildNamespaceNotification>>(
				"ChildNamespaceAvailabilityValidator",
				[](const auto& notification, const ValidatorContext& context) {
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
	}
}}
