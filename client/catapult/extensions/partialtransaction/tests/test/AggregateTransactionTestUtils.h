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

	/// A map of cosignature components.
	using CosignaturesMap = std::unordered_map<Key, Signature, utils::ArrayHasher<Key>>;

	/// Wrapper around an aggregate transaction and its component information.
	struct AggregateTransactionWrapper {
		/// The aggregate transaction.
		std::unique_ptr<model::AggregateTransaction> pTransaction;

		/// The sub transactions composing the aggregate transaction.
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
