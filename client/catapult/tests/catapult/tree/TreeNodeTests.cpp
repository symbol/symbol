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

#include "catapult/tree/TreeNode.h"
#include "catapult/crypto/Hashes.h"
#include "tests/TestHarness.h"

namespace catapult { namespace tree {

#define TEST_CLASS TreeNodeTests

	// region LeafTreeNode

	namespace {
		Hash256 CalculateLeafNodeHash(const std::vector<uint8_t>& path, const Hash256& value) {
			Hash256 hash;
			crypto::Sha3_256_Builder builder;
			builder.update(path);
			builder.update(value);
			builder.final(hash);
			return hash;
		}
	}

	TEST(TEST_CLASS, CanCreateLeafNodeWithEmptyPath) {
		// Act:
		auto path = TreeNodePath();
		auto value = test::GenerateRandomData<Hash256_Size>();
		auto node = LeafTreeNode(path, value);

		// Assert:
		EXPECT_EQ(path, node.path());
		EXPECT_EQ(value, node.value());

		auto expectedHash = CalculateLeafNodeHash({ 0x20 }, value);
		EXPECT_EQ(expectedHash, node.hash());
	}

	TEST(TEST_CLASS, CanCreateLeafNodeWithEvenPath) {
		// Act:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto value = test::GenerateRandomData<Hash256_Size>();
		auto node = LeafTreeNode(path, value);

		// Assert:
		EXPECT_EQ(path, node.path());
		EXPECT_EQ(value, node.value());

		auto expectedHash = CalculateLeafNodeHash({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, value);
		EXPECT_EQ(expectedHash, node.hash());
	}

	TEST(TEST_CLASS, CanCreateLeafNodeWithOddPath) {
		// Act:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto value = test::GenerateRandomData<Hash256_Size>();
		auto node = LeafTreeNode(path.subpath(0, 5), value);

		// Assert:
		EXPECT_EQ(path.subpath(0, 5), node.path());
		EXPECT_EQ(value, node.value());

		auto expectedHash = CalculateLeafNodeHash({ 0x36, 0x46, 0xF6 }, value);
		EXPECT_EQ(expectedHash, node.hash());
	}

	// endregion

	// region BranchTreeNode

	namespace {
		void AddEmptyHashes(crypto::Sha3_256_Builder& builder, size_t count) {
			for (auto i = 0u; i < count; ++i)
				builder.update(Hash256());
		}

		Hash256 CalculateEmptyBranchNodeHash(const std::vector<uint8_t>& path) {
			Hash256 hash;
			crypto::Sha3_256_Builder builder;
			builder.update(path);
			AddEmptyHashes(builder, 16);
			builder.final(hash);
			return hash;
		}

		void AssertLink(const BranchTreeNode& node, size_t index, const Hash256& expectedLink) {
			EXPECT_TRUE(node.hasLink(index)) << "link at " << index;
			EXPECT_EQ(expectedLink, node.link(index)) << "link at " << index;
		}

		void AssertEmptyLinks(const BranchTreeNode& node, size_t start, size_t end) {
			for (auto i = start; i <= end; ++i) {
				EXPECT_FALSE(node.hasLink(i)) << "link at " << i;
				EXPECT_EQ(Hash256(), node.link(i)) << "link at " << i;
			}
		}

		void AssertNoLinks(const BranchTreeNode& node) {
			EXPECT_EQ(0u, node.numLinks());
			AssertEmptyLinks(node, 0, 15);
		}
	}

	TEST(TEST_CLASS, CanCreateBranchTreeNodeWithEmptyPath) {
		// Act:
		auto path = TreeNodePath();
		auto node = BranchTreeNode(path);

		// Assert:
		EXPECT_EQ(path, node.path());
		AssertNoLinks(node);

		auto expectedHash = CalculateEmptyBranchNodeHash({ 0x00 });
		EXPECT_EQ(expectedHash, node.hash());
	}

	TEST(TEST_CLASS, CanCreateBranchTreeNodeWithEvenPath) {
		// Act:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto node = BranchTreeNode(path);

		// Assert:
		EXPECT_EQ(path, node.path());
		AssertNoLinks(node);

		auto expectedHash = CalculateEmptyBranchNodeHash({ 0x00, 0x64, 0x6F, 0x67, 0x00 });
		EXPECT_EQ(expectedHash, node.hash());
	}

	TEST(TEST_CLASS, CanCreateBranchTreeNodeWithOddPath) {
		// Act:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto node = BranchTreeNode(path.subpath(0, 5));

		// Assert:
		EXPECT_EQ(path.subpath(0, 5), node.path());
		AssertNoLinks(node);

		auto expectedHash = CalculateEmptyBranchNodeHash({ 0x16, 0x46, 0xF6 });
		EXPECT_EQ(expectedHash, node.hash());
	}

	TEST(TEST_CLASS, CanChangeBranchTreeNodePath) {
		// Arrange:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto node = BranchTreeNode(path);

		// Act:
		node.setPath(path.subpath(2, 3));

		// Assert:
		EXPECT_EQ(path.subpath(2, 3), node.path());
		AssertNoLinks(node);

		auto expectedHash = CalculateEmptyBranchNodeHash({ 0x16, 0xF6 });
		EXPECT_EQ(expectedHash, node.hash());
	}

	TEST(TEST_CLASS, BranchTreeNodeSetPathTriggersHashRecalculation) {
		// Arrange:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto node = BranchTreeNode(path);
		auto hash1 = node.hash();

		// Act:
		node.setPath(path.subpath(2, 3));
		auto hash2 = node.hash();

		// Assert:
		EXPECT_NE(hash1, hash2);
	}

	namespace {
		// these helpers assume two links at indexes 6 and 11
		void AssertTwoLinks(const BranchTreeNode& node, const Hash256& link1, const Hash256& link2) {
			// Assert:
			EXPECT_EQ(2u, node.numLinks());
			AssertEmptyLinks(node, 0, 5);
			AssertLink(node, 6, link1);
			AssertEmptyLinks(node, 7, 10);
			AssertLink(node, 11, link2);
			AssertEmptyLinks(node, 12, 15);
		}

		Hash256 CalculateTwoLinkHash(const std::vector<uint8_t>& path, const Hash256& link1, const Hash256& link2) {
			Hash256 expectedHash;
			crypto::Sha3_256_Builder builder;
			builder.update(path);
			AddEmptyHashes(builder, 6);
			builder.update(link1);
			AddEmptyHashes(builder, 4);
			builder.update(link2);
			AddEmptyHashes(builder, 4);
			builder.final(expectedHash);
			return expectedHash;
		}
	}

	TEST(TEST_CLASS, CanSetBranchTreeNodeLinks) {
		// Arrange:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto links = test::GenerateRandomDataVector<Hash256>(2);
		auto node = BranchTreeNode(path);

		// Act:
		node.setLink(links[0], 6);
		node.setLink(links[1], 11);

		// Assert:
		EXPECT_EQ(path, node.path());
		AssertTwoLinks(node, links[0], links[1]);

		auto expectedHash = CalculateTwoLinkHash({ 0x00, 0x64, 0x6F, 0x67, 0x00 }, links[0], links[1]);
		EXPECT_EQ(expectedHash, node.hash());
	}

	TEST(TEST_CLASS, BranchTreeNodeSetLinkTriggersHashRecalculation) {
		// Arrange:
		auto links = test::GenerateRandomDataVector<Hash256>(2);
		auto node = BranchTreeNode(TreeNodePath(0x64'6F'67'00));
		node.setLink(links[0], 6);
		auto hash1 = node.hash();

		// Act:
		node.setLink(links[1], 8);
		auto hash2 = node.hash();

		// Assert:
		EXPECT_NE(hash1, hash2);
	}

	TEST(TEST_CLASS, CanClearBranchTreeNodeLinks) {
		// Arrange:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto links = test::GenerateRandomDataVector<Hash256>(4);
		auto node = BranchTreeNode(path);
		node.setLink(links[0], 6);
		node.setLink(links[1], 8);
		node.setLink(links[2], 11);
		node.setLink(links[3], 14);

		// Act:
		node.clearLink(8);
		node.clearLink(14);

		// Assert:
		EXPECT_EQ(path, node.path());
		AssertTwoLinks(node, links[0], links[2]);

		auto expectedHash = CalculateTwoLinkHash({ 0x00, 0x64, 0x6F, 0x67, 0x00 }, links[0], links[2]);
		EXPECT_EQ(expectedHash, node.hash());
	}

	TEST(TEST_CLASS, BranchTreeNodeClearLinkTriggersHashRecalculation) {
		// Arrange:
		auto links = test::GenerateRandomDataVector<Hash256>(3);
		auto node = BranchTreeNode(TreeNodePath(0x64'6F'67'00));
		node.setLink(links[0], 6);
		node.setLink(links[1], 10);
		node.setLink(links[2], 12);
		auto hash1 = node.hash();

		// Act:
		node.clearLink(10);
		auto hash2 = node.hash();

		// Assert:
		EXPECT_NE(hash1, hash2);
	}

	namespace {
		BranchTreeNode CreateBranchTreeNodeWithLinks(std::initializer_list<size_t> linkIndexes) {
			auto node = BranchTreeNode(TreeNodePath(0x64'6F'67'00));

			for (auto index : linkIndexes)
				node.setLink(test::GenerateRandomData<Hash256_Size>(), index);

			return node;
		}

		void AssertHighestLinkIndex(uint8_t expectedHighestLinkIndex, std::initializer_list<size_t> linkIndexes) {
			// Arrange:
			auto node = CreateBranchTreeNodeWithLinks(linkIndexes);

			// Act:
			auto highestLinkIndex = node.highestLinkIndex();

			// Assert:
			EXPECT_EQ(expectedHighestLinkIndex, highestLinkIndex);
		}
	}

	TEST(TEST_CLASS, BranchTreeNodeHighestLinkIndexReturnsCorrectValueWhenNoLinksAreSet) {
		// Assert:
		AssertHighestLinkIndex(0xFF, {});
	}

	TEST(TEST_CLASS, BranchTreeNodeHighestLinkIndexReturnsCorrectValueWhenSingleLinkIsSet) {
		// Assert:
		AssertHighestLinkIndex(0, { 0 });
		AssertHighestLinkIndex(7, { 7 });
		AssertHighestLinkIndex(15, { 15 });
	}

	TEST(TEST_CLASS, BranchTreeNodeHighestLinkIndexReturnsCorrectValueWhenMultipleLinksAreSet) {
		// Assert:
		AssertHighestLinkIndex(4, { 1, 2, 3, 4 });
		AssertHighestLinkIndex(11, { 9, 11, 8 });
		AssertHighestLinkIndex(15, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 });
	}

	namespace {
		auto GetCurrentTime() {
			auto time = std::chrono::steady_clock::now().time_since_epoch();
			return std::chrono::duration_cast<std::chrono::nanoseconds>(time);
		}
	}

	TEST(TEST_CLASS, BranchTreeNodeDoesNotRecalculateHashWhenNotDirty) {
		// Arrange: non-deterministic because test is dependent on elapsed time
		test::RunNonDeterministicTest("BranchTreeNodeDoesNotRecalculateHashWhenNotDirty", []() {
			// Arrange:
			auto links = test::GenerateRandomDataVector<Hash256>(3);
			auto node = BranchTreeNode(TreeNodePath(0x64'6F'67'00));
			node.setLink(links[0], 6);
			node.setLink(links[1], 10);
			node.setLink(links[2], 12);

			// Act:
			auto time1 = GetCurrentTime();
			auto hash1 = node.hash();
			auto time2 = GetCurrentTime();
			auto hash2 = node.hash();
			auto time3 = GetCurrentTime();

			// Assert:
			EXPECT_EQ(hash1, hash2);

			// - only the first hash should recalculate the hash, so assert (conservatively) it is at least 10x slower
			auto elaspedHashOneTime = (time2 - time1).count();
			auto elaspedHashTwoTime = (time3 - time2).count();
			CATAPULT_LOG(debug) << "elaspedHashOneTime = " << elaspedHashOneTime << ", elaspedHashTwoTime = " << elaspedHashTwoTime;

			if (elaspedHashOneTime <= elaspedHashTwoTime * 10)
				return false;

			EXPECT_GT(elaspedHashOneTime, elaspedHashTwoTime * 10);
			return true;
		});
	}

	// endregion

	// region TreeNode - constructor + copy

	namespace {
		void AssertEmptyTreeNode(const TreeNode& node) {
			// Assert:
			EXPECT_TRUE(node.empty());
			EXPECT_FALSE(node.isLeaf());
			EXPECT_FALSE(node.isBranch());

			EXPECT_THROW(node.asLeafNode(), catapult_runtime_error);
			EXPECT_THROW(node.asBranchNode(), catapult_runtime_error);

			EXPECT_EQ(TreeNodePath(), node.path());
			EXPECT_EQ(Hash256(), node.hash());
		}
	}

	TEST(TEST_CLASS, CanCreateEmptyTreeNode) {
		// Act:
		TreeNode node;

		// Assert:
		AssertEmptyTreeNode(node);
	}

	TEST(TEST_CLASS, CanCopyEmptyTreeNode) {
		// Act:
		TreeNode node;
		auto copy = node.copy();

		// Assert:
		AssertEmptyTreeNode(copy);
	}

	namespace {
		void AssertLeafTreeNode(const TreeNode& node, const TreeNodePath& expectedPath, const Hash256& expectedHash) {
			// Assert:
			EXPECT_FALSE(node.empty());
			EXPECT_TRUE(node.isLeaf());
			EXPECT_FALSE(node.isBranch());

			EXPECT_NO_THROW(node.asLeafNode());
			EXPECT_THROW(node.asBranchNode(), catapult_runtime_error);

			EXPECT_EQ(expectedPath, node.path());
			EXPECT_EQ(expectedHash, node.hash());
		}
	}

	TEST(TEST_CLASS, CanCreateLeafBasedTreeNode) {
		// Arrange:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto value = test::GenerateRandomData<Hash256_Size>();
		auto leafNode = LeafTreeNode(path, value);

		// Act:
		TreeNode node(leafNode);

		// Assert:
		auto expectedHash = CalculateLeafNodeHash({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, value);
		AssertLeafTreeNode(node, path, expectedHash);
	}

	TEST(TEST_CLASS, CanCopyLeafBasedTreeNode) {
		// Arrange:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto value = test::GenerateRandomData<Hash256_Size>();
		auto leafNode = LeafTreeNode(path, value);

		// Act:
		TreeNode node(leafNode);
		auto copy = node.copy();

		// Assert:
		auto expectedHash = CalculateLeafNodeHash({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, value);
		AssertLeafTreeNode(copy, path, expectedHash);
	}

	namespace {
		void AssertBranchTreeNode(const TreeNode& node, const TreeNodePath& expectedPath, const Hash256& expectedHash) {
			// Assert:
			EXPECT_FALSE(node.empty());
			EXPECT_FALSE(node.isLeaf());
			EXPECT_TRUE(node.isBranch());

			EXPECT_THROW(node.asLeafNode(), catapult_runtime_error);
			EXPECT_NO_THROW(node.asBranchNode());

			EXPECT_EQ(expectedPath, node.path());
			EXPECT_EQ(expectedHash, node.hash());
		}
	}

	TEST(TEST_CLASS, CanCreateBranchBasedTreeNode) {
		// Arrange:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto link1 = test::GenerateRandomData<Hash256_Size>();
		auto link2 = test::GenerateRandomData<Hash256_Size>();

		auto branchNode = BranchTreeNode(path);
		branchNode.setLink(link1, 6);
		branchNode.setLink(link2, 11);

		// Act:
		TreeNode node(branchNode);

		// Assert:
		auto expectedHash = CalculateTwoLinkHash({ 0x00, 0x64, 0x6F, 0x67, 0x00 }, link1, link2);
		AssertBranchTreeNode(node, path, expectedHash);
	}

	TEST(TEST_CLASS, CanCopyBranchBasedTreeNode) {
		// Arrange:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto link1 = test::GenerateRandomData<Hash256_Size>();
		auto link2 = test::GenerateRandomData<Hash256_Size>();

		auto branchNode = BranchTreeNode(path);
		branchNode.setLink(link1, 6);
		branchNode.setLink(link2, 11);

		// Act:
		TreeNode node(branchNode);
		auto copy = node.copy();

		// Assert:
		auto expectedHash = CalculateTwoLinkHash({ 0x00, 0x64, 0x6F, 0x67, 0x00 }, link1, link2);
		AssertBranchTreeNode(copy, path, expectedHash);
	}

	// endregion

	// region TreeNode - setPath

	TEST(TEST_CLASS, CannotChangeEmptyTreeNodePathViaTreeNode) {
		// Arrange:
		TreeNode node;

		// Act + Assert:
		auto newPath = TreeNodePath(0x11'22'33'98);
		EXPECT_THROW(node.setPath(newPath), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanChangeLeafTreeNodePathViaTreeNode) {
		// Arrange:
		auto value = test::GenerateRandomData<Hash256_Size>();
		auto leafNode = LeafTreeNode(TreeNodePath(0x64'6F'67'00), value);
		TreeNode node(leafNode);

		// Act:
		auto newPath = TreeNodePath(0x11'22'33'98);
		node.setPath(newPath);

		// Assert:
		EXPECT_EQ(newPath, node.path());

		auto expectedHash = CalculateLeafNodeHash({ 0x20, 0x11, 0x22, 0x33, 0x98 }, value);
		EXPECT_EQ(expectedHash, node.hash());
	}

	TEST(TEST_CLASS, CanChangeBranchTreeNodePathViaTreeNode) {
		// Arrange:
		auto link1 = test::GenerateRandomData<Hash256_Size>();
		auto link2 = test::GenerateRandomData<Hash256_Size>();
		auto branchNode = BranchTreeNode(TreeNodePath(0x64'6F'67'00));
		branchNode.setLink(link1, 6);
		branchNode.setLink(link2, 11);
		TreeNode node(branchNode);

		// Act:
		auto newPath = TreeNodePath(0x11'22'33'98);
		node.setPath(newPath);

		// Assert:
		EXPECT_EQ(newPath, node.path());

		auto expectedHash = CalculateTwoLinkHash({ 0x00, 0x11, 0x22, 0x33, 0x98 }, link1, link2);
		EXPECT_EQ(expectedHash, node.hash());
	}

	// endregion
}}
