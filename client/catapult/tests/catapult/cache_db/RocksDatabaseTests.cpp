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

#include "catapult/cache_db/RocksDatabase.h"
#include "catapult/io/FileLock.h"
#include "catapult/io/RawFile.h"
#include "tests/catapult/cache_db/test/RdbTestUtils.h"
#include "tests/catapult/cache_db/test/SliceTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <filesystem>

namespace catapult { namespace cache {

#define TEST_CLASS RocksDatabaseTests

	namespace {
		auto CreateSettings(
				const std::vector<std::string>& columnNames,
				size_t numKilobytes = 0,
				FilterPruningMode pruningMode = FilterPruningMode::Disabled) {
			auto config = config::NodeConfiguration::CacheDatabaseSubConfiguration();
			config.MaxWriteBatchSize = utils::FileSize::FromKilobytes(numKilobytes);
			return RocksDatabaseSettings(test::TempDirectoryGuard::DefaultName(), config, columnNames, pruningMode);
		}

		auto DefaultSettings() {
			return CreateSettings({ "default" });
		}

		auto PruningSettings() {
			return CreateSettings({ "default" }, 0, FilterPruningMode::Enabled);
		}

		auto BatchSettings() {
			return CreateSettings({ "default" }, 100);
		}

		auto MultiColumnSettings() {
			return CreateSettings({ "default", "beta", "gamma" });
		}
	}

	// region constructor

	TEST(TEST_CLASS, RdbThrowsWhenNoColumnsAreGiven) {
		EXPECT_THROW(RocksDatabase(CreateSettings({})), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, RdbThrowsWhenDbCannotBeOpened) {
		// Arrange: use TempDirectoryGuard to create any intermediate directories (except for last one)
		test::TempDirectoryGuard dbDirGuard;
		std::filesystem::remove(dbDirGuard.name());

		// - create a lock file with a name that will be used by Open
		io::FileLock lock(dbDirGuard.name());
		lock.lock();

		// Act + Assert:
		EXPECT_THROW(RocksDatabase(CreateSettings({ "default" })), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanOpenDatabaseWithValidBatchSize) {
		// Arrange:
		test::TempDirectoryGuard dbDirGuard;

		// Act:
		RocksDatabase database(CreateSettings({ "default", "foo" }, 100));

		// Assert:
		EXPECT_EQ((std::vector<std::string>{ "default", "foo" }), database.columnFamilyNames());
		EXPECT_FALSE(database.canPrune());
	}

	TEST(TEST_CLASS, CanOpenDatabaseWithZeroBatchSize) {
		// Arrange:
		test::TempDirectoryGuard dbDirGuard;

		// Act:
		RocksDatabase database(CreateSettings({ "default", "foo" }, 0));

		// Assert:
		EXPECT_EQ((std::vector<std::string>{ "default", "foo" }), database.columnFamilyNames());
		EXPECT_FALSE(database.canPrune());
	}

	TEST(TEST_CLASS, CanOpenDatabaseWithPruningEnabled) {
		// Arrange:
		test::TempDirectoryGuard dbDirGuard;

		// Act:
		RocksDatabase database(CreateSettings({ "default", "foo" }, 100, FilterPruningMode::Enabled));

		// Assert:
		EXPECT_EQ((std::vector<std::string>{ "default", "foo" }), database.columnFamilyNames());
		EXPECT_TRUE(database.canPrune());
	}

	TEST(TEST_CLASS, CanCreatePlaceholderDatabase) {
		// Act:
		RocksDatabase database;

		// Assert:
		EXPECT_TRUE(database.columnFamilyNames().empty());
		EXPECT_FALSE(database.canPrune());
	}

	// endregion

	// region single value

	TEST(TEST_CLASS, ReadingNonexistentKeyReturnsSentinelValue) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings());
		auto& database = context.database();

		// Act:
		RdbDataIterator iter;
		database.get(0, "hello", iter);

		// Assert:
		EXPECT_EQ(RdbDataIterator::End(), iter);
	}

