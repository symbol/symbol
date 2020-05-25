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

#include "catapult/cache_db/RdbColumnContainer.h"
#include "catapult/cache_db/RocksInclude.h"
#include "tests/catapult/cache_db/test/RdbTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS RdbColumnContainerTests

	namespace {
		// wrapper to expose and test protected methods
		class TestColumnContainer : public RdbColumnContainer {
		public:
			using RdbColumnContainer::RdbColumnContainer;

			void find(const RawBuffer& key, RdbDataIterator& iterator) const {
				RdbColumnContainer::find(key, iterator);
			}

			void insert(const RawBuffer& key, const std::string& value) {
				RdbColumnContainer::insert(key, value);
			}

			void remove(const RawBuffer& key) {
				RdbColumnContainer::remove(key);
			}

			size_t prune(uint64_t pruningBoundary) {
				return RdbColumnContainer::prune(pruningBoundary);
			}
		};

		auto CreateSettings(size_t numKilobytes, FilterPruningMode pruningMode = FilterPruningMode::Disabled) {
			return RocksDatabaseSettings(
					test::TempDirectoryGuard::DefaultName(),
					{ "default" },
					utils::FileSize::FromKilobytes(numKilobytes),
					pruningMode);
		}

		auto DefaultSettings() {
			return CreateSettings(0);
		}

		auto PruningSettings() {
			return CreateSettings(0, FilterPruningMode::Enabled);
		}

		auto BatchSettings() {
			return CreateSettings(100);
		}
	}

	// region size

	TEST(TEST_CLASS, SizeIsInitiallyZero) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings());
		RdbColumnContainer container(context.database(), 0);

		// Act:
		auto size = container.size();

		// Assert:
		EXPECT_EQ(0u, size);
	}

	TEST(TEST_CLASS, SizeCanBeReadFromDb) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings(), [](auto& db, const auto& columns) {
			db.Put(rocksdb::WriteOptions(), columns[0], "size", "\x12\x34\x56\x78\x90\xAB\xCD\xEF");
		});
		RdbColumnContainer container(context.database(), 0);

		// Act:
		auto size = container.size();

		// Assert:
		EXPECT_EQ(0xEFCDAB90'78563412ull, size);
	}

	TEST(TEST_CLASS, SetSizeWritesSizeToDb) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings());
		RdbColumnContainer container(context.database(), 0);

		// Act:
		container.setSize(0x12345678'90ABCDEFull);
		container.database().flush();

		// Assert:
		EXPECT_EQ(0x12345678'90ABCDEFull, container.size());
		{
			RdbColumnContainer containerCopy(context.database(), 0);
			EXPECT_EQ(0x12345678'90ABCDEFull, containerCopy.size());
		}
	}

	namespace {
		template<typename TContainer, typename TContainerKey>
		void AssertElementValue(const TContainer& container, const TContainerKey& key, const std::string& value) {
			RdbDataIterator iter;
			container.find(key, iter);
			test::AssertIteratorValue(value, iter);
		}
	}

	TEST(TEST_CLASS, FlushFinalizesBatch) {
		// Arrange: five elements in batch
		auto key = test::GenerateRandomArray<10>();
		test::RdbTestContext context(BatchSettings());
		TestColumnContainer container(context.database(), 0);

		// Act:
		std::string value = "1234567890";
		container.insert(key, value);
		container.database().flush();

		// Assert:
		AssertElementValue(container, key, value);
		{
			// - create copy to make sure element has actually been written to db
			TestColumnContainer containerCopy(context.database(), 0);
			AssertElementValue(containerCopy, key, value);
		}
	}

	// endregion

	// region properties

	namespace {
		using PropType = Address;
	}

	TEST(TEST_CLASS, PropFailsWhenPropertyIsNotPresent) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings());
		RdbColumnContainer container(context.database(), 0);

		// Act + Assert:
		PropType value;
		EXPECT_FALSE(container.prop("amazing", value));
	}

	TEST(TEST_CLASS, CanSetCustomProperty) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings());
		RdbColumnContainer container(context.database(), 0);
		PropType originalProperty;
		test::FillWithRandomData(originalProperty);

		// Act:
		container.setProp("amazing", originalProperty);

		// Assert:
		PropType value;
		EXPECT_TRUE(container.prop("amazing", value));
		EXPECT_EQ(originalProperty, value);
	}

	TEST(TEST_CLASS, PropFailsWhenPropertyNameIsTooLong) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings(), [](auto& db, const auto& columns) {
			db.Put(rocksdb::WriteOptions(), columns[0], "12345678", "\x12\x34\x56\x78\x90\xAB\xCD\xEF");
		});
		RdbColumnContainer container(context.database(), 0);

		// Act + Assert:
		uint64_t value;
		EXPECT_THROW(container.prop("12345678", value), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, SetPropFailsWhenPropertyNameIsTooLong) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings());
		RdbColumnContainer container(context.database(), 0);
		PropType property;

		// Act:
		EXPECT_THROW(container.setProp("12345678", property), catapult_invalid_argument);
	}

	// endregion

	// region database

	TEST(TEST_CLASS, DatabaseReturnsUnderlyingDatabase) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings());
		RdbColumnContainer container(context.database(), 0);

		// Act + Assert:
		EXPECT_EQ(&context.database(), &container.database());
	}

	// endregion

	// region find + insert + remove

	namespace {
		template<typename TContainer>
		auto ToSlice(const TContainer& container) {
			return rocksdb::Slice(reinterpret_cast<const char*>(container.data()), container.size());
		}
	}

	TEST(TEST_CLASS, FindForwardsToGet) {
		// Arrange:
		auto key = test::GenerateRandomArray<10>();
		test::RdbTestContext context(DefaultSettings(), [&key](auto& db, const auto& columns) {
			db.Put(rocksdb::WriteOptions(), columns[0], ToSlice(key), "world");
		});
		TestColumnContainer container(context.database(), 0);

		// Act:
		RdbDataIterator iter;
		container.find(key, iter);

		// Assert:
		test::AssertIteratorValue("world", iter);
	}

	TEST(TEST_CLASS, InsertForwardsToPut) {
		// Arrange:
		auto key = test::GenerateRandomArray<10>();
		test::RdbTestContext context(DefaultSettings());
		TestColumnContainer container(context.database(), 0);

		// Act:
		container.insert(key, "1234567890");

		// Assert:
		RdbDataIterator iter;
		container.find(key, iter);
		test::AssertIteratorValue("1234567890", iter);
	}

	TEST(TEST_CLASS, InsertForwardsToPut_Batched) {
		// Arrange: five elements in batch
		auto key = test::GenerateRandomArray<10>();
		test::RdbTestContext context(BatchSettings());
		TestColumnContainer container(context.database(), 0);

		// Act:
		container.insert(key, "1234567890");

		// Assert: cannot retrieve element because batch wasn't finalized
		RdbDataIterator iter;
		container.find(key, iter);
		EXPECT_EQ(RdbDataIterator::End(), iter);
	}

	TEST(TEST_CLASS, RemoveForwardsToDel) {
		// Arrange:
		auto key = test::GenerateRandomArray<10>();
		test::RdbTestContext context(DefaultSettings(), [&key](auto& db, const auto& columns) {
			db.Put(rocksdb::WriteOptions(), columns[0], ToSlice(key), "world");
		});
		TestColumnContainer container(context.database(), 0);

		// Act:
		container.remove(key);

		// Assert:
		RdbDataIterator iter;
		container.find(key, iter);
		EXPECT_EQ(RdbDataIterator::End(), iter);
	}

	// endregion

	// region prune

	namespace {
		auto ToRawBuffer(const uint64_t& key) {
			return RawBuffer(test::AsBytePointer(&key), sizeof(uint64_t));
		}

		void AssertNoKey(TestColumnContainer& container, uint64_t key) {
			RdbDataIterator iter;
			container.find(ToRawBuffer(key), iter);
			EXPECT_EQ(RdbDataIterator::End(), iter);
		}

		void AssertValidKey(TestColumnContainer& container, uint64_t key) {
			RdbDataIterator iter;
			container.find(ToRawBuffer(key), iter);
			auto value = test::EvenKeyToValue(key);
			test::AssertIteratorValue(value, iter);
		}

		template<typename TAction>
		void AssertKeys(TestColumnContainer& container, size_t begin, size_t end, TAction action) {
			for (auto key = begin; key < end; key += 2)
				action(container, key);
		}
	}

	TEST(TEST_CLASS, PruneForwardsToPrune) {
		// Arrange: create 120 even keys (0 - 238)
		auto evenSeeder = test::CreateEvenDbSeeder(120);
		test::RdbTestContext context(PruningSettings(), evenSeeder);
		TestColumnContainer container(context.database(), 0);

		// Act: prune all keys < 200
		auto numRemoved = container.prune(200);

		// Assert:
		EXPECT_EQ(100u, numRemoved);
		AssertKeys(container, 0, 200, AssertNoKey);
		AssertKeys(container, 200, 238, AssertValidKey);
	}

	// endregion
}}
