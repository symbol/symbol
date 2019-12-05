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

#include "catapult/cache/CacheChangesStorageAdapter.h"
#include "tests/catapult/cache/test/DeltasAwareCache.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CacheChangesStorageAdapterTests

	using StorageAdapter = CacheChangesStorageAdapter<test::DeltasAwareCache<0>, test::DeltasAwareCacheStorageTraits>;

	// region id

	namespace {
		template<size_t CacheId>
		size_t GetCacheId() {
			// Arrange:
			test::ByteVectorCacheDeltas deltas;
			test::DeltasAwareCache<CacheId> cache(deltas);
			CacheChangesStorageAdapter<test::DeltasAwareCache<CacheId>, test::DeltasAwareCacheStorageTraits> adapter(cache);

			// Act:
			return adapter.id();
		}
	}

	TEST(TEST_CLASS, AdapterExposesCacheId) {
		EXPECT_EQ(4u, GetCacheId<4>());
		EXPECT_EQ(7u, GetCacheId<7>());
	}

	// endregion

	// region saveAll

	TEST(TEST_CLASS, CanSaveTypedChanges) {
		// Arrange: seed random data
		test::ByteVectorCacheChanges subCacheChanges;
		subCacheChanges.Added.push_back(test::GenerateRandomVector(21));
		subCacheChanges.Removed.push_back(test::GenerateRandomVector(21));
		subCacheChanges.Removed.push_back(test::GenerateRandomVector(14));
		subCacheChanges.Removed.push_back(test::GenerateRandomVector(17));
		subCacheChanges.Copied.push_back(test::GenerateRandomVector(21));
		subCacheChanges.Copied.push_back(test::GenerateRandomVector(14));

		// - create cache changes around single sub cache
		CacheChanges::MemoryCacheChangesContainer cacheChangesContainer;
		cacheChangesContainer.emplace_back(test::CopyByteVectorCacheChanges(subCacheChanges));
		CacheChanges cacheChanges(std::move(cacheChangesContainer));

		// Act:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		test::ByteVectorCacheDeltas deltas;
		test::DeltasAwareCache<0> cache(deltas);
		StorageAdapter adapter(cache);
		adapter.saveAll(cacheChanges, outputStream);

		// Assert:
		ASSERT_EQ(9u * sizeof(uint64_t) + 3 * 21 + 2 * 14 + 17, buffer.size());

		test::ByteVectorBufferReader reader(buffer);
		ASSERT_EQ(1u, reader.read64());
		ASSERT_EQ(3u, reader.read64());
		ASSERT_EQ(2u, reader.read64());

		test::AssertEquivalent(subCacheChanges.Added, reader, "added");
		test::AssertEquivalent(subCacheChanges.Removed, reader, "removed");
		test::AssertEquivalent(subCacheChanges.Copied, reader, "copied");
	}

	// endregion

	// region loadAll

	TEST(TEST_CLASS, CanLoadTypedChanges) {
		// Arrange:
		std::vector<uint8_t> buffer;
		test::ByteVectorBufferWriter writer(buffer);
		writer.write64(1);
		writer.write64(3);
		writer.write64(2);
		auto added1 = writer.writeBuffer(12);
		auto removed1 = writer.writeBuffer(12);
		auto removed2 = writer.writeBuffer(44);
		auto removed3 = writer.writeBuffer(23);
		auto copied1 = writer.writeBuffer(12);
		auto copied2 = writer.writeBuffer(44);

		// Sanity:
		ASSERT_EQ(9u * sizeof(uint64_t) + 3 * 12 + 2 * 44 + 23, buffer.size());

		// Act:
		mocks::MockMemoryStream inputStream(buffer);

		test::ByteVectorCacheDeltas deltas;
		test::DeltasAwareCache<0> cache(deltas);
		StorageAdapter adapter(cache);
		auto pChangesVoid = adapter.loadAll(inputStream);
		const auto& changes = static_cast<const test::ByteVectorCacheChanges&>(*pChangesVoid);

		// Assert:
		EXPECT_EQ(1u, changes.Added.size());
		EXPECT_EQ(3u, changes.Removed.size());
		EXPECT_EQ(2u, changes.Copied.size());

		EXPECT_EQ(std::vector<std::vector<uint8_t>>({ added1 }), changes.Added);
		EXPECT_EQ(std::vector<std::vector<uint8_t>>({ removed1, removed2, removed3 }), changes.Removed);
		EXPECT_EQ(std::vector<std::vector<uint8_t>>({ copied1, copied2 }), changes.Copied);
	}

	// endregion

	// region apply

	namespace {
		void AssertApplied(
				const std::set<std::vector<uint8_t>>& expectedBuffersSet,
				const test::BasicDeltasAwareCache::Breadcrumbs& breadcrumbs,
				size_t startIndex,
				size_t endIndex,
				const std::vector<test::BasicDeltasAwareCache::OperationType>& expectedOperations) {
			// Sanity:
			EXPECT_EQ(endIndex - startIndex + 1, expectedBuffersSet.size());

			// Assert:
			std::set<std::vector<uint8_t>> appliedBuffersSet;
			for (auto i = startIndex; i <= endIndex; ++i) {
				const auto& breadcrumbPair = breadcrumbs[i];
				appliedBuffersSet.insert(breadcrumbPair.first);

				EXPECT_EQ(expectedOperations, breadcrumbPair.second) << "operations at " << i;
			}

			EXPECT_EQ(expectedBuffersSet, appliedBuffersSet);
		}
	}

	TEST(TEST_CLASS, CanApplyTypedChanges) {
		// Arrange: seed random data
		test::ByteVectorCacheChanges subCacheChanges;
		subCacheChanges.Added.push_back(test::GenerateRandomVector(21));
		subCacheChanges.Removed.push_back(test::GenerateRandomVector(21));
		subCacheChanges.Removed.push_back(test::GenerateRandomVector(14));
		subCacheChanges.Removed.push_back(test::GenerateRandomVector(17));
		subCacheChanges.Copied.push_back(test::GenerateRandomVector(21));
		subCacheChanges.Copied.push_back(test::GenerateRandomVector(14));

		// - create cache changes around single sub cache
		CacheChanges::MemoryCacheChangesContainer cacheChangesContainer;
		cacheChangesContainer.emplace_back(test::CopyByteVectorCacheChanges(subCacheChanges));
		CacheChanges cacheChanges(std::move(cacheChangesContainer));

		// Act:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		test::BasicDeltasAwareCache::Breadcrumbs breadcrumbs;
		test::ByteVectorCacheDeltas deltas;
		test::DeltasAwareCache<0> cache(deltas, breadcrumbs);
		StorageAdapter adapter(cache);
		adapter.apply(cacheChanges);

		// Assert:
		ASSERT_EQ(6u, breadcrumbs.size());

		// - check added and copied (breadcrumbs in [0, 2] should be added and copied)
		{
			std::set<std::vector<uint8_t>> expectedBuffersSet(subCacheChanges.Added.cbegin(), subCacheChanges.Added.cend());
			expectedBuffersSet.insert(subCacheChanges.Copied.cbegin(), subCacheChanges.Copied.cend());
			AssertApplied(expectedBuffersSet, breadcrumbs, 0, 2, {
				test::BasicDeltasAwareCache::OperationType::Purge,
				test::BasicDeltasAwareCache::OperationType::Load_Into
			});
		}

		// - check removed (breadcrumbs in [3, 5] should be removed)
		{
			std::set<std::vector<uint8_t>> expectedBuffersSet(subCacheChanges.Removed.cbegin(), subCacheChanges.Removed.cend());
			AssertApplied(expectedBuffersSet, breadcrumbs, 3, 5, { test::BasicDeltasAwareCache::OperationType::Purge });
		}
	}

	// endregion
}}
