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

#include "catapult/tree/BasePatriciaTree.h"
#include "tests/test/tree/PassThroughEncoder.h"
#include "tests/TestHarness.h"

namespace catapult { namespace tree {

#define TEST_CLASS BasePatriciaTreeTests

	namespace {
		using MemoryBasePatriciaTree = BasePatriciaTree<test::PassThroughEncoder, MemoryDataSource>;

		template<typename TBasePatriciaTree>
		void SeedTreeWithFourNodes(TBasePatriciaTree& tree) {
			auto pDeltaTree = tree.rebase();
			pDeltaTree->set(0x64'6F'00'00, "verb");
			pDeltaTree->set(0x64'6F'67'00, "puppy");
			pDeltaTree->set(0x64'6F'67'65, "coin");
			pDeltaTree->set(0x68'6F'72'73, "stallion");
			tree.commit();
		}

		Hash256 CalculateRootHash(const std::vector<std::pair<uint32_t, std::string>>& pairs) {
			MemoryDataSource dataSource;
			PatriciaTree<test::PassThroughEncoder, MemoryDataSource> tree(dataSource);

			for (const auto& pair : pairs)
				tree.set(pair.first, pair.second);

			return tree.root();
		}

		Hash256 CalculateRootHashForTreeWithFourNodes() {
			return CalculateRootHash({
				{ 0x64'6F'00'00, "verb" },
				{ 0x64'6F'67'00, "puppy" },
				{ 0x64'6F'67'65, "coin" },
				{ 0x68'6F'72'73, "stallion" }
			});
		}
	}

	// region base + delta root hash forwarding

	TEST(TEST_CLASS, BasePatriciaTreeExposesCorrectRootHash) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// Act + Assert:
		auto expectedRoot = CalculateRootHashForTreeWithFourNodes();

		EXPECT_EQ(expectedRoot, tree.root());
	}

	TEST(TEST_CLASS, BasePatriciaTreeDeltaExposesCorrectRootHash) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// Act:
		auto pDeltaTree = tree.rebase();

		// Assert:
		auto expectedRoot = CalculateRootHashForTreeWithFourNodes();

		EXPECT_EQ(expectedRoot, pDeltaTree->root());
		EXPECT_EQ(expectedRoot, pDeltaTree->baseRoot());
	}

	TEST(TEST_CLASS, EmptyBasePatriciaTreeExposesCorrectRootHash) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);

		// Act + Assert:
		EXPECT_EQ(Hash256(), tree.root());
	}

	TEST(TEST_CLASS, EmptyBasePatriciaTreeDeltaExposesCorrectRootHash) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);

		// Act:
		auto pDeltaTree = tree.rebase();

		// Assert:
		EXPECT_EQ(Hash256(), pDeltaTree->root());
		EXPECT_EQ(Hash256(), pDeltaTree->baseRoot());
	}

	// endregion

	// region lookup forwarding

	TEST(TEST_CLASS, LookupForwardsToUnderlyingTreeWhenLookupSucceeds) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// Act:
		std::vector<TreeNode> nodePath;
		auto result = tree.lookup(0x64'6F'67'65, nodePath);

		// Assert:
		EXPECT_TRUE(result.second);
		EXPECT_NE(Hash256(), result.first);

		ASSERT_FALSE(nodePath.empty());
		EXPECT_TRUE(nodePath.back().isLeaf());

		const auto& nodePathLeaf = nodePath.back().asLeafNode();
		EXPECT_EQ(TreeNodePath(0x64'6F'67'65).subpath(7), nodePathLeaf.path());
		EXPECT_EQ(nodePathLeaf.value(), result.first);
	}

	TEST(TEST_CLASS, LookupForwardsToUnderlyingTreeWhenLookupFails) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// Act:
		std::vector<TreeNode> nodePath;
		auto result = tree.lookup(0x64'6F'67'64, nodePath);

		// Assert:
		EXPECT_FALSE(result.second);
		EXPECT_EQ(Hash256(), result.first);

		ASSERT_FALSE(nodePath.empty());
		EXPECT_TRUE(nodePath.back().isLeaf());

		// - leaf returned is `0x64'6F'67'65` because it shares branch path with queried (non-existent) node
		const auto& nodePathLeaf = nodePath.back().asLeafNode();
		EXPECT_EQ(TreeNodePath(0x64'6F'67'65).subpath(7), nodePathLeaf.path());
		EXPECT_NE(nodePathLeaf.value(), result.first);
	}

	// endregion

	// region loading

	TEST(TEST_CLASS, CanConstructBaseTreeAroundKnownRootHash) {
		// Arrange: add a tree to the data source
		MemoryDataSource dataSource;
		{
			MemoryBasePatriciaTree tree(dataSource);
			SeedTreeWithFourNodes(tree);
		}

		auto expectedRoot = CalculateRootHashForTreeWithFourNodes();

		// Act:
		MemoryBasePatriciaTree tree(dataSource, expectedRoot);

		// Assert:
		EXPECT_EQ(expectedRoot, tree.root());
	}

	TEST(TEST_CLASS, CannotConstructBaseTreeAroundUnknownRootHash) {
		// Arrange: add a tree to the data source
		MemoryDataSource dataSource;
		{
			MemoryBasePatriciaTree tree(dataSource);
			SeedTreeWithFourNodes(tree);
		}

		auto root = test::GenerateRandomByteArray<Hash256>();

		// Act + Assert:
		EXPECT_THROW(MemoryBasePatriciaTree tree(dataSource, root), catapult_runtime_error);
	}

	// endregion

	// region rebase / rebaseDetached

	TEST(TEST_CLASS, RebaseCreatesDeltaAroundTree) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// Act:
		auto pDeltaTree = tree.rebase();

		// Assert:
		EXPECT_EQ(tree.root(), pDeltaTree->root());
	}

	TEST(TEST_CLASS, RebaseAllowsOnlyOneAttachedDeltaAtATime) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		{
			auto pDeltaTree = tree.rebase();

			// Act + Assert:
			EXPECT_THROW(tree.rebase(), catapult_runtime_error);
		}

		// Act: delta went out of scope, another delta is allowed
		auto pDeltaTree = tree.rebase();
		EXPECT_EQ(tree.root(), pDeltaTree->root());
	}

	TEST(TEST_CLASS, RebaseDetachedCreatesDeltaAroundTree) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// Act:
		auto pDeltaTree = tree.rebaseDetached();

		// Assert:
		EXPECT_EQ(tree.root(), pDeltaTree->root());
	}

	TEST(TEST_CLASS, RebaseDetachedAllowsManyDeltas) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// Act:
		std::vector<decltype(tree.rebaseDetached())> deltas;
		for (auto i = 0u; i < 10; ++i)
			deltas.push_back(tree.rebaseDetached());

		// Assert:
		for (const auto& pDeltaTree : deltas)
			EXPECT_EQ(tree.root(), pDeltaTree->root());
	}

	// endregion

	// region commit

	TEST(TEST_CLASS, CannotCommitWhenThereAreNoPendingAttachedDeltas) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// Act + Assert:
		EXPECT_THROW(tree.commit(), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CommitThrowsWhenOnlyDetachedDeltasAreOutstanding) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		auto pDeltaTree = tree.rebaseDetached();

		// Act + Assert:
		EXPECT_THROW(tree.commit(), catapult_runtime_error);
	}

