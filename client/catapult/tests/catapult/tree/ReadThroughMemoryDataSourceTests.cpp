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

#include "catapult/tree/ReadThroughMemoryDataSource.h"
#include "catapult/utils/ArraySet.h"
#include "tests/test/tree/PatriciaTreeDataSourceTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace tree {

#define TEST_CLASS ReadThroughMemoryDataSourceTests

	// region basic tests

	namespace {
		struct DataSourceTraits {
			class DataSourceType : public ReadThroughMemoryDataSource<MemoryDataSource> {
			public:
				DataSourceType() : ReadThroughMemoryDataSource<MemoryDataSource>(m_backingDataSource)
				{}

			private:
				MemoryDataSource m_backingDataSource;
			};
		};
	}

	DEFINE_PATRICIA_TREE_DATA_SOURCE_TESTS(DataSourceTraits)

	TEST(TEST_CLASS, CanVisitAllSavedNodesViaForEach) {
		// Arrange: prepare four nodes
		auto leafNode1 = LeafTreeNode(TreeNodePath(0x64'6F'67'00), test::GenerateRandomByteArray<Hash256>());
		auto leafNode2 = LeafTreeNode(TreeNodePath(0x64'6F'67'02), test::GenerateRandomByteArray<Hash256>());
		auto branchNode1 = BranchTreeNode(TreeNodePath(0x64'6F'67'01));
		auto branchNode2 = BranchTreeNode(TreeNodePath(0x64'6F'67'04));

		// Arrange: set a leaf and branch node on the backing data source
		MemoryDataSource backingDataSource;
		backingDataSource.set(leafNode1);
		backingDataSource.set(branchNode1);

		// - set a leaf and branch node on the wrapping data source
		ReadThroughMemoryDataSource<MemoryDataSource> dataSource(backingDataSource);
		dataSource.set(leafNode2);
		dataSource.set(branchNode2);

		// Act:
		auto numVisits = 0u;
		utils::HashSet visitedHashes;
		dataSource.forEach([&numVisits, &visitedHashes](const auto& node) {
			visitedHashes.emplace(node.hash());
			++numVisits;
		});

		// Assert: only in memory nodes are accessible
		EXPECT_EQ(2u, numVisits);
		EXPECT_EQ(2u, visitedHashes.size());
		EXPECT_EQ(utils::HashSet({ leafNode2.hash(), branchNode2.hash() }), visitedHashes);
	}

	TEST(TEST_CLASS, CanClearAllMemoryNodes) {
		// Arrange: set a leaf and branch node on the backing data source
		MemoryDataSource backingDataSource;
		backingDataSource.set(LeafTreeNode(TreeNodePath(0x84'6F'67'00), test::GenerateRandomByteArray<Hash256>()));
		backingDataSource.set(BranchTreeNode(TreeNodePath(0x84'6F'67'01)));

		// - set a leaf and branch node on the wrapping data source
		ReadThroughMemoryDataSource<MemoryDataSource> dataSource(backingDataSource);
		dataSource.set(LeafTreeNode(TreeNodePath(0x64'6F'67'00), test::GenerateRandomByteArray<Hash256>()));
		dataSource.set(BranchTreeNode(TreeNodePath(0x64'6F'67'01)));

		// Sanity:
		EXPECT_EQ(2u, backingDataSource.size());
		EXPECT_EQ(2u, dataSource.size());

		// Act:
		dataSource.clear();

		// Assert: only the memory nodes are cleared
		EXPECT_EQ(2u, backingDataSource.size());
		EXPECT_EQ(0u, dataSource.size());
	}

	// endregion

	// region layering

	namespace {
		template<typename TNode>
		void AssertSetDoesNotModifyBackingSet(const TNode& node) {
			// Arrange:
			MemoryDataSource backingDataSource;
			ReadThroughMemoryDataSource<MemoryDataSource> dataSource(backingDataSource);

			// Act:
			dataSource.set(node);

			// Assert:
			EXPECT_EQ(0u, backingDataSource.size());
			EXPECT_EQ(1u, dataSource.size());
		}
	}

	TEST(TEST_CLASS, SetLeafNodeDoesNotModifyBackingSet) {
		AssertSetDoesNotModifyBackingSet(LeafTreeNode(TreeNodePath(0x64'6F'67'00), test::GenerateRandomByteArray<Hash256>()));
	}

	TEST(TEST_CLASS, SetBranchNodeDoesNotModifyBackingSet) {
		AssertSetDoesNotModifyBackingSet(BranchTreeNode(TreeNodePath(0x64'6F'67'00)));
	}

	TEST(TEST_CLASS, GetFallsBackToBackingDataSource) {
		// Arrange:
		MemoryDataSource backingDataSource;
		ReadThroughMemoryDataSource<MemoryDataSource> dataSource(backingDataSource);

		auto node = LeafTreeNode(TreeNodePath(0x64'6F'67'00), test::GenerateRandomByteArray<Hash256>());
		backingDataSource.set(node);

		// Act:
		auto dataSourceNode = dataSource.get(node.hash());

		// Assert:
		EXPECT_TRUE(dataSourceNode.isLeaf());
		EXPECT_EQ(node.hash(), dataSourceNode.hash());

		// Sanity:
		EXPECT_EQ(1u, backingDataSource.size());
		EXPECT_EQ(0u, dataSource.size());
	}

	// endregion
}}

