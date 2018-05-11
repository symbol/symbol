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

#include "RdbTestUtils.h"
#include "catapult/cache_db/RocksInclude.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	bool DbInitializer::SeedDb(const std::string& dbDir, const std::vector<std::string>& columns, const DbSeeder& seeder) {
		if (!seeder)
			return false;

		rocksdb::DB* pDb;
		rocksdb::Options dbOptions;
		dbOptions.create_if_missing = true;
		dbOptions.create_missing_column_families = true;

		std::vector<rocksdb::ColumnFamilyDescriptor> columnFamilies;
		columnFamilies.push_back(rocksdb::ColumnFamilyDescriptor("default", rocksdb::ColumnFamilyOptions()));
		for (const auto& name : columns)
			columnFamilies.push_back(rocksdb::ColumnFamilyDescriptor(name, rocksdb::ColumnFamilyOptions()));

		std::vector<rocksdb::ColumnFamilyHandle*> handles;
		auto status = rocksdb::DB::Open(dbOptions, dbDir, columnFamilies, &handles, &pDb);
		std::unique_ptr<rocksdb::DB> pDbGuard(pDb);
		std::vector<std::shared_ptr<rocksdb::ColumnFamilyHandle>> handleGuards;
		for (auto* pHandle : handles) {
			handleGuards.emplace_back(pHandle, [&db = *pDb](auto* pColumnHandle) {
				db.DestroyColumnFamilyHandle(pColumnHandle);
			});
		}

		seeder(*pDbGuard, handles);
		return true;
	}

	DbInitializer::DbInitializer(const std::string& dbDir, const ColumnNames& columns, const DbSeeder& seeder) {
		rocksdb::DestroyDB(dbDir, {});
		SeedDb(dbDir, columns, seeder);
	}

	RdbTestContext::RdbTestContext(const ColumnNames& columns, const DbSeeder& seeder)
			: DbInitializer("testdb", columns, seeder)
			, m_database("testdb", columns)
	{}

	cache::RocksDatabase& RdbTestContext::database() {
		return m_database;
	}

	void AssertIteratorValue(const std::string& value, const cache::RdbDataIterator& iter) {
		ASSERT_NE(cache::RdbDataIterator::End(), iter);
		EXPECT_EQ(value.size(), iter.storage().size());
		EXPECT_EQ(value, iter.storage().data());
	}
}}
