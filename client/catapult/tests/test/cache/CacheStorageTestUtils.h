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

#pragma once
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/nodeps/Random.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region ContainsOnlyCacheStorageTests

	/// Tests for cache storages for caches that support containment but not lookup operations.
	template<typename TTraits>
	class ContainsOnlyCacheStorageTests {
	private:
		using ValueType = typename TTraits::ValueType;

	public:
		static void AssertCanSaveValue() {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			// - create a random value
			auto originalValue = TTraits::CreateValue(123);

			// Act:
			TTraits::StorageType::Save(originalValue, stream);

			// Assert: the value was saved
			ASSERT_EQ(TTraits::Value_Size, buffer.size());
			const auto& savedValue = reinterpret_cast<const ValueType&>(buffer[0]);
			EXPECT_EQ(originalValue, savedValue);

			// - the stream was not flushed
			EXPECT_EQ(0u, stream.numFlushes());
		}

	private:
		struct LoadTraits {
			static void Load(io::InputStream& inputStream, const ValueType&, ValueType& result) {
				result = TTraits::StorageType::Load(inputStream);
			}
		};

		struct LoadIntoTraits {
			static void Load(io::InputStream& inputStream, const ValueType& originalValue, ValueType& result) {
				// Act:
				typename TTraits::CacheType cache;
				{
					auto delta = cache.createDelta();
					TTraits::StorageType::LoadInto(TTraits::StorageType::Load(inputStream), *delta);
					cache.commit();
				}

				// Assert:
				auto view = cache.createView();
				EXPECT_EQ(1u, view->size());

				// - cache only supports contains, so return original value iff it is contained
				result = view->contains(originalValue) ? originalValue : ValueType();
			}
		};

		template<typename TLoadTraits>
		static void AssertCanLoadValue() {
			// Arrange:
			auto buffer = test::GenerateRandomVector(TTraits::Value_Size);
			mocks::MockMemoryStream inputStream(buffer);
			const auto& originalValue = reinterpret_cast<const ValueType&>(buffer[0]);

			// Act:
			ValueType result;
			TLoadTraits::Load(inputStream, originalValue, result);

			// Assert:
			EXPECT_EQ(originalValue, result);
		}

	public:
		static void AssertCanLoadValueViaLoad() {
			AssertCanLoadValue<LoadTraits>();
		}

		static void AssertCanLoadValueViaLoadInto() {
			AssertCanLoadValue<LoadIntoTraits>();
		}

	public:
		static void AssertCanPurgeExistingValueFromCache() {
			// Arrange:
			auto originalValue = TTraits::CreateValue(112);
			auto otherValue = TTraits::CreateValue(111);

			// - add two values one of which will be purged
			typename TTraits::CacheType cache;
			{
				auto delta = cache.createDelta();
				delta->insert(otherValue);
				delta->insert(originalValue);
				cache.commit();
			}

			// Sanity:
			EXPECT_TRUE(cache.createView()->contains(originalValue));
			EXPECT_TRUE(cache.createView()->contains(otherValue));

			// Act:
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::Purge(originalValue, *delta);
				cache.commit();
			}

			// Assert:
			EXPECT_FALSE(cache.createView()->contains(originalValue));
			EXPECT_TRUE(cache.createView()->contains(otherValue));
		}

		static void AssertCanPurgeNonexistentValueFromCache() {
			// Arrange:
			auto originalValue = TTraits::CreateValue(112);
			auto otherValue = TTraits::CreateValue(111);

			// - add one value that will not be purged
			typename TTraits::CacheType cache;
			{
				auto delta = cache.createDelta();
				delta->insert(otherValue);
				cache.commit();
			}

			// Sanity:
			EXPECT_FALSE(cache.createView()->contains(originalValue));
			EXPECT_TRUE(cache.createView()->contains(otherValue));

			// Act:
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::Purge(originalValue, *delta);
				cache.commit();
			}

			// Assert:
			EXPECT_FALSE(cache.createView()->contains(originalValue));
			EXPECT_TRUE(cache.createView()->contains(otherValue));
		}
	};

