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

#include "RdbTestUtils.h"
#include "SliceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		void PutValue(rocksdb::DB& db, rocksdb::ColumnFamilyHandle* column, size_t key) {
			db.Put(rocksdb::WriteOptions(), column, test::ToSlice(key), EvenKeyToValue(key));
		}
	}

	std::string EvenKeyToValue(size_t key) {
		return std::to_string(123400 + key);
	}

	DbSeeder CreateEvenDbSeeder(size_t numKeys) {
		return [numKeys] (auto& db, const auto& columns) {
			for (auto i = 0u; i < numKeys; ++i)
				PutValue(db, columns[0], i * 2);
		};
	}

	DbInitializer::DbInitializer(const ColumnNames& columns, const DbSeeder& seeder) : DbInitializer(columns, seeder, nullptr)
	{}

	DbInitializer::DbInitializer(const ColumnNames& columns, const DbSeeder& seeder, const rocksdb::CompactionFilter* compactionFilter) {
		seedDb(m_dbDirGuard.name(), columns, seeder, compactionFilter);
	}

	bool DbInitializer::seedDb(
			const std::string& dbDir,
			const std::vector<std::string>& columns,
			const DbSeeder& seeder,
			const rocksdb::CompactionFilter* compactionFilter) {
		if (!seeder)
			return false;

		rocksdb::DB* pDb;
		rocksdb::Options dbOptions;
		dbOptions.create_if_missing = true;
		dbOptions.create_missing_column_families = true;

		rocksdb::ColumnFamilyOptions defaultColumnOptions;
		defaultColumnOptions.compaction_filter = compactionFilter;

		std::vector<rocksdb::ColumnFamilyDescriptor> columnFamilies;
		for (const auto& name : columns)
			columnFamilies.push_back(rocksdb::ColumnFamilyDescriptor(name, defaultColumnOptions));

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

	RdbTestContext::RdbTestContext(const cache::RocksDatabaseSettings& settings, const DbSeeder& seeder)
			: DbInitializer(settings.ColumnFamilyNames, seeder)
			, m_database(settings)
	{}

	cache::RocksDatabase& RdbTestContext::database() {
		return m_database;
	}

	void AssertIteratorValue(const std::string& value, const cache::RdbDataIterator& iter) {
		ASSERT_NE(cache::RdbDataIterator::End(), iter);
		EXPECT_EQ(value.size(), iter.storage().size());
		EXPECT_EQ(value, std::string(iter.storage().data(), iter.storage().size()));
	}
}}
