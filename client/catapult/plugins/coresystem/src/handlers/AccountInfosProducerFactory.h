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
#include "catapult/handlers/HandlerTypes.h"
#include "catapult/model/RangeTypes.h"
#include <vector>

namespace catapult {
	namespace cache { class AccountStateCache; }
	namespace model { struct AccountInfo; }
}

namespace catapult { namespace handlers {

	/// Alias for creating an account info producer given a range of addresses.
	using AccountInfosProducerFactory = SharedPointerProducerFactory<Address, model::AccountInfo>;

	/// Creates an account infos producer factory around \a accountStateCache.
	AccountInfosProducerFactory CreateAccountInfosProducerFactory(const cache::AccountStateCache& accountStateCache);
}}
