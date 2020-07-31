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

#include "Validators.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"

namespace catapult { namespace validators {

	using Notification = model::AggregateEmbeddedTransactionsNotification;

	namespace {
		RawBuffer ExtractDataBuffer(const model::EmbeddedTransaction& transaction) {
			auto headerSize = model::EmbeddedTransaction::Header_Size;
			return { reinterpret_cast<const uint8_t*>(&transaction) + headerSize, transaction.Size - headerSize };
		}

		Hash256 CalculateExpectedTransactionsHash(const Notification& notification) {
			crypto::MerkleHashBuilder transactionsHashBuilder(notification.TransactionsCount);

			const auto* pTransaction = notification.TransactionsPtr;
			for (auto i = 0u; i < notification.TransactionsCount; ++i) {
				Hash256 transactionHash;
				crypto::Sha3_256(ExtractDataBuffer(*pTransaction), transactionHash);
				transactionsHashBuilder.update(transactionHash);

				pTransaction = model::AdvanceNext(pTransaction);
			}

			Hash256 transactionsHash;
			transactionsHashBuilder.final(transactionsHash);
			return transactionsHash;
		}
	}

	DEFINE_STATELESS_VALIDATOR(AggregateTransactionsHash, [](const Notification& notification) {
		auto expectedTransactionsHash = CalculateExpectedTransactionsHash(notification);
		return expectedTransactionsHash == notification.TransactionsHash
				? ValidationResult::Success
				: Failure_Aggregate_Transactions_Hash_Mismatch;
	})
}}
