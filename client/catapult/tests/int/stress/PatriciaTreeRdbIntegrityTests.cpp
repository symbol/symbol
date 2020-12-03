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

#include "catapult/cache_db/PatriciaTreeRdbDataSource.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/tree/PatriciaTreeTests.h"

namespace catapult { namespace tree {

#define TEST_CLASS PatriciaTreeRdbIntegrityTests

	namespace {
		auto DefaultSettings(const std::string& dbDir) {
			return cache::RocksDatabaseSettings(dbDir, { "default" }, cache::FilterPruningMode::Disabled);
		}

		class RocksPatriciaTreeTraits {
		public:
			using DataSourceType = cache::PatriciaTreeRdbDataSource;

		public:
			explicit RocksPatriciaTreeTraits(tree::DataSourceVerbosity)
					: m_db(DefaultSettings(m_dbDirGuard.name()))
					, m_container(m_db, 0)
					, m_dataSource(m_container)
			{}

		public:
			DataSourceType& dataSource() {
				return m_dataSource;
			}

			void verifyDataSourceSize(size_t) const {
				// note: cannot verify size because setSize is not done anywhere
			}

		private:
			// use TempDirectoryGuard to remove db directory without including rocksdb related includes
			test::TempDirectoryGuard m_dbDirGuard;
			cache::RocksDatabase m_db;
			cache::PatriciaTreeContainer m_container;
			DataSourceType m_dataSource;
		};
	}

	DEFINE_PATRICIA_TREE_TESTS(RocksPatriciaTreeTraits)
}}
