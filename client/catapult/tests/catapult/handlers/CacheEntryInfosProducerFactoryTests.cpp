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

#include "catapult/handlers/CacheEntryInfosProducerFactory.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/TestHarness.h"
#include <numeric>
#include <unordered_map>

namespace catapult { namespace handlers {

#define TEST_CLASS CacheEntryInfosProducerFactoryTests

	namespace {
		constexpr auto Max_Data_Size = model::CacheEntryInfo<uint64_t>::Max_Data_Size;

		using CacheEntryInfos = std::vector<std::shared_ptr<const model::CacheEntryInfo<uint64_t>>>;
		using CacheDataMap = std::unordered_map<uint64_t, std::string>;

		class MockCacheView {
		public:
			using KeyType = uint64_t;
			using ValueType = std::string;

		public:
			explicit MockCacheView(const CacheDataMap& map) : m_map(map)
			{}

		public:
			auto find(KeyType key) const {
				auto iter = m_map.find(key);
				return m_map.cend() == iter ? MockCacheFindIterator() : MockCacheFindIterator(iter);
			}

		public:
			class MockCacheFindIterator {
			public:
				MockCacheFindIterator() : m_isValid(false)
				{}

				explicit MockCacheFindIterator(CacheDataMap::const_iterator iter)
						: m_isValid(true)
						, m_iter(iter)
				{}

			public:
				const ValueType* tryGetUnadapted() {
					return m_isValid ? &m_iter->second : nullptr;
				}

			private:
				bool m_isValid;
				CacheDataMap::const_iterator m_iter;
			};

		private:
			const CacheDataMap& m_map;
		};

		class MockCache {
		public:
			using KeyType = typename MockCacheView::KeyType;
			using ValueType = typename MockCacheView::ValueType;

		public:
			void insert(KeyType key) {
				m_map.emplace(key, ValueType(key, 'a'));
			}

		public:
			cache::LockedCacheView<MockCacheView> createView() const {
				auto readLock = m_lock.acquireReader();
				return cache::LockedCacheView<MockCacheView>(MockCacheView(m_map), std::move(readLock));
			}

		private:
			CacheDataMap m_map;
			mutable utils::SpinReaderWriterLock m_lock;
		};

		class MockCacheSerializer {
		public:
			using KeyType = typename MockCache::KeyType;
			using ValueType = typename MockCache::ValueType;

		public:
			static std::string SerializeValue(const ValueType& value) {
				return value;
			}

			static std::string DeserializeValue(const RawBuffer& buffer) {
				return std::string(reinterpret_cast<const char*>(buffer.pData), buffer.Size);
			}
		};

		struct MockCacheDescriptor {
		public:
			using KeyType = MockCache::KeyType;
			using ValueType = MockCache::ValueType;

			using CacheType = MockCache;
			using CacheViewType = MockCacheView;

			struct PatriciaTree {
				using Serializer = MockCacheSerializer;
			};
		};

		auto PrepareCache() {
			auto pCache = std::make_unique<MockCache>();
			for (auto i = 1u; i <= 10; ++i)
				pCache->insert(i);

			return pCache;
		}

		void AssertEqual(const std::vector<uint64_t>& expectedIds, const CacheEntryInfos& actualInfos) {
			// Assert:
			ASSERT_EQ(expectedIds.size(), actualInfos.size());

			auto i = 0u;
			for (auto id : expectedIds) {
				auto pInfo = actualInfos[i];
				auto expectedValue = std::string(id, 'a');

				auto message = "info at " + std::to_string(i);
				EXPECT_EQ(id, pInfo->Id) << message;
				EXPECT_EQ(expectedValue.size() + sizeof(model::CacheEntryInfo<uint64_t>), pInfo->Size) << message;

				ASSERT_EQ(expectedValue.size(), pInfo->DataSize) << "DataSize at " << i;

				auto value = MockCacheSerializer::DeserializeValue({ pInfo->DataPtr(), pInfo->DataSize });
				EXPECT_EQ(expectedValue, value) << message;

				++i;
			}
		}

		struct MockCacheEntryInfosContext {
		private:
			using MockCacheEntryInfosProducerFactory = CacheEntryInfosProducerFactory<MockCacheDescriptor>;

		public:
			MockCacheEntryInfosContext()
					: pCache(PrepareCache())
					, ProducerFactory(CacheEntryInfosProducerFactory<MockCacheDescriptor>::Create(*pCache))
			{}

			std::unique_ptr<MockCache> pCache;
			MockCacheEntryInfosProducerFactory ProducerFactory;
		};

		void AssertCanSupplyCacheEntryInfos(const std::vector<uint64_t>& ids) {
			// Arrange:
			MockCacheEntryInfosContext context;
			auto idRange = model::EntityRange<uint64_t>::CopyFixed(reinterpret_cast<const uint8_t*>(ids.data()), ids.size());

			// Act:
			auto producer = context.ProducerFactory(idRange);

			// Assert:
			auto infos = test::ProduceAll(producer);
			AssertEqual(ids, infos);
		}
	}

	// region basic

	TEST(TEST_CLASS, CanSupplySingleValue) {
		for (auto i = 1u; i <= 10; ++i)
			AssertCanSupplyCacheEntryInfos({ i });
	}

	TEST(TEST_CLASS, CanSupplyMultipleValues) {
		AssertCanSupplyCacheEntryInfos({ 1, 3, 5 });
		AssertCanSupplyCacheEntryInfos({ 2, 4 });
	}

	TEST(TEST_CLASS, CanSupplyAllValues) {
		auto ids = std::vector<MosaicId::ValueType>(10);
		std::iota(ids.begin(), ids.end(), 1);
		AssertCanSupplyCacheEntryInfos(ids);
	}

	// endregion

	// region unknown value

	TEST(TEST_CLASS, ReturnsInfoForUnkownMosaic) {
		// Arrange: request an unknown value to be supplied
		MockCacheEntryInfosContext context;
		uint64_t key = 11;
		auto idRange = model::EntityRange<uint64_t>::CopyFixed(reinterpret_cast<const uint8_t*>(&key), 1);

		// Act:
		auto producer = context.ProducerFactory(idRange);
		auto infos = test::ProduceAll(producer);

		// Assert:
		ASSERT_EQ(1u, infos.size());

		const auto& pInfo = infos[0];
		EXPECT_EQ(11u, pInfo->Id);
		EXPECT_EQ(0u, pInfo->DataSize);
	}

	// endregion

	// region overflow check

	TEST(TEST_CLASS, ReturnsMaxDataSizeWhenSerializedEntryIsTooLarge) {
		// Arrange:
		MockCacheEntryInfosContext context;
		uint64_t key = Max_Data_Size + 1;
		context.pCache->insert(key);
		auto idRange = model::EntityRange<uint64_t>::CopyFixed(reinterpret_cast<const uint8_t*>(&key), 1);

		// Act:
		auto producer = context.ProducerFactory(idRange);
		auto infos = test::ProduceAll(producer);

		// Assert:
		ASSERT_EQ(1u, infos.size());

		const auto& pInfo = infos[0];
		EXPECT_EQ(key, pInfo->Id);
		EXPECT_EQ(Max_Data_Size + sizeof(model::CacheEntryInfo<uint64_t>), pInfo->Size);
		EXPECT_EQ(Max_Data_Size, pInfo->DataSize);
	}

	// endregion
}}
