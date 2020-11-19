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

#include "catapult/tree/MemoryDataSource.h"
#include "catapult/utils/ArraySet.h"
#include "tests/test/tree/PatriciaTreeDataSourceTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace tree {

#define TEST_CLASS MemoryDataSourceTests

	namespace {
		struct DataSourceTraits {
			using DataSourceType = MemoryDataSource;
		};
	}

	DEFINE_PATRICIA_TREE_DATA_SOURCE_TESTS(DataSourceTraits)

	TEST(TEST_CLASS, CanVisitAllSavedNodesViaForEach) {
		// Arrange: prepare four nodes
		auto leafNode1 = LeafTreeNode(TreeNodePath(0x64'6F'67'00), test::GenerateRandomByteArray<Hash256>());
		auto leafNode2 = LeafTreeNode(TreeNodePath(0x64'6F'67'02), test::GenerateRandomByteArray<Hash256>());
		auto branchNode1 = BranchTreeNode(TreeNodePath(0x64'6F'67'01));
		auto branchNode2 = BranchTreeNode(TreeNodePath(0x64'6F'67'04));

		// - add all nodes to the data source
		MemoryDataSource dataSource;
		dataSource.set(leafNode1);
		dataSource.set(leafNode2);
		dataSource.set(branchNode1);
		dataSource.set(branchNode2);

		// Act:
		auto numVisits = 0u;
		utils::HashSet visitedHashes;
		dataSource.forEach([&numVisits, &visitedHashes](const auto& node) {
			visitedHashes.emplace(node.hash());
			++numVisits;
		});

		// Assert:
		EXPECT_EQ(4u, numVisits);
		EXPECT_EQ(4u, visitedHashes.size());
		EXPECT_EQ(utils::HashSet({ leafNode1.hash(), leafNode2.hash(), branchNode1.hash(), branchNode2.hash() }), visitedHashes);
	}

	TEST(TEST_CLASS, CanClearAllNodes) {
		// Arrange: set a leaf and branch node
		MemoryDataSource dataSource;
		dataSource.set(LeafTreeNode(TreeNodePath(0x64'6F'67'00), test::GenerateRandomByteArray<Hash256>()));
		dataSource.set(BranchTreeNode(TreeNodePath(0x64'6F'67'01)));

		// Sanity:
		EXPECT_EQ(2u, dataSource.size());

		// Act:
		dataSource.clear();

		// Assert:
		EXPECT_EQ(0u, dataSource.size());
	}

	// region perf

	TEST(TEST_CLASS, SetDoesNotRecalculateHashWhenNotDirty) {
		// Arrange: non-deterministic because test is dependent on elapsed time
		test::RunNonDeterministicTest("SetDoesNotRecalculateHashWhenNotDirty", []() {
			// Arrange:
			MemoryDataSource dataSource;

			// - use branch tree node because leaf tree node is immutable and calculates hash in constructor
			auto links = test::GenerateRandomDataVector<Hash256>(3);
			auto node = BranchTreeNode(TreeNodePath(0x64'6F'67'00));
			node.setLink(links[0], 6);
			node.setLink(links[1], 10);
			node.setLink(links[2], 12);

			// Act:
			auto time1 = test::GetCurrentTimeNanoseconds();
			auto hash1 = node.hash();
			auto time2 = test::GetCurrentTimeNanoseconds();
			dataSource.set(node);
			auto time3 = test::GetCurrentTimeNanoseconds();

			// Assert:
			auto savedNode = dataSource.get(hash1);
			EXPECT_FALSE(savedNode.empty());
			if (savedNode.empty())
				return true;

			EXPECT_EQ(hash1, savedNode.hash());

			// - only the first hash should recalculate the hash, so assert (conservatively) it is at least 1.5x slower
			auto elaspedHashTime = (time2 - time1).count();
			auto elaspedSetTime = (time3 - time2).count();
			CATAPULT_LOG(debug) << "elaspedHashTime = " << elaspedHashTime << ", elaspedSetTime = " << elaspedSetTime;

			if (elaspedHashTime <= elaspedSetTime * 3 / 2)
				return false;

			EXPECT_GT(elaspedHashTime, elaspedSetTime * 3 / 2);
			return true;
		});
	}

	// endregion
}}

