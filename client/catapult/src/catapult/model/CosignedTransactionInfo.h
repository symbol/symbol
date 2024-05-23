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
#include "Cosignature.h"
#include "Transaction.h"
#include <vector>

namespace catapult { namespace model {

	/// Information about a cosigned transaction.
	/// \note In order to allow efficient aggregation, only some information may be specified.
	struct CosignedTransactionInfo {
		/// Transaction entity hash.
		Hash256 EntityHash;

		/// Transaction pointer (optional).
		std::shared_ptr<const Transaction> pTransaction;

		/// Cosignatures (optional).
		std::vector<Cosignature> Cosignatures;
	};
}}
