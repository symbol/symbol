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
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"
#include "catapult/utils/Hashers.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include <unordered_map>

namespace catapult { namespace test {

	/// Creates an aggregate transaction with \a numCosignatures cosignatures.
	std::unique_ptr<model::AggregateTransaction> CreateRandomAggregateTransactionWithCosignatures(uint32_t numCosignatures);

	/// Generates a random cosignature for parent hash (\a aggregateHash).
	model::DetachedCosignature GenerateValidCosignature(const Hash256& aggregateHash);

	/// Fix cosignatures of \a aggregateTransaction having \a aggregateHash.
	void FixCosignatures(const Hash256& aggregateHash, model::AggregateTransaction& aggregateTransaction);

	/// Map of cosignature components.
	using CosignaturesMap = std::unordered_map<Key, Signature, utils::ArrayHasher<Key>>;

	/// Wrapper around an aggregate transaction and its component information.
	struct AggregateTransactionWrapper {
		/// Aggregate transaction.
		std::unique_ptr<model::AggregateTransaction> pTransaction;

		/// Sub transactions composing the aggregate transaction.
		std::vector<const mocks::EmbeddedMockTransaction*> SubTransactions;
	};

	/// Creates an aggregate transaction composed of \a numTransactions sub transactions.
	AggregateTransactionWrapper CreateAggregateTransaction(uint8_t numTransactions);

	/// Creates a new transaction based on \a aggregateTransaction that excludes all cosignatures.
	std::unique_ptr<model::Transaction> StripCosignatures(const model::AggregateTransaction& aggregateTransaction);

	/// Extracts component parts of \a cosignatures into a map.
	CosignaturesMap ToMap(const std::vector<model::Cosignature>& cosignatures);

	/// Asserts that \a stitchedTransaction is equal to \a originalTransaction with \a expectedCosignatures.
	/// If nonzero, \a numCosignaturesToIgnore will allow that many cosignatures to be unmatched.
	void AssertStitchedTransaction(
			const model::Transaction& stitchedTransaction,
			const model::AggregateTransaction& originalTransaction,
			const std::vector<model::Cosignature>& expectedCosignatures,
			size_t numCosignaturesToIgnore = 0);
}}
