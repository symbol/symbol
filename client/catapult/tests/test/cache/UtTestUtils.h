#pragma once
#include "catapult/model/EntityInfo.h"
#include "catapult/model/EntityRange.h"
#include "catapult/utils/ShortHash.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include <vector>

namespace catapult {
	namespace cache {
		class MemoryUtCache;
		class MemoryUtCacheView;
		class UtCache;
	}
}

namespace catapult { namespace test {

	/// Adds all \a infos to \a cache.
	void AddAll(cache::UtCache& cache, const std::vector<model::TransactionInfo>& infos);

	/// Removes all infos identified by the hashes in \a hashes from \a cache.
	void RemoveAll(cache::UtCache& cache, const std::vector<Hash256>& hashes);

	/// Extracts the deadlines of all transactions stored in \a cache.
	std::vector<Timestamp::ValueType> ExtractRawDeadlines(const cache::MemoryUtCache& cache);

	/// Extracts copies of the first \a count transactions stored in \a cache.
	std::vector<std::unique_ptr<const model::Transaction>> ExtractTransactions(const cache::MemoryUtCache& cache, size_t count);

	/// Extracts pointers to the first \a count transaction infos stored in \a view.
	std::vector<const model::TransactionInfo*> ExtractTransactionInfos(const cache::MemoryUtCacheView& view, size_t count);

	/// Asserts that the deadlines of all transactions stored in \a cache are equal to \a expectedDeadlines.
	void AssertDeadlines(const cache::MemoryUtCache& cache, const std::vector<Timestamp::ValueType>& expectedDeadlines);

	/// Asserts that \a cache contains all of the hashes in \a hashes.
	void AssertContainsAll(const cache::MemoryUtCache& cache, const std::vector<Hash256>& hashes);

	/// Asserts that \a cache contains none of the hashes in \a hashes.
	void AssertContainsNone(const cache::MemoryUtCache& cache, const std::vector<Hash256>& hashes);

	/// Asserts that \a cache contains all of the infos in \a infos.
	void AssertContainsAll(const cache::MemoryUtCache& cache, const std::vector<model::TransactionInfo>& infos);

	/// Asserts that \a cache contains none of the infos in \a infos.
	void AssertContainsNone(const cache::MemoryUtCache& cache, const std::vector<model::TransactionInfo>& infos);
}}
