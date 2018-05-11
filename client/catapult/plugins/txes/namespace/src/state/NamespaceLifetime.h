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
#include "catapult/exceptions.h"
#include "catapult/types.h"

namespace catapult { namespace state {

	/// Lifetime of a namespace.
	struct NamespaceLifetime {
	public:
		/// Creates a lifetime composed of a \a start height and an \a end height.
		explicit NamespaceLifetime(Height start, Height end)
				: Start(start)
				, End(end) {
			if (start >= end)
				CATAPULT_THROW_INVALID_ARGUMENT("namespace lifetime must be positive");
		}

	public:
		bool isActive(Height height) const {
			return height >= Start && height < End;
		}

	public:
		/// Returns \c true if this NamespaceLifetime is equal to \a rhs.
		bool operator==(const NamespaceLifetime& rhs) const {
			return Start == rhs.Start && End == rhs.End;
		}

		/// Returns \c true if this NamespaceLifetime is not equal to \a rhs.
		bool operator!=(const NamespaceLifetime& rhs) const {
			return !(*this == rhs);
		}

	public:
		/// Start height.
		Height Start;

		/// End height.
		Height End;
	};
}}
