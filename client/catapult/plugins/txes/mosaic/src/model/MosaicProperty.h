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
#include <stdint.h>

namespace catapult { namespace model {

	/// Available mosaic property ids.
	enum class MosaicPropertyId : uint8_t {
		// region required properties

		/// Mosaic flags.
		Flags,

		/// Mosaic divisibility.
		Divisibility,

		// endregion

		// region optional properties

		/// Mosaic duration.
		Duration,

		// endregion

		Sentinel_Property_Id
	};

	/// Mosaic property flags.
	enum class MosaicFlags : uint8_t {
		/// No flags present.
		None = 0x00,

		/// Mosaic supports supply changes even when mosaic owner owns partial supply.
		Supply_Mutable = 0x01,

		/// Mosaic supports transfers between arbitrary accounts.
		/// \note When not set, mosaic can only be transferred to and from mosaic owner.
		Transferable = 0x02,

		/// Mosaic supports custom restrictions configured by mosaic owner.
		Restrictable = 0x04,

		/// All flags.
		All = 0x07
	};

	MAKE_BITWISE_ENUM(MosaicFlags)

#pragma pack(push, 1)

	/// Mosaic property composed of an id and a value.
	struct MosaicProperty {
	public:
		/// Mosaic property id.
		MosaicPropertyId Id;

		/// Mosaic property value.
		uint64_t Value;
	};

#pragma pack(pop)

	/// Number of available properties.
	constexpr size_t Num_Mosaic_Properties = utils::to_underlying_type(MosaicPropertyId::Sentinel_Property_Id);

	/// Index of first optional property.
	constexpr size_t First_Optional_Property = utils::to_underlying_type(MosaicPropertyId::Duration);
}}
