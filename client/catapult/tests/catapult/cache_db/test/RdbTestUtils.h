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

#pragma once
#include "catapult/cache_db/RocksDatabase.h"
#include "catapult/functions.h"
#include "tests/test/nodeps/Filesystem.h"
#include <string>
#include <vector>

namespace catapult { namespace test {

	/// Names of columns.
	using ColumnNames = std::vector<std::string>;

	/// Db column handles.
	using ColumnHandles = std::vector<rocksdb::ColumnFamilyHandle*>;

	/// Seed callback that fills db with data.
	using DbSeeder = consumer<rocksdb::DB&, const ColumnHandles&>;

	/// Converts even \a key used in even db seeder to db value.
	std::string EvenKeyToValue(size_t key);

	/// Creates seeder that creates \a numKeys with even keys.
	DbSeeder CreateEvenDbSeeder(size_t numKeys);

	/// Db initialization helper that destroys and seeds db.
	class DbInitializer {
	public:
		/// Creates db in default directory with \a columns and seeds using \a seeder.
		DbInitializer(const ColumnNames& columns, const DbSeeder& seeder);

		/// Creates db with \a compactionFilter in default directory with \a columns and seeds using \a seeder.
		DbInitializer(const ColumnNames& columns, const DbSeeder& seeder, const rocksdb::CompactionFilter* compactionFilter);

	private:
		bool seedDb(
				const std::string& dbDir,
				const std::vector<std::string>& columns,
				const DbSeeder& seeder,
				const rocksdb::CompactionFilter* compactionFilter);

	private:
		test::TempDirectoryGuard m_dbDirGuard;
	};

	/// Db test context.
	class RdbTestContext : public DbInitializer {
	public:
		/// Creates context around \a settings with optional \a seeder.
		explicit RdbTestContext(const cache::RocksDatabaseSettings& settings, const DbSeeder& seeder = DbSeeder());

	public:
		/// Gets a reference to the database.
		cache::RocksDatabase& database();

	private:
		cache::RocksDatabase m_database;
	};

	/// Asserts that value stored in iterator (\a iter) matches \a value.
	void AssertIteratorValue(const std::string& value, const cache::RdbDataIterator& iter);
}}
