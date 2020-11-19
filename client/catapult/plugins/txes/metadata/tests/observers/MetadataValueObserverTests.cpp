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

#include "src/observers/Observers.h"
#include "tests/test/MetadataCacheTestUtils.h"
#include "tests/test/MetadataTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS MetadataValueObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(MetadataValue,)

	// region test utils

	namespace {
		using ObserverTestContext = test::ObserverTestContextT<test::MetadataCacheFactory>;

		struct ValueSizes {
			int16_t Delta;
			uint16_t New;
		};

		struct ModificationTestSeed {
			ValueSizes Sizes;
			std::vector<uint8_t> CacheValue;
			std::vector<uint8_t> NotificationValue;
			std::vector<uint8_t> ExpectedValue;
		};

		auto CreateNotification(const state::MetadataKey& metadataKey, const ValueSizes& valueSizes, const uint8_t* pValue) {
			return test::CreateMetadataValueNotification(metadataKey, valueSizes.Delta, valueSizes.New, pValue);
		}
	}

	// endregion

	// region add cache entry

	namespace {
		void AssertMetadataEntryAddedToCache(NotifyMode notifyMode, const ModificationTestSeed& testSeed) {
			// Arrange:
			ObserverTestContext context(notifyMode);
			const auto& metadataCache = context.cache().sub<cache::MetadataCache>();

			auto pObserver = CreateMetadataValueObserver();

			auto metadataKey = test::GenerateRandomMetadataKey();
			auto notification = CreateNotification(metadataKey, testSeed.Sizes, testSeed.NotificationValue.data());

			// Sanity:
			EXPECT_EQ(0u, metadataCache.size());

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			EXPECT_EQ(1u, metadataCache.size());

			auto metadataEntryIter = metadataCache.find(metadataKey.uniqueKey());
			ASSERT_TRUE(!!metadataEntryIter.tryGet());

			const auto& metadataValue = metadataEntryIter.get().value();
			ASSERT_EQ(testSeed.ExpectedValue.size(), metadataValue.size());
			EXPECT_EQ_MEMORY(testSeed.ExpectedValue.data(), metadataValue.data(), testSeed.ExpectedValue.size());
		}

		void AssertMetadataEntryRemovedFromCache(NotifyMode notifyMode, const ModificationTestSeed& testSeed) {
			// Arrange:
			ObserverTestContext context(notifyMode);
			auto& metadataCache = context.cache().sub<cache::MetadataCache>();

			auto pObserver = CreateMetadataValueObserver();

			auto metadataKey = test::GenerateRandomMetadataKey();
			auto notification = CreateNotification(metadataKey, testSeed.Sizes, testSeed.NotificationValue.data());

			{
				state::MetadataEntry metadataEntry(metadataKey);
				metadataEntry.value().update(testSeed.CacheValue);
				metadataCache.insert(metadataEntry);
			}

			// Sanity:
			EXPECT_EQ(1u, metadataCache.size());

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			EXPECT_EQ(0u, metadataCache.size());
		}
	}

	TEST(TEST_CLASS, CanAddMetadataEntry_Commit) {
		AssertMetadataEntryAddedToCache(NotifyMode::Commit, {
			{ 5, 5 },
			{},
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B }
		});
	}

	TEST(TEST_CLASS, CanAddMetadataEntry_Rollback) {
		AssertMetadataEntryRemovedFromCache(NotifyMode::Rollback, {
			{ 5, 5 },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B },
			{}
		});
	}

	TEST(TEST_CLASS, CanRemoveMetadataEntry_Commit) {
		AssertMetadataEntryRemovedFromCache(NotifyMode::Commit, {
			{ -5, 5 },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B },
			{}
		});
	}

	TEST(TEST_CLASS, CanRemoveMetadataEntry_Rollback) {
		AssertMetadataEntryAddedToCache(NotifyMode::Rollback, {
			{ -5, 5 },
			{},
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B }
		});
	}

	// endregion

	// region change cache entry value

	namespace {
		void AssertValueModification(NotifyMode notifyMode, const ModificationTestSeed& testSeed) {
			// Arrange:
			ObserverTestContext context(notifyMode);
			auto& metadataCache = context.cache().sub<cache::MetadataCache>();

			auto pObserver = CreateMetadataValueObserver();

			auto metadataKey = test::GenerateRandomMetadataKey();
			auto notification = CreateNotification(metadataKey, testSeed.Sizes, testSeed.NotificationValue.data());

			{
				state::MetadataEntry metadataEntry(metadataKey);
				metadataEntry.value().update(testSeed.CacheValue);
				metadataCache.insert(metadataEntry);
			}

			// Sanity:
			EXPECT_EQ(1u, metadataCache.size());

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			EXPECT_EQ(1u, metadataCache.size());

			auto metadataEntryIter = metadataCache.find(metadataKey.uniqueKey());
			ASSERT_TRUE(!!metadataEntryIter.tryGet());

			const auto& metadataValue = metadataEntryIter.get().value();
			ASSERT_EQ(testSeed.ExpectedValue.size(), metadataValue.size());
			EXPECT_EQ_MEMORY(testSeed.ExpectedValue.data(), metadataValue.data(), testSeed.ExpectedValue.size());
		}
	}

	TEST(TEST_CLASS, CanModifyMetadataValueSameLength_Commit) {
		AssertValueModification(NotifyMode::Commit, {
			{ 0, 5 },
			{ 0xE0, 0x77, 0xCC, 0xCC, 0x02 },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B },
			{ 0xE0 ^ 0xBA, 0x77 ^ 0xE8, 0xCC ^ 0xBC, 0xCC ^ 0x4B, 0x02 ^ 0x4B }
		});
	}

	TEST(TEST_CLASS, CanModifyMetadataValueSameLength_Rollback) {
		AssertValueModification(NotifyMode::Rollback, {
			{ 0, 5 },
			{ 0xE0 ^ 0xBA, 0x77 ^ 0xE8, 0xCC ^ 0xBC, 0xCC ^ 0x4B, 0x02 ^ 0x4B },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B },
			{ 0xE0, 0x77, 0xCC, 0xCC, 0x02 }
		});
	}

	TEST(TEST_CLASS, CanModifyMetadataValueShorterLength_Commit) {
		AssertValueModification(NotifyMode::Commit, {
			{ -2, 5 },
			{ 0xE0, 0x77, 0xCC, 0xCC, 0x02 },
			{ 0xBA, 0xE8, 0xBC, 0xCC, 0x02 },
			{ 0xE0 ^ 0xBA, 0x77 ^ 0xE8, 0xCC ^ 0xBC }
		});
	}

	TEST(TEST_CLASS, CanModifyMetadataValueShorterLength_Rollback) {
		AssertValueModification(NotifyMode::Rollback, {
			{ -2, 5 },
			{ 0xE0 ^ 0xBA, 0x77 ^ 0xE8, 0xCC ^ 0xBC },
			{ 0xBA, 0xE8, 0xBC, 0xCC, 0x02 },
			{ 0xE0, 0x77, 0xCC, 0xCC, 0x02 }
		});
	}

	TEST(TEST_CLASS, CanModifyMetadataValueLongerLength_Commit) {
		AssertValueModification(NotifyMode::Commit, {
			{ 2, 7 },
			{ 0xE0, 0x77, 0xCC, 0xCC, 0x02 },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B, 0xEE, 0x1F },
			{ 0xE0 ^ 0xBA, 0x77 ^ 0xE8, 0xCC ^ 0xBC, 0xCC ^ 0x4B, 0x02 ^ 0x4B, 0xEE, 0x1F }
		});
	}

	TEST(TEST_CLASS, CanModifyMetadataValueLongerLength_Rollback) {
		AssertValueModification(NotifyMode::Rollback, {
			{ 2, 7 },
			{ 0xE0 ^ 0xBA, 0x77 ^ 0xE8, 0xCC ^ 0xBC, 0xCC ^ 0x4B, 0x02 ^ 0x4B, 0xEE, 0x1F },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B, 0xEE, 0x1F },
			{ 0xE0, 0x77, 0xCC, 0xCC, 0x02 }
		});
	}

	// endregion
}}
