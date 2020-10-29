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

#include "AggregateTransaction.h"

namespace catapult { namespace model {

	size_t GetTransactionPayloadSize(const AggregateTransactionHeader& header) {
		return header.PayloadSize;
	}

	namespace {
		constexpr bool IsPayloadSizeValid(const AggregateTransaction& aggregate) {
			return
					aggregate.Size >= sizeof(AggregateTransaction) &&
					0 == aggregate.PayloadSize % 8 && // require 8-byte alignment of Cosignatures
					aggregate.Size - sizeof(AggregateTransaction) >= aggregate.PayloadSize &&
					0 == (aggregate.Size - sizeof(AggregateTransaction) - aggregate.PayloadSize) % sizeof(Cosignature);
		}
	}

	bool IsSizeValid(const AggregateTransaction& aggregate, const TransactionRegistry& registry) {
		if (!IsPayloadSizeValid(aggregate)) {
			CATAPULT_LOG(warning) << "aggregate transaction failed size validation with size " << aggregate.Size;
			return false;
		}

		auto transactions = aggregate.Transactions(EntityContainerErrorPolicy::Suppress);
		auto areAllTransactionsValid = std::all_of(transactions.cbegin(), transactions.cend(), [&registry](const auto& transaction) {
			return IsSizeValid(transaction, registry);
		});

		if (areAllTransactionsValid && !transactions.hasError())
			return true;

		CATAPULT_LOG(warning)
				<< "aggregate transactions failed size validation (valid sizes? " << areAllTransactionsValid
				<< ", errors? " << transactions.hasError() << ")";
		return false;
	}
}}
