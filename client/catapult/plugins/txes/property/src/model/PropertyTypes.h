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
#include "catapult/types.h"
#include <vector>
#include <stdint.h>

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Property types.
	enum class PropertyType : uint8_t {
		/// Property type is an address.
		Address = 0x01,

		/// Property type is a mosaic id.
		MosaicId = 0x02,

		/// Property type is a transaction type.
		TransactionType = 0x04,

		/// Property type sentinel.
		Sentinel = 0x05,

		/// Property is interpreted as blocking operation.
		Block = 0x80
	};

	MAKE_BITWISE_ENUM(PropertyType)

	/// Property modification type.
	enum class PropertyModificationType : uint8_t {
		/// Add property value.
		Add,

		/// Remove property value.
		Del
	};

	/// Binary layout for a property modification.
	template<typename TPropertyValue>
	struct PropertyModification {
	public:
		/// Modification type.
		PropertyModificationType ModificationType;

		/// Property value.
		TPropertyValue Value;
	};

#pragma pack(pop)

	/// Raw property modification.
	struct RawPropertyModification {
	public:
		/// Modification type.
		PropertyModificationType ModificationType;

		/// Property value.
		std::vector<uint8_t> Value;
	};
}}
