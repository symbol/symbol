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
		bool TryCalculateRealSize(const Transaction& transaction, const TransactionRegistry& registry, uint64_t& realSize) {
			const auto* pPlugin = registry.findPlugin(transaction.Type);
			if (!pPlugin || !pPlugin->supportsTopLevel()) {
				CATAPULT_LOG(warning) << "rejected transaction with type: " << transaction.Type;
				return false;
			}

			realSize = pPlugin->calculateRealSize(transaction);
			return true;
		}
	}

	bool IsSizeValid(const Transaction& transaction, const TransactionRegistry& registry) {
		uint64_t realSize;
		if (!TryCalculateRealSize(transaction, registry, realSize))
			return false;

		if (transaction.Size == realSize)
			return true;

		CATAPULT_LOG(warning)
				<< transaction.Type << " transaction failed size validation with size " << transaction.Size
				<< " (expected " << realSize << ")";
		return false;
	}
}}
