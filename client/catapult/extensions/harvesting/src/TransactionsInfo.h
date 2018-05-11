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
#include "catapult/model/BlockUtils.h"

namespace catapult { namespace cache { class MemoryUtCache; } }

namespace catapult { namespace harvesting {

	/// Information about transactions.
	struct TransactionsInfo {
		/// Transactions.
		model::Transactions Transactions;

		/// Aggregate transactions hash.
		Hash256 TransactionsHash;
	};

	/// Supplies a transactions info composed of a maximum number of transactions.
	using TransactionsInfoSupplier = std::function<TransactionsInfo (uint32_t)>;

	/// Creates a default transactions info supplier around \a utCache.
	TransactionsInfoSupplier CreateTransactionsInfoSupplier(const cache::MemoryUtCache& utCache);
}}
