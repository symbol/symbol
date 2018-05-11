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
#include "AccountInfo.h"
#include "Block.h"
#include "EntityRange.h"
#include "catapult/utils/ShortHash.h"

namespace catapult { namespace model {

	/// An entity range composed of blocks.
	using BlockRange = EntityRange<Block>;

	/// An entity range composed of transactions.
	using TransactionRange = EntityRange<Transaction>;

	/// An entity range composed of hashes.
	using HashRange = EntityRange<Hash256>;

	/// An entity range composed of short hashes.
	using ShortHashRange = EntityRange<utils::ShortHash>;

	/// An entity range composed of addresses.
	using AddressRange = EntityRange<Address>;

	/// An entity range composed of account infos.
	using AccountInfoRange = EntityRange<AccountInfo>;
}}
