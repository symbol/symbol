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
#include "catapult/types.h"

namespace catapult { namespace state {

	/// Lifetime of a namespace.
	struct NamespaceLifetime {
	public:
		/// Creates a lifetime with \a start height and \a end height.
		NamespaceLifetime(Height start, Height end);

		/// Creates a lifetime with \a start height, \a end height and a grace period (\a gracePeriodDuration).
		NamespaceLifetime(Height start, Height end, BlockDuration gracePeriodDuration);

	public:
		/// Returns \c true if history is active at \a height (excluding grace period).
		bool isActiveAndUnlocked(Height height) const;

		/// Returns \c true if history is active at \a height (including grace period).
		bool isActiveOrGracePeriod(Height height) const;

	public:
		/// Returns \c true if this NamespaceLifetime is equal to \a rhs.
		bool operator==(const NamespaceLifetime& rhs) const;

		/// Returns \c true if this NamespaceLifetime is not equal to \a rhs.
		bool operator!=(const NamespaceLifetime& rhs) const;

	public:
		/// Start height.
		Height Start;

		/// End height excluding grace period.
		Height End;

		/// End height including grace period.
		Height GracePeriodEnd;
	};
}}
