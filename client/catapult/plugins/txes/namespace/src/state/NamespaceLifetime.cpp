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

#include "NamespaceLifetime.h"
#include "catapult/exceptions.h"

namespace catapult { namespace state {

	namespace {
		[[noreturn]]
		void ThrowInvalidLifetimeException(const char* message, Height start, Height end, BlockDuration gracePeriodDuration) {
			std::ostringstream out;
			out << message << ": (start = " << start << ", end = " << end << ", gracePeriodDuration = " << gracePeriodDuration << ")";
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}
	}

	NamespaceLifetime::NamespaceLifetime(Height start, Height end) : NamespaceLifetime(start, end, BlockDuration(0))
	{}

	NamespaceLifetime::NamespaceLifetime(Height start, Height end, BlockDuration gracePeriodDuration)
			: Start(start)
			, End(end)
			, GracePeriodEnd(end.unwrap() + gracePeriodDuration.unwrap()) {
		// if end is max height, there can be no grace period
		if (End == Height(std::numeric_limits<Height::ValueType>::max()))
			GracePeriodEnd = End;

		if (Start >= End)
			ThrowInvalidLifetimeException("namespace lifetime must be positive", start, end, gracePeriodDuration);

		if (GracePeriodEnd < End)
			ThrowInvalidLifetimeException("namespace grace period end overflow detected", start, end, gracePeriodDuration);
	}

	bool NamespaceLifetime::isActiveAndUnlocked(Height height) const {
		return height >= Start && height < End;
	}

	bool NamespaceLifetime::isActiveOrGracePeriod(Height height) const {
		return height >= Start && height < GracePeriodEnd;
	}

	bool NamespaceLifetime::operator==(const NamespaceLifetime& rhs) const {
		return Start == rhs.Start && End == rhs.End && GracePeriodEnd == rhs.GracePeriodEnd;
	}

	bool NamespaceLifetime::operator!=(const NamespaceLifetime& rhs) const {
		return !(*this == rhs);
	}
}}
