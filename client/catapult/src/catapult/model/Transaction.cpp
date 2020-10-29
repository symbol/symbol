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

#include "Transaction.h"
#include "TransactionPlugin.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace model {

	namespace {
		bool IsSizeValidInternal(const Transaction& transaction, const TransactionRegistry& registry) {
			const auto* pPlugin = registry.findPlugin(transaction.Type);
			if (!pPlugin || !pPlugin->supportsTopLevel()) {
				CATAPULT_LOG(warning)
						<< "rejected transaction with type: " << transaction.Type
						<< (pPlugin ? " (top level not supported)" : "");
				return false;
			}

			return pPlugin->isSizeValid(transaction);
		}
	}

	bool IsSizeValid(const Transaction& transaction, const TransactionRegistry& registry) {
		if (transaction.Size < sizeof(Transaction)) {
			CATAPULT_LOG(warning) << "transaction failed size validation with size " << transaction.Size;
			return false;
		}

		if (IsSizeValidInternal(transaction, registry))
			return true;

		CATAPULT_LOG(warning) << transaction.Type << " transaction failed size validation with size " << transaction.Size;
		return false;
	}
}}
