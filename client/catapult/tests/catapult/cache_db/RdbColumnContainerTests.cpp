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

	TEST(TEST_CLASS, SizeIsInitiallyZero) {
		// Arrange:
		test::RdbTestContext context({});
		RdbColumnContainer container(context.database(), 0);

		// Act:
		auto size = container.size();

		// Assert:
		EXPECT_EQ(0u, size);
	}

	TEST(TEST_CLASS, SizeCanBeReadFromDb) {
		// Arrange:
		test::RdbTestContext context({}, [](auto& db, const auto& columns) {
			db.Put(rocksdb::WriteOptions(), columns[0], "size", "\x12\x34\x56\x78\x90\xAB\xCD\xEF");
		});
		RdbColumnContainer container(context.database(), 0);

		// Act:
		auto size = container.size();

		// Assert:
		EXPECT_EQ(0xEFCDAB90'78563412ull, size);
	}

	TEST(TEST_CLASS, SaveSizeWritesSizeToDb) {
		// Arrange:
		test::RdbTestContext context({});
		RdbColumnContainer container(context.database(), 0);

		// Act:
		container.saveSize(0x12345678'90ABCDEFull);

		// Assert:
		EXPECT_EQ(0x12345678'90ABCDEFull, container.size());
		{
			RdbColumnContainer containerCopy(context.database(), 0);
			EXPECT_EQ(0x12345678'90ABCDEFull, containerCopy.size());
		}
	}

	namespace {
		template<typename TContainer>
		auto ToSlice(const TContainer& container) {
			return rocksdb::Slice(reinterpret_cast<const char*>(container.data()), container.size());
		}
	}

	TEST(TEST_CLASS, FindForwardsToGet) {
		// Arrange:
		auto key = test::GenerateRandomData<10>();
		test::RdbTestContext context({}, [&key](auto& db, const auto& columns) {
			db.Put(rocksdb::WriteOptions(), columns[0], ToSlice(key), "world");
		});
		RdbColumnContainer container(context.database(), 0);

		// Act:
		RdbDataIterator iter;
		container.find(key, iter);

		// Assert:
		test::AssertIteratorValue("world", iter);
	}

	TEST(TEST_CLASS, InsertForwardsToPut) {
		// Arrange:
		auto key = test::GenerateRandomData<10>();
		test::RdbTestContext context({});
		RdbColumnContainer container(context.database(), 0);

		// Act:
		container.insert(key, "1234567890");

		// Assert:
		RdbDataIterator iter;
		container.find(key, iter);
		test::AssertIteratorValue("1234567890", iter);
	}

	TEST(TEST_CLASS, RemoveForwardsToDel) {
		// Arrange:
		auto key = test::GenerateRandomData<10>();
		test::RdbTestContext context({}, [&key](auto& db, const auto& columns) {
			db.Put(rocksdb::WriteOptions(), columns[0], ToSlice(key), "world");
		});
		RdbColumnContainer container(context.database(), 0);

		// Act:
		container.remove(key);

		// Assert:
		RdbDataIterator iter;
		container.find(key, iter);
		EXPECT_EQ(RdbDataIterator::End(), iter);
	}
}}
