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
			, End(end) {
		if (Start >= End)
			ThrowInvalidLifetimeException("namespace lifetime must be positive", start, end, gracePeriodDuration);

		// if end is max height, there can be no grace period
		if (end != Height(std::numeric_limits<Height::ValueType>::max()))
			End = Height(End.unwrap() + gracePeriodDuration.unwrap());

		if (End < end)
			ThrowInvalidLifetimeException("namespace grace period end overflow detected", start, end, gracePeriodDuration);
	}

	bool NamespaceLifetime::isActive(Height height) const {
		return height >= Start && height < End;
	}

	bool NamespaceLifetime::isActiveExcludingGracePeriod(Height height, BlockDuration gracePeriodDuration) const {
		if (gracePeriodDuration.unwrap() >= (End - Start).unwrap())
			CATAPULT_THROW_INVALID_ARGUMENT_1("grace period duration cannot be larger than lifetime", gracePeriodDuration);

		return height >= Start && height < Height(End.unwrap() - gracePeriodDuration.unwrap());
	}

	bool NamespaceLifetime::operator==(const NamespaceLifetime& rhs) const {
		return Start == rhs.Start && End == rhs.End;
	}

	bool NamespaceLifetime::operator!=(const NamespaceLifetime& rhs) const {
		return !(*this == rhs);
	}
}}
