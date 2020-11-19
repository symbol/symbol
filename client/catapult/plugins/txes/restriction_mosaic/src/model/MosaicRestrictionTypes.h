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
#include <stdint.h>

namespace catapult { namespace model {

	/// Mosaic restriction types.
	enum class MosaicRestrictionType : uint8_t {
		/// Uninitialized value indicating no restriction.
		NONE,

		/// Allow if equal.
		EQ,

		/// Allow if not equal.
		NE,

		/// Allow if less than.
		LT,

		/// Allow if less than or equal.
		LE,

		/// Allow if greater than.
		GT,

		/// Allow if greater than or equal.
		GE
	};
}}
