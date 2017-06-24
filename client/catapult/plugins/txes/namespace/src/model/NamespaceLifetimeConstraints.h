#pragma once
#include "NamespaceConstants.h"
#include "catapult/preprocessor.h"

namespace catapult { namespace model {

	/// Constraints for a namespace's lifetime.
	struct NamespaceLifetimeConstraints {
	public:
		/// Creates constraints around \a maxDuration, \a gracePeriodDuration and \a maxRollbackBlocks.
		constexpr explicit NamespaceLifetimeConstraints(
				ArtifactDuration maxDuration,
				ArtifactDuration gracePeriodDuration,
				uint32_t maxRollbackBlocks)
				: TotalGracePeriodDuration(gracePeriodDuration.unwrap() + maxRollbackBlocks)
				, MaxNamespaceDuration(maxDuration.unwrap() + gracePeriodDuration.unwrap())
		{}

	public:
		/// The total grace period duration including the possibility of a chain rollback.
		ArtifactDuration TotalGracePeriodDuration;

		/// The maximum lifetime a namespace may have including the grace period.
		ArtifactDuration MaxNamespaceDuration;

	public:
		/// Returns a value indicating whether or not \a height is within the total grace period given \a lifetimeEnd.
		CPP14_CONSTEXPR
		bool IsWithinLifetimePlusDuration(Height lifetimeEnd, Height height) const {
			auto rawHeight = height.unwrap();
			auto rawDuration = TotalGracePeriodDuration.unwrap();
			return rawHeight <= rawDuration || lifetimeEnd.unwrap() > (rawHeight - rawDuration);
		}
	};
}}