	TEST(TEST_CLASS, CanReadFromDb_DefaultColumn) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings(), [](auto& db, const auto& columns) {
			db.Put(rocksdb::WriteOptions(), columns[0], "hello", "amazing");
		});
		auto& database = context.database();

		// Act:
		RdbDataIterator iter;
		database.get(0, "hello", iter);

		// Assert:
		test::AssertIteratorValue("amazing", iter);
	}

	TEST(TEST_CLASS, CanWriteToDb_DefaultColumn) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings());
		auto& database = context.database();

		// Act:
		database.put(0, "hello", "amazing");

		// Assert:
		RdbDataIterator iter;
		database.get(0, "hello", iter);
		test::AssertIteratorValue("amazing", iter);
	}

	TEST(TEST_CLASS, CanDeleteFromDb_DefaultColumn_NonexistentKey) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings());
		auto& database = context.database();

		// Act: exception is not thrown
		EXPECT_NO_THROW(database.del(0, "hello"));
	}

	namespace {
		void AssertKeyValueColumn0(RocksDatabase& database, const std::string& key, const std::string& value) {
			RdbDataIterator iter;
			database.get(0, key, iter);
			test::AssertIteratorValue(value, iter);
		}
	}

	TEST(TEST_CLASS, CanDeleteFromDb_DefaultColumn) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings());
		auto& database = context.database();
		database.put(0, "hello", "amazing");
		database.put(0, "world", "awesome");

		// Sanity: 'hello' is present
		RdbDataIterator iter;
		database.get(0, "hello", iter);
		EXPECT_NE(RdbDataIterator::End(), iter);

		// Act:
		database.del(0, "hello");

		// Assert: 'hello' is deleted
		database.get(0, "hello", iter);
		EXPECT_EQ(RdbDataIterator::End(), iter);

		// Sanity: 'world' is left untouched
		AssertKeyValueColumn0(database, "world", "awesome");
	}

	// endregion

	// region default db ctor

	TEST(TEST_CLASS, DefaultCreatedRdbDoesNotAllowGet) {
		// Arrange:
		RocksDatabase database;

		// Act + Assert:
		RdbDataIterator iter;
		EXPECT_THROW(database.get(0, "hello", iter), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, DefaultCreatedRdbDoesNotAllowPut) {
		// Arrange:
		RocksDatabase database;

		// Act + Assert:
		EXPECT_THROW(database.put(0, "hello", "amazing"), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, DefaultCreatedRdbDoesNotAllowDelete) {
		// Arrange:
		RocksDatabase database;

		// Act + Assert:
		EXPECT_THROW(database.del(0, "hello"), catapult_invalid_argument);
	}

	// endregion

	namespace {
		auto GetKeyFromColumns(RocksDatabase& database, const std::string& name, size_t numColumns) {
			std::vector<RdbDataIterator> iters(numColumns);
			for (auto i = 0u; i < numColumns; ++i)
				database.get(i, name, iters[i]);

			return iters;
		}

		auto GetHelloKeyFromColumns(RocksDatabase& database, size_t numColumns) {
			return GetKeyFromColumns(database, "hello", numColumns);
		}
	}

	// region different columns

	TEST(TEST_CLASS, CanReadFromDb_DifferentColumns) {
		// Arrange:
		test::RdbTestContext context(MultiColumnSettings(), [](auto& db, const auto& columns) {
			db.Put(rocksdb::WriteOptions(), columns[0], "hello", "amazing");
			db.Put(rocksdb::WriteOptions(), columns[1], "hello", "awesome");
			db.Put(rocksdb::WriteOptions(), columns[2], "hello", "incredible");
		});
		auto& database = context.database();

		// Act:
		auto iters = GetHelloKeyFromColumns(database, 3);

		// Assert:
		test::AssertIteratorValue("amazing", iters[0]);
		test::AssertIteratorValue("awesome", iters[1]);
		test::AssertIteratorValue("incredible", iters[2]);
	}

	TEST(TEST_CLASS, CanWriteToDb_DifferentColumns) {
		// Arrange:
		test::RdbTestContext context(MultiColumnSettings(), [](const auto&, const auto&) {});
		auto& database = context.database();

		// Act:
		database.put(0, "hello", "amazing");
		database.put(1, "hello", "awesome");
		database.put(2, "hello", "incredible");

		// Assert:
		auto iters = GetHelloKeyFromColumns(database, 3);

		test::AssertIteratorValue("amazing", iters[0]);
		test::AssertIteratorValue("awesome", iters[1]);
		test::AssertIteratorValue("incredible", iters[2]);
	}

	TEST(TEST_CLASS, CanDeleteFromDb_DifferentColumns) {
		// Arrange:
		test::RdbTestContext context(MultiColumnSettings(), [](auto& db, const auto& columns) {
			db.Put(rocksdb::WriteOptions(), columns[0], "hello", "amazing");
			db.Put(rocksdb::WriteOptions(), columns[0], "world", "fractured");
			db.Put(rocksdb::WriteOptions(), columns[1], "hello", "awesome");
			db.Put(rocksdb::WriteOptions(), columns[2], "hello", "incredible");
		});
		auto& database = context.database();

		// Act:
		for (auto i = 0u; i < 3; ++i)
			database.del(i, "hello");

		// Assert: 'hello' keys are deleted
		auto iters = GetHelloKeyFromColumns(database, 3);
		for (const auto& iter : iters)
			EXPECT_EQ(RdbDataIterator::End(), iter);

		// Sanity: 'world' is left untouched
		AssertKeyValueColumn0(database, "world", "fractured");
	}

	// endregion

	// region multiple values

	TEST(TEST_CLASS, CanReadFromDb_MultipleValues) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings(), [](auto& db, const auto& columns) {
			db.Put(rocksdb::WriteOptions(), columns[0], "hello", "amazing");
			db.Put(rocksdb::WriteOptions(), columns[0], "world", "awesome");
		});
		auto& database = context.database();

		// Act:
		RdbDataIterator iter1;
		RdbDataIterator iter2;
		database.get(0, "hello", iter1);
		database.get(0, "world", iter2);

		// Assert:
		test::AssertIteratorValue("amazing", iter1);
		test::AssertIteratorValue("awesome", iter2);
	}

	TEST(TEST_CLASS, CanWriteToDb_MultipleValues) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings());
		auto& database = context.database();

		// Act:
		database.put(0, "hello", "amazing");
		database.put(0, "world", "awesome");

		// Assert:
		RdbDataIterator iter1;
		RdbDataIterator iter2;
		database.get(0, "hello", iter1);
		database.get(0, "world", iter2);

		test::AssertIteratorValue("amazing", iter1);
		test::AssertIteratorValue("awesome", iter2);
	}

	TEST(TEST_CLASS, CanDeleteFromDb_MultipleValues) {
		// Arrange:
		test::RdbTestContext context(DefaultSettings());
		auto& database = context.database();
		database.put(0, "hello", "amazing");
		database.put(0, "world", "awesome");
		database.put(0, "apple", "incredible");

		// Act:
		database.del(0, "hello");
		database.del(0, "world");

		// Assert:
		RdbDataIterator iters[2];
		database.get(0, "hello", iters[0]);
		database.get(0, "world", iters[1]);

		// Assert: 'hello' and 'world' are deleted
		EXPECT_EQ(RdbDataIterator::End(), iters[0]);
		EXPECT_EQ(RdbDataIterator::End(), iters[1]);

		// Sanity: 'apple' is left untouched
		AssertKeyValueColumn0(database, "apple", "incredible");
	}

	// endregion

	// region iterators

	namespace {
		enum class KeyState {
			Existent,
			Nonexistent
		};

		auto StateToKey(KeyState keyState) {
			return KeyState::Existent == keyState ? "hello" : "nonexistent";
		}

		void AssertIteratorValue(KeyState keyState, const std::string& value, const RdbDataIterator& iter) {
			if (KeyState::Existent == keyState)
				test::AssertIteratorValue(value, iter);
			else
				EXPECT_EQ(RdbDataIterator::End(), iter);
		}

		void AssertCanReuseIteratorWhenReadingFromDb(KeyState first, KeyState second) {
			// Arrange:
			test::RdbTestContext context(MultiColumnSettings(), [](auto& db, const auto& columns) {
				db.Put(rocksdb::WriteOptions(), columns[0], "hello", "amazing");
				db.Put(rocksdb::WriteOptions(), columns[1], "hello", "awesome");
				db.Put(rocksdb::WriteOptions(), columns[2], "hello", "incredible");
			});
			auto& database = context.database();

			// Act:
			RdbDataIterator iter;
			database.get(0, StateToKey(first), iter);

			// Sanity:
			AssertIteratorValue(first, "amazing", iter);

			// Act:
			database.get(1, StateToKey(second), iter);

			// Assert:
			AssertIteratorValue(second, "awesome", iter);
		}
	}

	TEST(TEST_CLASS, CanReuseIteratorWhenReadingFromDb_ExistentAndExistent) {
		AssertCanReuseIteratorWhenReadingFromDb(KeyState::Existent, KeyState::Existent);
	}

	TEST(TEST_CLASS, CanReuseIteratorWhenReadingFromDb_ExistentAndNonexistent) {
		AssertCanReuseIteratorWhenReadingFromDb(KeyState::Existent, KeyState::Nonexistent);
	}

	TEST(TEST_CLASS, CanReuseIteratorWhenReadingFromDb_NonexistentAndExistent) {
		AssertCanReuseIteratorWhenReadingFromDb(KeyState::Nonexistent, KeyState::Existent);
	}

	TEST(TEST_CLASS, CanReuseIteratorWhenReadingFromDb_NonexistentAndNonexistent) {
		AssertCanReuseIteratorWhenReadingFromDb(KeyState::Nonexistent, KeyState::Nonexistent);
	}

	// endregion

	// region pruning

	namespace {
		void AssertNoKey(cache::RocksDatabase& database, uint64_t value) {
			RdbDataIterator iter;
			database.get(0, test::ToSlice(value), iter);

			EXPECT_EQ(RdbDataIterator::End(), iter);
		}

		void AssertHasValidKey(cache::RocksDatabase& database, uint64_t key) {
			RdbDataIterator iter;
			database.get(0, test::ToSlice(key), iter);

			auto strValue = test::EvenKeyToValue(key);
			test::AssertIteratorValue(strValue, iter);
		}

		template<typename TAssertion>
		void RunPruneTest(TAssertion assertion) {
			// Arrange: create 120 even keys (0 - 238)
			auto evenSeeder = test::CreateEvenDbSeeder(120);
			test::RdbTestContext context(PruningSettings(), evenSeeder);

			// Act: prune all keys < 200
			auto numPruned = context.database().prune(0, 200);

			// Assert:
			EXPECT_EQ(100u, numPruned);
			assertion(context.database());
		}
	}

	TEST(TEST_CLASS, PruneRemovesAllValuesBelowBoundary) {
		RunPruneTest([](auto& database) {
			// Assert:
			for (auto i = 0u; i < 200; i += 2)
				AssertNoKey(database, i);
		});
	}

	TEST(TEST_CLASS, PruneDoesNotRemoveValuesAboveBoundary) {
		RunPruneTest([](auto& database) {
			// Assert:
			for (auto i = 200u; i < 240; i += 2)
				AssertHasValidKey(database, i);
		});
	}

	// endregion

	// region batch processing

	namespace {
		enum class Flush {
			No,
			Yes
		};

		template<typename TAction>
		void RunBasicBatchTest(const test::DbSeeder& seeder, Flush enableFlush, KeyState keyState, TAction action) {
			// Arrange:
			test::RdbTestContext context(BatchSettings(), seeder);
			auto& database = context.database();

			// Act:
			action(database);
			if (Flush::Yes == enableFlush)
				database.flush();

			// Assert: validate element presence depending on keyState
			RdbDataIterator iter;
			database.get(0, "hello", iter);
			AssertIteratorValue(keyState, "amazing", iter);
		}

		void RunPutTest(Flush enableFlush, KeyState keyState) {
			RunBasicBatchTest(test::DbSeeder(), enableFlush, keyState, [](auto& database) {
				database.put(0, "hello", "amazing");
			});
		}

		void RunDelTest(Flush enableFlush, KeyState keyState) {
			auto seeder = [](auto& db, const auto& columns) {
				db.Put(rocksdb::WriteOptions(), columns[0], "hello", "amazing");
			};
			RunBasicBatchTest(seeder, enableFlush, keyState, [](auto& database) {
				database.del(0, "hello");
			});
		}
	}

	TEST(TEST_CLASS, SinglePutDoesNotTriggerBatchedWrite) {
		RunPutTest(Flush::No, KeyState::Nonexistent);
	}

	TEST(TEST_CLASS, FinalizeBatchCommitsBatchedPuts) {
		RunPutTest(Flush::Yes, KeyState::Existent);
	}

	TEST(TEST_CLASS, SingleDelDoesNotTriggerBatchedWrite) {
		RunDelTest(Flush::No, KeyState::Existent);
	}

	TEST(TEST_CLASS, FinalizeBatchCommitsBatchedDels) {
		RunDelTest(Flush::Yes, KeyState::Nonexistent);
	}

	namespace {
		bool IsIteratorEnd(const RdbDataIterator& iter) {
			return RdbDataIterator::End() == iter;
		}

		bool IsNotIteratorEnd(const RdbDataIterator& iter) {
			return !IsIteratorEnd(iter);
		}

		auto Negate(KeyState state) {
			return state == KeyState::Existent ? KeyState::Nonexistent : KeyState::Existent;
		}

		template<size_t Num_Elements>
		auto VerifyIters(const std::array<RdbDataIterator, Num_Elements>& iters, KeyState firstGroupState, size_t valueAdjustment = 0) {
			auto condition = firstGroupState == KeyState::Existent ? IsIteratorEnd : IsNotIteratorEnd;
			auto pivotIndex = static_cast<size_t>(std::distance(iters.cbegin(), std::find_if(iters.cbegin(), iters.cend(), condition)));

			// Sanity: pivot index is somewhere between
			EXPECT_LT(0u, pivotIndex);
			EXPECT_GT(Num_Elements - 1, pivotIndex);

			// Assert: verify iterator values
			for (auto i = 0u; i < Num_Elements; ++i) {
				auto value = test::EvenKeyToValue(valueAdjustment + i * 2);
				auto expectedState = !condition(iters[i]) ? firstGroupState : Negate(firstGroupState);
				AssertIteratorValue(expectedState, value, iters[i]);
			}

			return pivotIndex;
		}

		void PutTriggersBatchedWrite(const std::vector<std::string>& columnNames) {
			// Arrange:
			auto numColumns = columnNames.size();
			auto settings = CreateSettings(columnNames, 100);
			test::RdbTestContext context(settings);
			auto& database = context.database();

			// Act: entry size is ~17 bytes
			//      at least 100k / 17 entries are needed to trigger write, use bit bigger number
			constexpr auto Num_Elements = 8'000u;
			for (auto i = 0u; i < Num_Elements; ++i)
				database.put(0 % numColumns, test::ToSlice(i * 2), test::EvenKeyToValue(i * 2));

			// Assert:
			std::array<RdbDataIterator, Num_Elements> iters;
			for (auto i = 0u; i < Num_Elements; ++i)
				database.get(0 % numColumns, test::ToSlice(i * 2), iters[i]);

			// - verify that not all entries have been saved
			// * [0, pivotElementIndex) - elements should be added to db
			// * [pivotElementIndex, Num_Elements) - should not be in db
			auto pivotElementIndex = VerifyIters(iters, KeyState::Existent);

			// - at least 6k puts were required to trigger write (this value will be used in mixed test)
			EXPECT_LT(6000u, pivotElementIndex);
		}
	}

	TEST(TEST_CLASS, PutTriggersBatchedWrite) {
		PutTriggersBatchedWrite({ "default" });
	}

	TEST(TEST_CLASS, PutTriggersBatchedWrite_MultipleColumns) {
		PutTriggersBatchedWrite({ "default", "beta", "gamma" });
	}

	TEST(TEST_CLASS, DelTriggersBatchedWrite) {
		// Arrange:  for some magic when using even seeder entry size is ~10 bytes (I'd expect at least 8 + 6 = 12),
		//           at least 100k / 10 entries, are needed, use bit bigger number to leave some undeleted keys
		constexpr auto Num_Elements = 12'000;
		auto evenSeeder = test::CreateEvenDbSeeder(Num_Elements);
		test::RdbTestContext context(BatchSettings(), evenSeeder);
		auto& database = context.database();

		// Act:
		for (auto i = 0u; i < Num_Elements; ++i)
			database.del(0, test::ToSlice(i * 2));

		// Assert:
		std::array<RdbDataIterator, Num_Elements> iters;
		for (auto i = 0u; i < Num_Elements; ++i)
			database.get(0, test::ToSlice(i * 2), iters[i]);

		// - verify that not all entries have been deleted
		// * [0, pivotElementIndex) - elements should be deleted from db
		// * [pivotElementIndex, Num_Elements) - elements should be left in db
		auto pivotElementIndex = VerifyIters(iters, KeyState::Nonexistent);
		EXPECT_LT(10000u, pivotElementIndex);
	}

	TEST(TEST_CLASS, MixedPutDelTriggersBatchedWrite) {
		// Arrange:
		constexpr auto Num_Elements = 12'000;
		auto evenSeeder = test::CreateEvenDbSeeder(Num_Elements);
		test::RdbTestContext context(BatchSettings(), evenSeeder);
		auto& database = context.database();

		// Act: add some new data and remove some data from db,
		//      make sure keys don't conflict to make assertions simpler
		constexpr auto Num_Operations = 6000;
		for (auto i = 0u; i < Num_Operations; ++i) {
			database.put(0, test::ToSlice(50'000 + i * 2), test::EvenKeyToValue(50'000 + i * 2));
			database.del(0, test::ToSlice(i * 2));
		}

		// Assert:
		std::array<RdbDataIterator, Num_Operations> putIters;
		for (auto i = 0u; i < Num_Operations; ++i)
			database.get(0, test::ToSlice(50'000 + i * 2), putIters[i]);

		std::array<RdbDataIterator, Num_Operations> delIters;
		for (auto i = 0u; i < Num_Operations; ++i)
			database.get(0, test::ToSlice(i * 2), delIters[i]);

		// - verify that not all entries have been saved
		// * [0, putPivotIndex) - elements should be added to db
		// * [putPivotIndex, Num_Operations) - elements should not be in db
		auto putPivotIndex = VerifyIters(putIters, KeyState::Existent, 50'000);

		// - there were interleaving puts and dels, so much less operations were needed to trigger write
		//   (compare with PutTriggersBatchedWrite)
		EXPECT_GT(5000u, putPivotIndex);

		// - verify that not all entries have been deleted
		// * [0, delPivotIndex) - elements should be deleted from db
		// * [delPivotIndex, Num_Operations) - should be left in db
		auto delPivotIndex = VerifyIters(delIters, KeyState::Nonexistent);

		// - puts and dels are interleaving, so same condition should be fulfilled for dels as well
		EXPECT_GT(5000u, delPivotIndex);
	}

	// endregion
}}
