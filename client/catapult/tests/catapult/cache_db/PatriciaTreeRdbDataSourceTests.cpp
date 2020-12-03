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
#include "tests/test/tree/PatriciaTreeDataSourceTests.h"

namespace catapult { namespace cache {

#define TEST_CLASS PatriciaTreeRdbDataSourceTests

	namespace {
		auto DefaultSettings(const std::string& dbName) {
			// use 0 size to force flush after every write
			return RocksDatabaseSettings(dbName, { "default" }, FilterPruningMode::Disabled);
		}

		class RocksDataSourceWrapper {
		public:
			RocksDataSourceWrapper()
					: m_db(DefaultSettings(m_dbDirGuard.name()))
					, m_container(m_db, 0)
					, m_dataSource(m_container) {
				m_container.setSize(0);
			}

		public:
			size_t size() {
				return m_dataSource.size();
			}

			tree::TreeNode get(const Hash256& hash) {
				return m_dataSource.get(hash);
			}

			void set(const tree::BranchTreeNode& node) {
				m_dataSource.set(node);
				m_container.setSize(size() + 1);
			}

			void set(const tree::LeafTreeNode& node) {
				m_dataSource.set(node);
				m_container.setSize(size() + 1);
			}

		private:
			test::TempDirectoryGuard m_dbDirGuard;
			RocksDatabase m_db;
			PatriciaTreeContainer m_container;
			PatriciaTreeRdbDataSource m_dataSource;
		};

		struct RocksDataSourceTraits {
			using DataSourceType = RocksDataSourceWrapper;
		};
	}

	DEFINE_PATRICIA_TREE_DATA_SOURCE_TESTS(RocksDataSourceTraits)
}}
