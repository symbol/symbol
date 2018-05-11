/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "NamespaceConstants.h"
#include "catapult/preprocessor.h"

namespace catapult { namespace model {

	/// Constraints for a namespace's lifetime.
	struct NamespaceLifetimeConstraints {
	public:
		/// Creates constraints around \a maxDuration, \a gracePeriodDuration and \a maxRollbackBlocks.
		constexpr explicit NamespaceLifetimeConstraints(
				BlockDuration maxDuration,
				BlockDuration gracePeriodDuration,
				uint32_t maxRollbackBlocks)
				: TotalGracePeriodDuration(gracePeriodDuration.unwrap() + maxRollbackBlocks)
				, MaxNamespaceDuration(maxDuration.unwrap() + gracePeriodDuration.unwrap())
		{}

	public:
		/// Total grace period duration including the possibility of a chain rollback.
		BlockDuration TotalGracePeriodDuration;

		/// Maximum lifetime a namespace may have including the grace period.
		BlockDuration MaxNamespaceDuration;

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
