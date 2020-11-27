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

#include "catapult/tree/TreeNode.h"
#include "catapult/crypto/Hashes.h"
#include "tests/TestHarness.h"

namespace catapult { namespace tree {

#define TEST_CLASS TreeNodeTests

	// region TreeNodePath utils

	namespace {
		struct TreeNodePathTestCase {
			TreeNodePath Path;
			std::vector<uint8_t> RawPath; // includes leaf flag
		};

		enum class PathType { Short, Long, Ending_On_Boundary };

		TreeNodePathTestCase GetEvenTreeNodePathTestCase(PathType type) {
			switch (type) {
			case PathType::Short:
				return { TreeNodePath(0x64'6F'67'00), { 0x20, 0x64, 0x6F, 0x67, 0x00 } };

			case PathType::Long:
				return {
					TreeNodePath(std::vector<uint8_t>{
						0x64, 0x6F, 0x67, 0x00, 0x19, 0xA3, 0xBC, 0x74,
						0x24, 0x82, 0x3C, 0x98, 0x79, 0x25, 0xD4, 0x60,
						0xFC, 0xEE
					}),
					{
						0x20, 0x64, 0x6F, 0x67, 0x00, 0x19, 0xA3, 0xBC,
						0x74, 0x24, 0x82, 0x3C, 0x98, 0x79, 0x25, 0xD4,
						0x60, 0xFC, 0xEE
					}
				};

			case PathType::Ending_On_Boundary:
				return {
					TreeNodePath(std::vector<uint8_t>{
						0x64, 0x6F, 0x67, 0x00, 0x19, 0xA3, 0xBC, 0x74,
						0x24, 0x82, 0x3C, 0x98, 0x79, 0x25, 0xD4
					}),
					{
						0x20, 0x64, 0x6F, 0x67, 0x00, 0x19, 0xA3, 0xBC,
						0x74, 0x24, 0x82, 0x3C, 0x98, 0x79, 0x25, 0xD4
					}
				};
			}

			CATAPULT_THROW_INVALID_ARGUMENT_1("unknown path type was requested", static_cast<int>(type));
		}

		TreeNodePathTestCase GetOddTreeNodePathTestCase(PathType type) {
			switch (type) {
			case PathType::Short:
				return { TreeNodePath(0x64'6F'67'00).subpath(0, 5), { 0x36, 0x46, 0xF6 } };

			case PathType::Long:
				return {
					TreeNodePath(std::vector<uint8_t>{
						0x64, 0x6F, 0x67, 0x00, 0x19, 0xA3, 0xBC, 0x74,
						0x24, 0x82, 0x3C, 0x98, 0x79, 0x25, 0xD4, 0x60,
						0xFC, 0xEE
					}).subpath(0, 35),
					{
						0x36, 0x46, 0xF6, 0x70, 0x01, 0x9A, 0x3B, 0xC7,
						0x42, 0x48, 0x23, 0xC9, 0x87, 0x92, 0x5D, 0x46,
						0x0F, 0xCE
					}
				};

			case PathType::Ending_On_Boundary:
				return {
					TreeNodePath(std::vector<uint8_t>{
						0x64, 0x6F, 0x67, 0x00, 0x19, 0xA3, 0xBC, 0x74,
						0x24, 0x82, 0x3C, 0x98, 0x79, 0x25, 0xD4, 0x60
					}).subpath(0, 31),
					{
						0x36, 0x46, 0xF6, 0x70, 0x01, 0x9A, 0x3B, 0xC7,
						0x42, 0x48, 0x23, 0xC9, 0x87, 0x92, 0x5D, 0x46
					}
				};
			}

			CATAPULT_THROW_INVALID_ARGUMENT_1("unknown path type was requested", static_cast<int>(type));
		}
	}

	// endregion

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

		void AssertCanCreateLeafNodeWithPath(const TreeNodePathTestCase& testCase) {
			// Act:
			auto value = test::GenerateRandomByteArray<Hash256>();
			auto node = LeafTreeNode(testCase.Path, value);

			// Assert:
			EXPECT_EQ(testCase.Path, node.path());
			EXPECT_EQ(value, node.value());

			auto expectedHash = CalculateLeafNodeHash(testCase.RawPath, value);
			EXPECT_EQ(expectedHash, node.hash());
		}
	}

	TEST(TEST_CLASS, CanCreateLeafTreeNodeWithEmptyPath) {
		AssertCanCreateLeafNodeWithPath({ TreeNodePath(), { 0x20 } });
	}

	TEST(TEST_CLASS, CanCreateLeafTreeNodeWithEvenPath) {
		AssertCanCreateLeafNodeWithPath(GetEvenTreeNodePathTestCase(PathType::Short));
	}

	TEST(TEST_CLASS, CanCreateLeafTreeNodeWithEvenPath_MultipleWorkingBuffers) {
		AssertCanCreateLeafNodeWithPath(GetEvenTreeNodePathTestCase(PathType::Long));
	}

	TEST(TEST_CLASS, CanCreateLeafTreeNodeWithEvenPath_EndingOnWorkingBufferBoundary) {
		AssertCanCreateLeafNodeWithPath(GetEvenTreeNodePathTestCase(PathType::Ending_On_Boundary));
	}

	TEST(TEST_CLASS, CanCreateLeafTreeNodeWithOddPath) {
		AssertCanCreateLeafNodeWithPath(GetOddTreeNodePathTestCase(PathType::Short));
	}

	TEST(TEST_CLASS, CanCreateLeafTreeNodeWithOddPath_MultipleWorkingBuffers) {
		AssertCanCreateLeafNodeWithPath(GetOddTreeNodePathTestCase(PathType::Long));
	}

	TEST(TEST_CLASS, CanCreateLeafTreeNodeWithOddPath_EndingOnWorkingBufferBoundary) {
		AssertCanCreateLeafNodeWithPath(GetOddTreeNodePathTestCase(PathType::Ending_On_Boundary));
	}

	// endregion

	// region BranchTreeNode - helpers

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

		void AssertHashLink(const BranchTreeNode& node, size_t index, const Hash256& expectedLink) {
			auto message = "link at " + std::to_string(index);
			EXPECT_TRUE(node.hasLink(index)) << message;
			EXPECT_FALSE(node.hasLinkedNode(index)) << message;

			EXPECT_EQ(expectedLink, node.link(index)) << message;
			EXPECT_TRUE(node.linkedNode(index).empty()) << message;
		}

		void AssertNodeLink(const BranchTreeNode& node, size_t index, const Hash256& expectedLink) {
			auto message = "link at " + std::to_string(index);
			EXPECT_TRUE(node.hasLink(index)) << message;
			EXPECT_TRUE(node.hasLinkedNode(index)) << message;

			EXPECT_EQ(expectedLink, node.link(index)) << message;
			EXPECT_FALSE(node.linkedNode(index).empty()) << message;
			EXPECT_EQ(expectedLink, node.linkedNode(index).hash()) << message;
		}

		void AssertEmptyLinks(const BranchTreeNode& node, size_t start, size_t end) {
			for (auto i = start; i <= end; ++i) {
				auto message = "link at " + std::to_string(i);
				EXPECT_FALSE(node.hasLink(i)) << message;
				EXPECT_FALSE(node.hasLinkedNode(i)) << message;

				EXPECT_EQ(Hash256(), node.link(i)) << message;
				EXPECT_TRUE(node.linkedNode(i).empty()) << message;
			}
		}

		void AssertNoLinks(const BranchTreeNode& node) {
			EXPECT_EQ(0u, node.numLinks());
			AssertEmptyLinks(node, 0, 15);
		}
	}

	// endregion

	// region BranchTreeNode - constructor

	namespace {
		void AssertCanCreateBranchNodeWithPath(const TreeNodePathTestCase& testCase) {
			// Act:
			auto node = BranchTreeNode(testCase.Path);

			// Assert:
			EXPECT_EQ(testCase.Path, node.path());
			AssertNoLinks(node);

			// - strip leaf flag from raw path
			auto rawPath = testCase.RawPath;
			rawPath[0] = static_cast<uint8_t>(rawPath[0] & ~0x20);

			auto expectedHash = CalculateEmptyBranchNodeHash(rawPath);
			EXPECT_EQ(expectedHash, node.hash());
		}
	}

	TEST(TEST_CLASS, CanCreateBranchTreeNodeWithEmptyPath) {
		AssertCanCreateBranchNodeWithPath({ TreeNodePath(), { 0x00 } });
	}

	TEST(TEST_CLASS, CanCreateBranchTreeNodeWithEvenPath) {
		AssertCanCreateBranchNodeWithPath(GetEvenTreeNodePathTestCase(PathType::Short));
	}

	TEST(TEST_CLASS, CanCreateBranchTreeNodeWithEvenPath_MultipleWorkingBuffers) {
		AssertCanCreateBranchNodeWithPath(GetEvenTreeNodePathTestCase(PathType::Long));
	}

	TEST(TEST_CLASS, CanCreateBranchTreeNodeWithEvenPath_EndingOnWorkingBufferBoundary) {
		AssertCanCreateBranchNodeWithPath(GetEvenTreeNodePathTestCase(PathType::Ending_On_Boundary));
	}

	TEST(TEST_CLASS, CanCreateBranchTreeNodeWithOddPath) {
		AssertCanCreateBranchNodeWithPath(GetOddTreeNodePathTestCase(PathType::Short));
	}

	TEST(TEST_CLASS, CanCreateBranchTreeNodeWithOddPath_MultipleWorkingBuffers) {
		AssertCanCreateBranchNodeWithPath(GetOddTreeNodePathTestCase(PathType::Long));
	}

	TEST(TEST_CLASS, CanCreateBranchTreeNodeWithOddPath_EndingOnWorkingBufferBoundary) {
		AssertCanCreateBranchNodeWithPath(GetOddTreeNodePathTestCase(PathType::Ending_On_Boundary));
	}

	// endregion

	// region BranchTreeNode - setPath

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

	// endregion

	// region BranchTreeNode - links

	namespace {
		struct HashLinkTraits {
			static constexpr auto AssertLink = AssertHashLink;

			static std::vector<Hash256> GenerateLinks(size_t count) {
				return test::GenerateRandomDataVector<Hash256>(count);
			}

			static const Hash256& GetHash(const Hash256& hash) {
				return hash;
			}
		};

		struct NodeLinkTraits {
			static constexpr auto AssertLink = AssertNodeLink;

			static std::vector<TreeNode> GenerateLinks(size_t count) {
				std::vector<TreeNode> nodes;
				for (auto i = 0u; i < count; ++i)
					nodes.emplace_back(LeafTreeNode(TreeNodePath(i), test::GenerateRandomByteArray<Hash256>()));

				return nodes;
			}

			static const Hash256& GetHash(const TreeNode& node) {
				return node.hash();
			}
		};
	}

#define BRANCH_LINK_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_HashLink) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<HashLinkTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NodeLink) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NodeLinkTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		// these helpers assume two links at indexes 6 and 11
		template<typename TTraits>
		void AssertTwoLinks(const BranchTreeNode& node, const Hash256& link1, const Hash256& link2) {
			// Assert:
			EXPECT_EQ(2u, node.numLinks());
			AssertEmptyLinks(node, 0, 5);
			TTraits::AssertLink(node, 6, link1);
			AssertEmptyLinks(node, 7, 10);
			TTraits::AssertLink(node, 11, link2);
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

	BRANCH_LINK_TEST(CanSetBranchTreeNodeLinks) {
		// Arrange:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto links = TTraits::GenerateLinks(2);
		auto node = BranchTreeNode(path);

		// Act:
		node.setLink(links[0], 6);
		node.setLink(links[1], 11);

		// Assert:
		EXPECT_EQ(path, node.path());
		AssertTwoLinks<TTraits>(node, TTraits::GetHash(links[0]), TTraits::GetHash(links[1]));

		auto expectedHash = CalculateTwoLinkHash({ 0x00, 0x64, 0x6F, 0x67, 0x00 }, TTraits::GetHash(links[0]), TTraits::GetHash(links[1]));
		EXPECT_EQ(expectedHash, node.hash());
	}

	BRANCH_LINK_TEST(BranchTreeNodeSetLinkTriggersHashRecalculation) {
		// Arrange:
		auto links = TTraits::GenerateLinks(2);
		auto node = BranchTreeNode(TreeNodePath(0x64'6F'67'00));
		node.setLink(links[0], 6);
		auto hash1 = node.hash();

		// Act:
		node.setLink(links[1], 8);
		auto hash2 = node.hash();

		// Assert:
		EXPECT_NE(hash1, hash2);
	}

	BRANCH_LINK_TEST(CanClearBranchTreeNodeLinks) {
		// Arrange:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto links = TTraits::GenerateLinks(4);
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
		AssertTwoLinks<TTraits>(node, TTraits::GetHash(links[0]), TTraits::GetHash(links[2]));

		auto expectedHash = CalculateTwoLinkHash({ 0x00, 0x64, 0x6F, 0x67, 0x00 }, TTraits::GetHash(links[0]), TTraits::GetHash(links[2]));
		EXPECT_EQ(expectedHash, node.hash());
	}

	BRANCH_LINK_TEST(BranchTreeNodeClearLinkTriggersHashRecalculation) {
		// Arrange:
		auto links = TTraits::GenerateLinks(3);
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

	BRANCH_LINK_TEST(BranchTreeNodeSetLinkCanChangeExistingLinks) {
		// Arrange:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto links = TTraits::GenerateLinks(2);
		auto node = BranchTreeNode(path);

		// - set a hash and node link
		node.setLink(HashLinkTraits::GenerateLinks(1)[0], 6);
		node.setLink(NodeLinkTraits::GenerateLinks(1)[0], 11);

		// Act: reset the links
		node.setLink(links[0], 6);
		node.setLink(links[1], 11);

		// Assert:
		EXPECT_EQ(path, node.path());
		AssertTwoLinks<TTraits>(node, TTraits::GetHash(links[0]), TTraits::GetHash(links[1]));

		auto expectedHash = CalculateTwoLinkHash({ 0x00, 0x64, 0x6F, 0x67, 0x00 }, TTraits::GetHash(links[0]), TTraits::GetHash(links[1]));
		EXPECT_EQ(expectedHash, node.hash());
	}

	BRANCH_LINK_TEST(BranchTreeNodeCompactLinksReplacesLinksWithHashLinks) {
		// Arrange:
		auto path = TreeNodePath(0x64'6F'67'00);
		auto links = TTraits::GenerateLinks(2);
		auto node = BranchTreeNode(path);

		// Act:
		node.setLink(links[0], 6);
		node.setLink(links[1], 11);
		node.compactLinks();

		// Assert: all links have been converted to hash links
		EXPECT_EQ(path, node.path());
		AssertTwoLinks<HashLinkTraits>(node, TTraits::GetHash(links[0]), TTraits::GetHash(links[1]));

		auto expectedHash = CalculateTwoLinkHash({ 0x00, 0x64, 0x6F, 0x67, 0x00 }, TTraits::GetHash(links[0]), TTraits::GetHash(links[1]));
		EXPECT_EQ(expectedHash, node.hash());
	}

	// endregion

	// region BranchTreeNode - highestLinkIndex

	namespace {
		BranchTreeNode CreateBranchTreeNodeWithLinks(std::initializer_list<size_t> linkIndexes) {
			auto node = BranchTreeNode(TreeNodePath(0x64'6F'67'00));

			for (auto index : linkIndexes)
				node.setLink(test::GenerateRandomByteArray<Hash256>(), index);

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
		AssertHighestLinkIndex(0xFF, {});
	}

	TEST(TEST_CLASS, BranchTreeNodeHighestLinkIndexReturnsCorrectValueWhenSingleLinkIsSet) {
		AssertHighestLinkIndex(0, { 0 });
		AssertHighestLinkIndex(7, { 7 });
		AssertHighestLinkIndex(15, { 15 });
	}

	TEST(TEST_CLASS, BranchTreeNodeHighestLinkIndexReturnsCorrectValueWhenMultipleLinksAreSet) {
		AssertHighestLinkIndex(4, { 1, 2, 3, 4 });
		AssertHighestLinkIndex(11, { 9, 11, 8 });
		AssertHighestLinkIndex(15, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 });
	}

	// endregion

	// region BranchTreeNode - hash optimization

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
			auto time1 = test::GetCurrentTimeNanoseconds();
			auto hash1 = node.hash();
			auto time2 = test::GetCurrentTimeNanoseconds();
			auto hash2 = node.hash();
			auto time3 = test::GetCurrentTimeNanoseconds();

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
		auto value = test::GenerateRandomByteArray<Hash256>();
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
		auto value = test::GenerateRandomByteArray<Hash256>();
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
		auto link1 = test::GenerateRandomByteArray<Hash256>();
		auto link2 = test::GenerateRandomByteArray<Hash256>();

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
		auto link1 = test::GenerateRandomByteArray<Hash256>();
		auto link2 = test::GenerateRandomByteArray<Hash256>();

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
		auto value = test::GenerateRandomByteArray<Hash256>();
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
		auto link1 = test::GenerateRandomByteArray<Hash256>();
		auto link2 = test::GenerateRandomByteArray<Hash256>();
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
