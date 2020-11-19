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
#include "catapult/model/EntityInfo.h"
#include "catapult/model/EntityRange.h"
#include "catapult/utils/ShortHash.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include <vector>

namespace catapult {
	namespace cache {
		class MemoryUtCache;
		class MemoryUtCacheProxy;
		class MemoryUtCacheView;
		class UtCache;
	}
}

namespace catapult { namespace test {

	/// Creates a memory ut cache seeded with \a count transactions.
	std::unique_ptr<cache::MemoryUtCache> CreateSeededMemoryUtCache(uint32_t count);

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

	/// Asserts that \a cacheProxy contains all of the hashes in \a hashes.
	void AssertContainsAll(const cache::MemoryUtCacheProxy& cacheProxy, const std::vector<Hash256>& hashes);

	/// Asserts that \a cache contains all of the hashes in \a hashes.
	void AssertContainsAll(const cache::MemoryUtCache& cache, const std::vector<Hash256>& hashes);

	/// Asserts that \a cache contains none of the hashes in \a hashes.
	void AssertContainsNone(const cache::MemoryUtCache& cache, const std::vector<Hash256>& hashes);

	/// Asserts that \a cacheView contains all of the hashes in \a hashes.
	void AssertContainsAll(const cache::MemoryUtCacheView& cacheView, const std::vector<Hash256>& hashes);

	/// Asserts that \a cacheView contains none of the hashes in \a hashes.
	void AssertContainsNone(const cache::MemoryUtCacheView& cacheView, const std::vector<Hash256>& hashes);

	/// Asserts that \a cache contains all of the infos in \a infos.
	void AssertContainsAll(const cache::MemoryUtCache& cache, const std::vector<model::TransactionInfo>& infos);

	/// Asserts that \a cache contains none of the infos in \a infos.
	void AssertContainsNone(const cache::MemoryUtCache& cache, const std::vector<model::TransactionInfo>& infos);
}}
