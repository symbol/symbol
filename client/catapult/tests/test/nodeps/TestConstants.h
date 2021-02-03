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

namespace catapult { namespace test {

	/// Default (well known) namespace id used in tests (`cat`).
	/// \note Cannot use type NamespaceId because it is defined in plugin.
	constexpr uint64_t Default_Namespace_Id(0xB149'7F5F'BA65'1B4F);

	/// Default (well known) currency mosaic id used in tests (`currency`).
	constexpr MosaicId Default_Currency_Mosaic_Id(0x4B39'06B7'32A9'04B0);

	/// Default (well known) harvesting mosaic id used in tests (`harvest`).
	constexpr MosaicId Default_Harvesting_Mosaic_Id(0x6648'43D5'56E6'27F1);

	/// Default total chain importance used for scaling block target calculation.
	constexpr Importance Default_Total_Chain_Importance(8'999'999'998);

	/// File database batch size used in tests.
	constexpr uint32_t File_Database_Batch_Size = 100;

	/// Network generation hash seed string used by deterministic tests.
	constexpr auto Deterministic_Network_Generation_Hash_Seed_String = "070D67A92D441EAAD25AB5C78F1F68628BE33EAA1DEBEDBE14D4FBE8F4DC326E";
}}
