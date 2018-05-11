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

#include "catapult/cache_db/RocksDatabase.h"
#include "catapult/cache_db/RocksInclude.h"
#include "catapult/io/FileLock.h"
#include "catapult/io/RawFile.h"
#include "tests/catapult/cache_db/test/RdbTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS RocksDatabaseTests

	TEST(TEST_CLASS, RdbThrowsIfDbCannotBeOpened) {
		// Arrange: create a lock file with a name that will be used by Open
		{
			rocksdb::DestroyDB("testdb", {});
			test::TempDirectoryGuard dirGuard("testdb");
		}
		io::FileLock lock("testdb");
		lock.lock();

		// Act + Assert:
		EXPECT_THROW(RocksDatabase("testdb", {}), catapult_runtime_error);
	}

	TEST(TEST_CLASS, ReadingNonExistentKeyReturnsSentinelValue) {
		// Arrange:
		test::RdbTestContext context({});
		auto& database = context.database();

		// Act:
		RdbDataIterator iter;
		database.get(0, "hello", iter);

		// Assert:
		EXPECT_EQ(RdbDataIterator::End(), iter);
	}

	// region single value

	TEST(TEST_CLASS, CanReadFromDb_DefaultColumn) {
		// Arrange:
		test::RdbTestContext context({}, [](auto& db, const auto& columns) {
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
		test::RdbTestContext context({});
		auto& database = context.database();

		// Act:
		database.put(0, "hello", "amazing");

		// Assert:
		RdbDataIterator iter;
		database.get(0, "hello", iter);
		test::AssertIteratorValue("amazing", iter);
	}

	TEST(TEST_CLASS, CanDeleteFromDb_DefaultColumn_NonExistingKey) {
		// Arrange:
		test::RdbTestContext context({});
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
		test::RdbTestContext context({});
		auto& database = context.database();
		database.put(0, "hello", "amazing");
		database.put(0, "world", "awesome");

		// Sanity: 'hello' is present
		RdbDataIterator iter;
		database.get(0, "hello", iter);
		EXPECT_NE(RdbDataIterator::End(), iter);

		// Act:
		database.del(0, "hello");

		// Assert: 'hello' is removed
		database.get(0, "hello", iter);
		EXPECT_EQ(RdbDataIterator::End(), iter);

		// Sanity: 'world' is left untouched
		AssertKeyValueColumn0(database, "world", "awesome");
	}

	// endregion

	namespace {
		auto GetHelloKeyFromColumns(RocksDatabase& database, size_t numColumns) {
			std::vector<RdbDataIterator> iters(numColumns);
			for (auto i = 0u; i < numColumns; ++i)
				database.get(i, "hello", iters[i]);

			return iters;
		}
	}

	// region different columns

	TEST(TEST_CLASS, CanReadFromDb_DifferentColumns) {
		// Arrange:
		test::RdbTestContext context({ "beta", "gamma" }, [](auto& db, const auto& columns) {
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
		test::RdbTestContext context({ "beta", "gamma" });
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
		test::RdbTestContext context({ "beta", "gamma" }, [](auto& db, const auto& columns) {
			db.Put(rocksdb::WriteOptions(), columns[0], "hello", "amazing");
			db.Put(rocksdb::WriteOptions(), columns[0], "world", "fractured");
			db.Put(rocksdb::WriteOptions(), columns[1], "hello", "awesome");
			db.Put(rocksdb::WriteOptions(), columns[2], "hello", "incredible");
		});
		auto& database = context.database();

		// Act:
		for (auto i = 0u; i < 3; ++i)
			database.del(i, "hello");

		// Assert: 'hello' keys are removed
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
		test::RdbTestContext context({}, [](auto& db, const auto& columns) {
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
		test::RdbTestContext context({});
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
		test::RdbTestContext context({});
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

		// Assert: 'hello' and 'world' are removed
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

		void AssertIteratorValue(KeyState keyState, const std::string& value, RdbDataIterator& iter) {
			if (KeyState::Existent == keyState)
				test::AssertIteratorValue(value, iter);
			else
				EXPECT_EQ(RdbDataIterator::End(), iter);
		}

		void AssertCanReuseIteratorWhenReadingFromDb(KeyState first, KeyState second) {
			// Arrange:
			test::RdbTestContext context({ "beta", "gamma" }, [](auto& db, const auto& columns) {
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
}}
