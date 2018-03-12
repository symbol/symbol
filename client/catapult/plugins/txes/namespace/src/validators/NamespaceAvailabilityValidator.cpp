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

		constexpr Height ToHeight(BlockDuration duration) {
			return Height(duration.unwrap());
		}
	}

	DECLARE_STATEFUL_VALIDATOR(RootNamespaceAvailability, Notification)(const model::NamespaceLifetimeConstraints& constraints) {
		return MAKE_STATEFUL_VALIDATOR(RootNamespaceAvailability, [constraints](
				const auto& notification,
				const ValidatorContext& context) {
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
}}