#define MAKE_CONTAINS_ONLY_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::ContainsOnlyCacheStorageTests<TRAITS>::Assert##TEST_NAME(); }

#define DEFINE_CONTAINS_ONLY_CACHE_STORAGE_TESTS(TEST_CLASS, TRAITS) \
	MAKE_CONTAINS_ONLY_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, CanSaveValue) \
	MAKE_CONTAINS_ONLY_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, CanLoadValueViaLoad) \
	MAKE_CONTAINS_ONLY_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, CanLoadValueViaLoadInto) \
	MAKE_CONTAINS_ONLY_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, CanPurgeExistingValueFromCache) \
	MAKE_CONTAINS_ONLY_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, CanPurgeNonexistentValueFromCache)

	// endregion

	// region BasicInsertRemoveCacheStorageTests

	/// Tests for cache storages for caches that support basic insert remove operations.
	template<typename TTraits>
	class BasicInsertRemoveCacheStorageTests {
	public:
		static void AssertCanLoadValueIntoCache() {
			// Arrange:
			auto originalValue = TTraits::CreateValue(TTraits::CreateId(148));

			// Act:
			typename TTraits::CacheType cache;
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::LoadInto(originalValue, *delta);
				cache.commit();
			}

			// Assert: the cache contains the value
			auto view = cache.createView();
			EXPECT_EQ(1u, view->size());
			ASSERT_TRUE(view->contains(TTraits::CreateId(148)));

			// - the loaded cache value is correct
			auto valueIter = view->find(TTraits::CreateId(148));
			TTraits::AssertEqual(originalValue, valueIter.get());
		}

		static void AssertCanPurgeExistingValueFromCache() {
			// Arrange:
			auto originalValue = TTraits::CreateValue(TTraits::CreateId(148));
			auto otherValue = TTraits::CreateValue(TTraits::CreateId(200));

			typename TTraits::CacheType cache;
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::LoadInto(originalValue, *delta);
				TTraits::StorageType::LoadInto(otherValue, *delta);
				cache.commit();
			}

			// Sanity:
			EXPECT_TRUE(cache.createView()->contains(TTraits::CreateId(148)));
			EXPECT_TRUE(cache.createView()->contains(TTraits::CreateId(200)));

			// Act:
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::Purge(originalValue, *delta);
				cache.commit();
			}

			// Assert:
			EXPECT_FALSE(cache.createView()->contains(TTraits::CreateId(148)));
			EXPECT_TRUE(cache.createView()->contains(TTraits::CreateId(200)));
		}

		static void AssertCanPurgeNonexistentValueFromCache() {
			// Arrange:
			auto originalValue = TTraits::CreateValue(TTraits::CreateId(148));
			auto otherValue = TTraits::CreateValue(TTraits::CreateId(200));

			// - add one value that will not be purged
			typename TTraits::CacheType cache;
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::LoadInto(otherValue, *delta);
				cache.commit();
			}

			// Sanity:
			EXPECT_FALSE(cache.createView()->contains(TTraits::CreateId(148)));
			EXPECT_TRUE(cache.createView()->contains(TTraits::CreateId(200)));

			// Act:
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::Purge(originalValue, *delta);
				cache.commit();
			}

			// Assert:
			EXPECT_FALSE(cache.createView()->contains(TTraits::CreateId(148)));
			EXPECT_TRUE(cache.createView()->contains(TTraits::CreateId(200)));
		}
	};

#define MAKE_BASIC_INSERT_REMOVE_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::BasicInsertRemoveCacheStorageTests<TRAITS>::Assert##TEST_NAME(); }

#define DEFINE_BASIC_INSERT_REMOVE_CACHE_STORAGE_TESTS(TEST_CLASS, TRAITS) \
	MAKE_BASIC_INSERT_REMOVE_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, CanLoadValueIntoCache) \
	MAKE_BASIC_INSERT_REMOVE_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, CanPurgeExistingValueFromCache) \
	MAKE_BASIC_INSERT_REMOVE_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, CanPurgeNonexistentValueFromCache)

	// endregion
}}
