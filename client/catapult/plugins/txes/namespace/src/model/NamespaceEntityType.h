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
#ifndef CUSTOM_ENTITY_TYPE_DEFINITION
#include "catapult/model/EntityType.h"

namespace catapult {
namespace model {

#endif

	/// Namespace registration transaction.
	DEFINE_TRANSACTION_TYPE(Namespace, Namespace_Registration, 0x1);

	/// Address alias transaction.
	DEFINE_TRANSACTION_TYPE(Namespace, Alias_Address, 0x2);

	/// Mosaic alias transaction.
	DEFINE_TRANSACTION_TYPE(Namespace, Alias_Mosaic, 0x3);

#ifndef CUSTOM_ENTITY_TYPE_DEFINITION
}
}
#endif
