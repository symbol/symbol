/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
		/// Creates a lifetime with \a start height and \a end height (including grace period).
		NamespaceLifetime(Height start, Height end);

		/// Creates a lifetime with \a start height, \a end height (excluding grace period) and grace period (\a gracePeriodDuration).
		NamespaceLifetime(Height start, Height end, BlockDuration gracePeriodDuration);

	public:
		/// Returns \c true if history is active at \a height.
		bool isActive(Height height) const;

		/// Returns \c true if history is active at \a height excluding grace period (\a gracePeriodDuration).
		bool isActiveExcludingGracePeriod(Height height, BlockDuration gracePeriodDuration) const;

	public:
		/// Returns \c true if this NamespaceLifetime is equal to \a rhs.
		bool operator==(const NamespaceLifetime& rhs) const;

		/// Returns \c true if this NamespaceLifetime is not equal to \a rhs.
		bool operator!=(const NamespaceLifetime& rhs) const;

	public:
		/// Start height.
		Height Start;

		/// End height including grace period.
		Height End;
	};
}}
