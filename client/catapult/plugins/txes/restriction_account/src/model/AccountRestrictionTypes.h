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
#include "catapult/utils/BitwiseEnum.h"
#include <vector>
#include <stdint.h>

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Account restriction types.
	enum class AccountRestrictionType : uint8_t {
		/// Account restriction type is an address.
		Address = 0x01,

		/// Account restriction type is a mosaic id.
		MosaicId = 0x02,

		/// Account restriction type is a transaction type.
		TransactionType = 0x04,

		/// Account restriction type sentinel.
		Sentinel = 0x05,

		/// Account restriction is interpreted as blocking operation.
		Block = 0x80
	};

	MAKE_BITWISE_ENUM(AccountRestrictionType)

	/// Account restriction modification type.
	enum class AccountRestrictionModificationType : uint8_t {
		/// Add restriction value.
		Add,

		/// Remove restriction value.
		Del
	};

	/// Binary layout for an account restriction modification.
	template<typename TRestrictionValue>
	struct AccountRestrictionModification {
	public:
		/// Modification type.
		AccountRestrictionModificationType ModificationType;

		/// Restriction value.
		TRestrictionValue Value;
	};

#pragma pack(pop)

	/// Raw account restriction modification.
	struct RawAccountRestrictionModification {
	public:
		/// Modification type.
		AccountRestrictionModificationType ModificationType;

		/// Restriction value.
		std::vector<uint8_t> Value;
	};
}}
