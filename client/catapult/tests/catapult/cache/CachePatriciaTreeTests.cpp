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

#include "catapult/cache/CachePatriciaTree.h"
#include "catapult/cache_db/RocksInclude.h"
#include "catapult/tree/BasePatriciaTree.h"
#include "tests/catapult/cache/test/PatriciaTreeTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CachePatriciaTreeTests

	namespace {
		using DatabaseBasePatriciaTree = tree::BasePatriciaTree<
			SerializerPlainKeyEncoder<test::MemoryPatriciaTreeSimpleSerializer>,
			PatriciaTreeRdbDataSource>;
	}

	// region disabled

	TEST(TEST_CLASS, Disabled_GetReturnsNullptr) {
		// Arrange:
		CacheDatabase database;
		CachePatriciaTree<DatabaseBasePatriciaTree> tree(false, database, 1);

		// Act + Assert:
		EXPECT_FALSE(!!tree.get());
	}

	TEST(TEST_CLASS, Disabled_RebaseReturnsNullptr) {
		// Arrange:
		CacheDatabase database;
		CachePatriciaTree<DatabaseBasePatriciaTree> tree(false, database, 1);

		// Act:
		auto pDeltaTree = tree.rebase();

		// Act + Assert:
		EXPECT_FALSE(!!pDeltaTree);
	}

	TEST(TEST_CLASS, Disabled_RebaseDetachedReturnsNullptr) {
		// Arrange:
		CacheDatabase database;
		CachePatriciaTree<DatabaseBasePatriciaTree> tree(false, database, 1);

		// Act:
		auto pDeltaTree = tree.rebaseDetached();

		// Act + Assert:
		EXPECT_FALSE(!!pDeltaTree);
	}

	TEST(TEST_CLASS, Disabled_CommitHasNoEffect) {
		// Arrange:
		CacheDatabase database;
		CachePatriciaTree<DatabaseBasePatriciaTree> tree(false, database, 1);

		// Act + Assert: no exception
		tree.commit();
	}

	// endregion

	// region enabled

	namespace {
		class CacheDatabaseHolder {
		public:
			CacheDatabaseHolder()
					: m_database(CacheDatabaseSettings(m_dbDirGuard.name(), { "default", "patricia_tree" }, FilterPruningMode::Disabled))
			{}

		public:
			CacheDatabase& database() {
				return m_database;
			}

		private:
			test::TempDirectoryGuard m_dbDirGuard;
			CacheDatabase m_database;
		};
	}

	TEST(TEST_CLASS, Enabled_GetReturnsValidPointer) {
		// Arrange:
		CacheDatabaseHolder holder;
		CachePatriciaTree<DatabaseBasePatriciaTree> tree(true, holder.database(), 1);

		// Act + Assert:
		EXPECT_TRUE(!!tree.get());
	}

	TEST(TEST_CLASS, Enabled_RebaseReturnsValidPointer) {
		// Arrange:
		CacheDatabaseHolder holder;
		CachePatriciaTree<DatabaseBasePatriciaTree> tree(true, holder.database(), 1);

		// Act:
		auto pDeltaTree = tree.rebase();

		// Act + Assert:
		EXPECT_TRUE(!!pDeltaTree);
	}

	TEST(TEST_CLASS, Enabled_RebaseDetachedReturnsValidPointer) {
		// Arrange:
		CacheDatabaseHolder holder;
		CachePatriciaTree<DatabaseBasePatriciaTree> tree(true, holder.database(), 1);

		// Act:
		auto pDeltaTree = tree.rebaseDetached();

		// Act + Assert:
		EXPECT_TRUE(!!pDeltaTree);
	}

	TEST(TEST_CLASS, Enabled_CommitHasEffect) {
		// Arrange:
		CacheDatabaseHolder holder;
		CachePatriciaTree<DatabaseBasePatriciaTree> tree(true, holder.database(), 1);

		// Act + Assert: exception because no outstanding delta
		EXPECT_THROW(tree.commit(), catapult_runtime_error);
	}

	// endregion

	// region enabled - root hash

	namespace {
		std::string HashToString(const Hash256& hash) {
			return std::string(reinterpret_cast<const char*>(hash.data()), hash.size());
		}
	}

	TEST(TEST_CLASS, Enabled_CanInitializeWithNoRootHashInDb) {
		// Arrange:
		CacheDatabaseHolder holder;

		// Act:
		CachePatriciaTree<DatabaseBasePatriciaTree> tree(true, holder.database(), 1);

		// Assert:
		ASSERT_TRUE(!!tree.get());
		EXPECT_EQ(Hash256(), tree.get()->root());
	}

	TEST(TEST_CLASS, Enabled_CanInitializeWithEmptyTreeInDb) {
		// Arrange: empty tree means stored root hash == 0
		CacheDatabaseHolder holder;
		holder.database().put(1, "root", HashToString(Hash256()));

		// Act:
		CachePatriciaTree<DatabaseBasePatriciaTree> tree(true, holder.database(), 1);

		// Assert:
		ASSERT_TRUE(!!tree.get());
		EXPECT_EQ(Hash256(), tree.get()->root());
	}

	TEST(TEST_CLASS, Enabled_CanInitializeWithRootHashInDb) {
		// Arrange:
		CacheDatabaseHolder holder;

		tree::LeafTreeNode leafNode(tree::TreeNodePath(0x01'23'4A'B6'78), test::GenerateRandomByteArray<Hash256>());
		auto rootHash = leafNode.hash();
		auto serializedLeafNode = tree::PatriciaTreeSerializer::SerializeValue(tree::TreeNode(leafNode));

		holder.database().put(1, "root", HashToString(rootHash));
		holder.database().put(1, HashToString(rootHash), serializedLeafNode);

		// Act:
		CachePatriciaTree<DatabaseBasePatriciaTree> tree(true, holder.database(), 1);

		// Assert:
		ASSERT_TRUE(!!tree.get());
		EXPECT_NE(Hash256(), tree.get()->root());
		EXPECT_EQ(rootHash, tree.get()->root());
	}

	TEST(TEST_CLASS, Enabled_CannotInitializeWithUnknownRootHashInDb) {
		// Arrange:
		CacheDatabaseHolder holder;

		auto rootHash = test::GenerateRandomByteArray<Hash256>();
		holder.database().put(1, "root", HashToString(rootHash));

		// Act + Assert:
		EXPECT_THROW(CachePatriciaTree<DatabaseBasePatriciaTree>(true, holder.database(), 1), catapult_runtime_error);
	}

	namespace {
		template<typename T>
		void ReadValue(RocksDatabase& database, const std::string& key, T& value) {
			RdbDataIterator iter;
			database.get(1, key, iter);
			ASSERT_NE(RdbDataIterator::End(), iter);

			const auto& buffer = iter.buffer();
			ASSERT_EQ(sizeof(T), buffer.Size);
			value = reinterpret_cast<const T&>(*buffer.pData);
		}
	}

	TEST(TEST_CLASS, Enabled_CommitSavesRootHash) {
		// Arrange:
		CacheDatabaseHolder holder;
		CachePatriciaTree<DatabaseBasePatriciaTree> tree(true, holder.database(), 1);

		// Act:
		auto pDeltaTree = tree.rebase();
		pDeltaTree->set(0x01'23'4A'B6, "alpha");
		pDeltaTree->set(0x01'23'4A'99, "beta");
		tree.commit();

		// Assert:
		Hash256 rootHash;
		ReadValue(holder.database(), "root", rootHash);
		EXPECT_EQ(tree.get()->root(), rootHash);
	}

	// endregion
}}
