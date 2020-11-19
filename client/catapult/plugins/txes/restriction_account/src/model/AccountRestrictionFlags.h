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
#include "catapult/utils/BitwiseEnum.h"
#include <stdint.h>

namespace catapult { namespace model {

	/// Account restriction flags.
	enum class AccountRestrictionFlags : uint16_t {
		/// Restriction type is an address.
		Address = 0x0001,

		/// Restriction type is a mosaic identifier.
		MosaicId = 0x0002,

		/// Restriction type is a transaction type.
		TransactionType = 0x0004,

		/// Restriction type sentinel.
		Sentinel = 0x0008,

		/// Restriction is interpreted as outgoing.
		Outgoing = 0x4000,

		/// Restriction is interpreted as blocking operation.
		Block = 0x8000
	};

	MAKE_BITWISE_ENUM(AccountRestrictionFlags)
}}
