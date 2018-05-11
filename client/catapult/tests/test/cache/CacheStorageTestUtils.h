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
			mocks::MockMemoryStream stream("", buffer);

			// - create a random value
			auto originalValue = TTraits::CreateRandomValue();

			// Act:
			TTraits::StorageType::Save(originalValue, stream);

			// Assert: the value was saved
			ASSERT_EQ(static_cast<uint64_t>(TTraits::Value_Size), buffer.size());
			const auto& savedValue = reinterpret_cast<const ValueType&>(*buffer.data());
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
				auto delta = cache.createDelta();
				TTraits::StorageType::LoadInto(inputStream, *delta);
				cache.commit();

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
			std::vector<uint8_t> buffer(TTraits::Value_Size);
			test::FillWithRandomData(buffer);
			mocks::MockMemoryStream inputStream("", buffer);
			const auto& originalValue = reinterpret_cast<const ValueType&>(*buffer.data());

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
	};

#define MAKE_CONTAINS_ONLY_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::ContainsOnlyCacheStorageTests<TRAITS>::Assert##TEST_NAME(); }

#define DEFINE_CONTAINS_ONLY_CACHE_STORAGE_TESTS(TEST_CLASS, TRAITS) \
	MAKE_CONTAINS_ONLY_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, CanSaveValue) \
	MAKE_CONTAINS_ONLY_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, CanLoadValueViaLoad) \
	MAKE_CONTAINS_ONLY_CACHE_STORAGE_TEST(TEST_CLASS, TRAITS, CanLoadValueViaLoadInto)

	// endregion

	// region LookupCacheStorageTests

	/// Tests for cache storages for caches that support lookup operations.
	template<typename TTraits>
	class LookupCacheStorageTests {
	private:
		using KeyType = typename TTraits::KeyType;
		using ValueType = typename TTraits::ValueType;

	private:
		struct LoadTraits {
			static void Load(io::InputStream& inputStream, const KeyType&, ValueType& result) {
				result = TTraits::StorageType::Load(inputStream);
			}
		};

		struct LoadIntoTraits {
			static void Load(io::InputStream& inputStream, const KeyType& expectedKey, ValueType& result) {
				// Act:
				typename TTraits::CacheType cache;
				auto delta = cache.createDelta();
				TTraits::StorageType::LoadInto(inputStream, *delta);
				cache.commit();

				// Assert:
				auto view = cache.createView();
				EXPECT_EQ(1u, view->size());
				ASSERT_TRUE(view->contains(expectedKey));
				result = view->get(expectedKey);
			}
		};

		template<typename TLoadTraits>
		static void RunLoadValueTest(const KeyType& key, std::vector<uint8_t>& buffer, ValueType& result) {
			// Arrange:
			mocks::MockMemoryStream inputStream("", buffer);

			// Act:
			TLoadTraits::Load(inputStream, key, result);

			// Assert: whole buffer has been read
			EXPECT_EQ(buffer.size(), inputStream.position());
		}

	public:
		static void RunLoadValueViaLoadTest(const KeyType& key, std::vector<uint8_t>& buffer, ValueType& result) {
			RunLoadValueTest<LoadTraits>(key, buffer, result);
		}

		static void RunLoadValueViaLoadIntoTest(const KeyType& key, std::vector<uint8_t>& buffer, ValueType& result) {
			RunLoadValueTest<LoadIntoTraits>(key, buffer, result);
		}
	};

	// endregion
}}
