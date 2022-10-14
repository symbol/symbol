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

#include "Validators.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/InvalidMerkleHashBuilder.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/utils/IntegerMath.h"

namespace catapult { namespace validators {

	using Notification = model::AggregateEmbeddedTransactionsNotification;

	namespace {
		using TransactionHasher = consumer<const model::EmbeddedTransaction&, Hash256&>;

		RawBuffer ExtractDataBuffer(const model::EmbeddedTransaction& transaction) {
			return { reinterpret_cast<const uint8_t*>(&transaction), transaction.Size };
		}

		void HashEmbeddedTransaction(const model::EmbeddedTransaction& transaction, Hash256& transactionHash) {
			crypto::Sha3_256(ExtractDataBuffer(transaction), transactionHash);
		}

		void HashEmbeddedTransactionWithPadding(const model::EmbeddedTransaction& transaction, Hash256& transactionHash) {
			crypto::Sha3_256_Builder builder;
			builder.update(ExtractDataBuffer(transaction));
			builder.update(std::vector<uint8_t>(utils::GetPaddingSize(transaction.Size, 8)));
			builder.final(transactionHash);
		}

		template<typename TMerkleHashBuilder>
		Hash256 CalculateExpectedTransactionsHash(const Notification& notification, const TransactionHasher& transactionHasher) {
			TMerkleHashBuilder transactionsHashBuilder(notification.TransactionsCount);

			const auto* pTransaction = notification.TransactionsPtr;
			for (auto i = 0u; i < notification.TransactionsCount; ++i) {
				Hash256 transactionHash;
				transactionHasher(*pTransaction, transactionHash);
				transactionsHashBuilder.update(transactionHash);

				pTransaction = model::AdvanceNext(pTransaction);
			}

			Hash256 transactionsHash;
			transactionsHashBuilder.final(transactionsHash);
			return transactionsHash;
		}

		template<typename TMerkleHashBuilder>
		bool CheckTransactionsHash(const Notification& notification, const TransactionHasher& transactionHasher) {
			auto expectedTransactionsHash = CalculateExpectedTransactionsHash<TMerkleHashBuilder>(notification, transactionHasher);
			return expectedTransactionsHash == notification.TransactionsHash;
		}
	}

	DECLARE_STATELESS_VALIDATOR(AggregateTransactionsHash, Notification)(
			const std::unordered_map<Hash256, Hash256, utils::ArrayHasher<Hash256>>& knownCorruptedHashes) {
		return MAKE_STATELESS_VALIDATOR(AggregateTransactionsHash, ([knownCorruptedHashes](const Notification& notification) {
			// calculate the expected transactions hash correctly
			if (CheckTransactionsHash<crypto::MerkleHashBuilder>(notification, HashEmbeddedTransaction))
				return ValidationResult::Success;

			// if this is a newer aggregate, only the correct calculation is allowed
			static constexpr auto Failure_Result = Failure_Aggregate_Transactions_Hash_Mismatch;
			if (notification.AggregateVersion > 1)
				return Failure_Result;

			// check other combinations of known (mis) transactions hash calculations
			auto anyTransactionHashMatches =
					CheckTransactionsHash<crypto::MerkleHashBuilder>(notification, HashEmbeddedTransactionWithPadding)
					|| CheckTransactionsHash<crypto::InvalidMerkleHashBuilder>(notification, HashEmbeddedTransaction)
					|| CheckTransactionsHash<crypto::InvalidMerkleHashBuilder>(notification, HashEmbeddedTransactionWithPadding);
			if (anyTransactionHashMatches)
				return ValidationResult::Success;

			// finally, check if any known corrupted hash matches
			auto knownCorruptedHashesIter = knownCorruptedHashes.find(notification.AggregateTransactionHash);
			if (knownCorruptedHashes.cend() != knownCorruptedHashesIter) {
				if (notification.TransactionsHash == knownCorruptedHashesIter->second) {
					CATAPULT_LOG(debug)
							<< "detected aggregate " << knownCorruptedHashesIter->first
							<< " with corrupted hash " << knownCorruptedHashesIter->second;
					return ValidationResult::Success;
				}
			}

			// nothing matched, even reject for V1!
			return Failure_Result;
		}));
	}
}}
