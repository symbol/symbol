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

#include "TransactionsInfo.h"
#include "catapult/cache/MemoryUtCache.h"

namespace catapult { namespace harvesting {

	TransactionsInfoSupplier CreateTransactionsInfoSupplier(const cache::MemoryUtCache& utCache) {
		return [&utCache](auto count) {
			TransactionsInfo info;
			std::vector<const model::TransactionInfo*> transactionInfos;

			auto view = utCache.view();
			if (0 != count) {
				view.forEach([count, &info, &transactionInfos](const auto& transactionInfo) {
					info.Transactions.push_back(transactionInfo.pEntity);
					transactionInfos.push_back(&transactionInfo);
					return info.Transactions.size() != count;
				});
			}

			CalculateBlockTransactionsHash(transactionInfos, info.TransactionsHash);
			return info;
		};
	}
}}