	namespace {
		void AssertCommitChangesBasePatriciaTree(size_t numCommits) {
			// Arrange:
			MemoryDataSource dataSource;
			MemoryBasePatriciaTree tree(dataSource);
			SeedTreeWithFourNodes(tree);

			auto pDeltaTree = tree.rebase();
			pDeltaTree->set(0x26'54'32'10, "alpha");
			pDeltaTree->unset(0x64'6F'67'65);
			pDeltaTree->set(0x64'6F'00'00, "noun");

			// Act:
			for (auto i = 0u; i < numCommits; ++i)
				tree.commit();

			// Assert:
			auto expectedRoot = CalculateRootHash({
				{ 0x64'6F'00'00, "noun" },
				{ 0x64'6F'67'00, "puppy" },
				{ 0x68'6F'72'73, "stallion" },
				{ 0x26'54'32'10, "alpha" }
			});

			EXPECT_EQ(expectedRoot, tree.root());
			EXPECT_EQ(expectedRoot, pDeltaTree->root());
		}
	}

	TEST(TEST_CLASS, CommitChangesBasePatriciaTree) {
		AssertCommitChangesBasePatriciaTree(1);
	}

	TEST(TEST_CLASS, CommitIsIdempotent) {
		AssertCommitChangesBasePatriciaTree(9);
	}

	TEST(TEST_CLASS, UncommittedChangesDoNotChangeBasePatriciaTree) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// Act: make delta changes but don't commit
		auto pDeltaTree = tree.rebase();
		pDeltaTree->set(0x26'54'32'10, "alpha");
		pDeltaTree->unset(0x64'6F'67'65);
		pDeltaTree->set(0x64'6F'00'00, "noun");

		// Assert:
		auto expectedBaseRoot = CalculateRootHashForTreeWithFourNodes();

		auto expectedDeltaRoot = CalculateRootHash({
			{ 0x64'6F'00'00, "noun" },
			{ 0x64'6F'67'00, "puppy" },
			{ 0x68'6F'72'73, "stallion" },
			{ 0x26'54'32'10, "alpha" }
		});

		EXPECT_EQ(expectedBaseRoot, tree.root());
		EXPECT_EQ(expectedDeltaRoot, pDeltaTree->root());

		// Sanity:
		EXPECT_NE(expectedBaseRoot, expectedDeltaRoot);
	}

	TEST(TEST_CLASS, CommitRespectsChangeGroups) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// - group one set of changes
		auto pDeltaTree = tree.rebase();
		pDeltaTree->set(0x26'54'32'10, "alpha");
		pDeltaTree->unset(0x64'6F'67'65);
		pDeltaTree->set(0x64'6F'00'00, "noun");
		pDeltaTree->setCheckpoint();
		auto root1 = pDeltaTree->root();

		// - group another set of changes
		pDeltaTree->set(0x26'54'32'11, "beta");
		pDeltaTree->unset(0x68'6F'72'73);
		pDeltaTree->set(0x64'6F'67'00, "kitten");
		pDeltaTree->setCheckpoint();
		auto root2 = pDeltaTree->root();

		// Act:
		tree.commit();

		// Assert: both roots were saved to the data source
		EXPECT_FALSE(dataSource.get(root1).empty());
		EXPECT_FALSE(dataSource.get(root2).empty());

		// - lookup of all (set) leafs should succeed
		std::vector<TreeNode> nodePath;
		for (auto key : std::initializer_list<uint32_t>{ 0x26'54'32'10, 0x64'6F'00'00, 0x26'54'32'11, 0x64'6F'67'00 })
			EXPECT_TRUE(tree.lookup(key, nodePath).second) << utils::HexFormat(key);
	}

	TEST(TEST_CLASS, SaveOptimizationIncludesChangedSubTrees) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);

		// - group one set of changes
		auto pDeltaTree = tree.rebase();
		pDeltaTree->set(0x26'54'32'10, "alpha");
		pDeltaTree->set(0x64'6F'00'00, "noun");
		pDeltaTree->setCheckpoint();
		auto root1 = pDeltaTree->root();

		// - group another set of changes
		pDeltaTree->set(0x64'6F'11'11, "verb");
		pDeltaTree->setCheckpoint();
		auto root2 = pDeltaTree->root();

		// Act:
		tree.commit();

		// Assert: both roots were saved to the data source
		EXPECT_FALSE(dataSource.get(root1).empty());
		EXPECT_FALSE(dataSource.get(root2).empty());

		// -  first tree: B(alpha, noun), L(alpha), L(verb)
		// - second tree: B(alpha, B(noun, verb)), B(noun, verb), L(noun), L(verb)
		EXPECT_EQ(3 + 4u, dataSource.size());

		// - lookup of all leafs should succeed
		std::vector<TreeNode> nodePath;
		for (auto key : std::initializer_list<uint32_t>{ 0x26'54'32'10, 0x64'6F'00'00, 0x64'6F'11'11 })
			EXPECT_TRUE(tree.lookup(key, nodePath).second) << utils::HexFormat(key);
	}

	// endregion

	// region reset

	namespace {
		template<typename TAction>
		void RunResetToBaseRootTest(TAction action) {
			// Arrange:
			MemoryDataSource dataSource;
			MemoryBasePatriciaTree tree(dataSource);
			SeedTreeWithFourNodes(tree);

			// - make delta changes but don't commit
			auto pDeltaTree = tree.rebase();
			pDeltaTree->set(0x26'54'32'10, "alpha");
			pDeltaTree->unset(0x64'6F'67'65);
			pDeltaTree->set(0x64'6F'00'00, "noun");

			// Act + Assert:
			action(tree, *pDeltaTree);
		}
	}

	TEST(TEST_CLASS, DeltaResetCanResetToBaseRoot) {
		// Act:
		RunResetToBaseRootTest([](const auto& tree, auto& deltaTree) {
			deltaTree.reset(tree.root());

			// Assert:
			auto expectedRoot = CalculateRootHashForTreeWithFourNodes();

			EXPECT_EQ(expectedRoot, deltaTree.root());
			EXPECT_EQ(expectedRoot, deltaTree.baseRoot());
		});
	}

	TEST(TEST_CLASS, CanCommitAfterDeltaResetToBaseRoot) {
		// Act:
		RunResetToBaseRootTest([](auto& tree, auto& deltaTree) {
			deltaTree.reset(tree.root());
			tree.commit();

			// Assert:
			auto expectedRoot = CalculateRootHashForTreeWithFourNodes();

			EXPECT_EQ(expectedRoot, tree.root());
		});
	}

	namespace {
		template<typename TAction>
		void RunResetToPreviousBaseRootTest(TAction action) {
			// Arrange:
			MemoryDataSource dataSource;
			MemoryBasePatriciaTree tree(dataSource);
			SeedTreeWithFourNodes(tree);

			// - make delta changes and commit
			auto pDeltaTree = tree.rebase();
			pDeltaTree->set(0x26'54'32'10, "alpha");
			pDeltaTree->unset(0x64'6F'67'65);
			tree.commit();

			// - make delta changes but don't commit (reset should discard these changes)
			pDeltaTree->set(0x64'6F'00'00, "noun");
			pDeltaTree->unset(0x68'6F'72'73);

			// Act + Assert:
			action(tree, *pDeltaTree);
		}
	}

	TEST(TEST_CLASS, DeltaResetCanResetToPreviousBaseRoot) {
		// Act:
		RunResetToPreviousBaseRootTest([](const auto&, auto& deltaTree) {
			auto expectedRoot = CalculateRootHashForTreeWithFourNodes();
			deltaTree.reset(expectedRoot);

			// Assert:
			EXPECT_EQ(expectedRoot, deltaTree.root());
			EXPECT_EQ(expectedRoot, deltaTree.baseRoot());
		});
	}

	TEST(TEST_CLASS, CanCommitAfterDeltaResetToPreviousBaseRoot) {
		// Act:
		RunResetToPreviousBaseRootTest([](auto& tree, auto& deltaTree) {
			auto expectedRoot = CalculateRootHashForTreeWithFourNodes();
			deltaTree.reset(expectedRoot);
			tree.commit();

			// Assert:
			EXPECT_EQ(expectedRoot, tree.root());
		});
	}

	TEST(TEST_CLASS, DeltaResetCanResetToEmptyRoot) {
		// Act:
		RunResetToBaseRootTest([](const auto&, auto& deltaTree) {
			deltaTree.reset(Hash256());

			// Assert:
			EXPECT_EQ(Hash256(), deltaTree.root());
			EXPECT_EQ(Hash256(), deltaTree.baseRoot());
		});
	}

	TEST(TEST_CLASS, CanCommitAfterDeltaResetToEmptyRoot) {
		// Act:
		RunResetToBaseRootTest([](auto& tree, auto& deltaTree) {
			deltaTree.reset(Hash256());
			tree.commit();

			// Assert:
			EXPECT_EQ(Hash256(), tree.root());
		});
	}

	TEST(TEST_CLASS, DeltaResetCannotSetRootToUnknownHash) {
		// Act:
		RunResetToBaseRootTest([](const auto&, auto& deltaTree) {
			// Act + Assert:
			EXPECT_THROW(deltaTree.reset(test::GenerateRandomByteArray<Hash256>()), catapult_runtime_error);
		});
	}

	// endregion

	// region change ordering

	TEST(TEST_CLASS, CanSetAndUnsetValue) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// Act: the last operation (unset) should take precedence
		auto pDeltaTree = tree.rebase();
		pDeltaTree->set(0x64'6F'67'00, "kitten");
		pDeltaTree->unset(0x64'6F'67'00);
		tree.commit();

		// Assert:
		auto expectedRoot = CalculateRootHash({
			{ 0x64'6F'00'00, "verb" },
			{ 0x64'6F'67'65, "coin" },
			{ 0x68'6F'72'73, "stallion" }
		});

		EXPECT_EQ(expectedRoot, tree.root());
		EXPECT_EQ(expectedRoot, pDeltaTree->root());
	}

	TEST(TEST_CLASS, CanUnsetAndSetValue) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// Act: the last operation (set) should take precedence
		auto pDeltaTree = tree.rebase();
		pDeltaTree->unset(0x64'6F'67'00);
		pDeltaTree->set(0x64'6F'67'00, "kitten");
		tree.commit();

		// Assert:
		auto expectedRoot = CalculateRootHash({
			{ 0x64'6F'00'00, "verb" },
			{ 0x64'6F'67'00, "kitten" },
			{ 0x64'6F'67'65, "coin" },
			{ 0x68'6F'72'73, "stallion" }
		});

		EXPECT_EQ(expectedRoot, tree.root());
		EXPECT_EQ(expectedRoot, pDeltaTree->root());
	}

	TEST(TEST_CLASS, CanSetAndChangeValue) {
		// Arrange:
		MemoryDataSource dataSource;
		MemoryBasePatriciaTree tree(dataSource);
		SeedTreeWithFourNodes(tree);

		// Act: the last operation (set) should take precedence
		auto pDeltaTree = tree.rebase();
		pDeltaTree->set(0x64'6F'67'00, "kitten");
		pDeltaTree->set(0x64'6F'67'00, "pony");
		tree.commit();

		// Assert:
		auto expectedRoot = CalculateRootHash({
			{ 0x64'6F'00'00, "verb" },
			{ 0x64'6F'67'00, "pony" },
			{ 0x64'6F'67'65, "coin" },
			{ 0x68'6F'72'73, "stallion" }
		});

		EXPECT_EQ(expectedRoot, tree.root());
		EXPECT_EQ(expectedRoot, pDeltaTree->root());
	}

	// endregion

	// region custom hasher

	namespace {
		class CustomKeyPassThroughEncoder : public test::PassThroughEncoder {
		public:
			struct CustomKey_tag {};
			using KeyType = utils::BaseValue<uint32_t, CustomKey_tag>;

		public:
			static const KeyType& EncodeKey(const KeyType& key) {
				return key;
			}
		};
	}

	TEST(TEST_CLASS, CanSpecifyCustomKeyHasher) {
		// Arrange: this is same as commit changes test but with different key type
		using CustomKey = CustomKeyPassThroughEncoder::KeyType;

		MemoryDataSource dataSource;
		BasePatriciaTree<CustomKeyPassThroughEncoder, MemoryDataSource, utils::BaseValueHasher<CustomKey>> tree(dataSource);
		{
			auto pDeltaTree = tree.rebase();
			pDeltaTree->set(CustomKey(0x64'6F'00'00), "verb");
			pDeltaTree->set(CustomKey(0x64'6F'67'00), "puppy");
			pDeltaTree->set(CustomKey(0x64'6F'67'65), "coin");
			pDeltaTree->set(CustomKey(0x68'6F'72'73), "stallion");
			tree.commit();
		}

		// Act:
		auto pDeltaTree = tree.rebase();
		pDeltaTree->set(CustomKey(0x26'54'32'10), "alpha");
		pDeltaTree->unset(CustomKey(0x64'6F'67'65));
		pDeltaTree->set(CustomKey(0x64'6F'00'00), "noun");
		tree.commit();

		// Assert:
		auto expectedRoot = CalculateRootHash({
			{ 0x64'6F'00'00, "noun" },
			{ 0x64'6F'67'00, "puppy" },
			{ 0x68'6F'72'73, "stallion" },
			{ 0x26'54'32'10, "alpha" }
		});

		EXPECT_EQ(expectedRoot, tree.root());
		EXPECT_EQ(expectedRoot, pDeltaTree->root());
	}

	// endregion

	// region data source with delayed flush

	namespace {
		// simulate a data source that queues all writes until a commit action
		class DelayedWriteDataSource : public MemoryDataSource {
		public:
			void set(const LeafTreeNode& node) {
				m_pendingLeafNodes.push_back(node);
			}

			void set(const BranchTreeNode& node) {
				m_pendingBranchNodes.push_back(node);
			}

		public:
			void commit() {
				for (const auto& node : m_pendingLeafNodes)
					MemoryDataSource::set(node);

				for (const auto& node : m_pendingBranchNodes)
					MemoryDataSource::set(node);
			}

		private:
			std::vector<LeafTreeNode> m_pendingLeafNodes;
			std::vector<BranchTreeNode> m_pendingBranchNodes;
		};
	}

	TEST(TEST_CLASS, CommitSupportsDataSourcePerformingDelayedWrite) {
		// Arrange:
		DelayedWriteDataSource dataSource;
		BasePatriciaTree<test::PassThroughEncoder, DelayedWriteDataSource> tree(dataSource);
		SeedTreeWithFourNodes(tree);
		dataSource.commit();

		auto pDeltaTree = tree.rebase();
		pDeltaTree->set(0x26'54'32'10, "alpha");
		pDeltaTree->unset(0x64'6F'67'65);
		pDeltaTree->set(0x64'6F'00'00, "noun");

		// Act:
		tree.commit();
		dataSource.commit();

		// Assert:
		auto expectedRoot = CalculateRootHash({
			{ 0x64'6F'00'00, "noun" },
			{ 0x64'6F'67'00, "puppy" },
			{ 0x68'6F'72'73, "stallion" },
			{ 0x26'54'32'10, "alpha" }
		});

		EXPECT_EQ(expectedRoot, tree.root());
		EXPECT_EQ(expectedRoot, pDeltaTree->root());
	}

	// endregion
}}
