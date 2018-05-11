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
#include "types.h"

namespace catapult {

	/// Maximum number of parts for a namespace.
	constexpr size_t Namespace_Max_Depth = 3;

	/// Duration of eternal artifact.
	constexpr BlockDuration Eternal_Artifact_Duration(0);

	/// Base id for namespaces.
	constexpr NamespaceId Namespace_Base_Id(0);

	/// Number of rules for a mosaic levy.
	constexpr size_t Num_Mosaic_Levy_Rule_Ids = 5;

	/// NEM namespace id.
#ifdef SIGNATURE_SCHEME_NIS1
	constexpr NamespaceId Nem_Id(0x2912FDE512A2C7B8ULL);
#else
	constexpr NamespaceId Nem_Id(0x84B3552D375FFA4BULL);
#endif
}
