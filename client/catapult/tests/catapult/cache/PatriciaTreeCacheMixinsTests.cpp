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

#include "catapult/cache/PatriciaTreeCacheMixins.h"
#include "tests/catapult/cache/test/PatriciaTreeTestUtils.h"
#include "tests/test/other/DeltaElementsTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS PatriciaTreeCacheMixinsTests

	namespace {
		using MemoryPatriciaTree = test::MemoryPatriciaTree;
		using DeltasWrapper = test::DeltaElementsTestUtils::WrapperWithGenerationalSupport<std::unordered_map<uint32_t, std::string>>;
	}

	// region PatriciaTreeMixin - supportsMerkleRoot

	TEST(TEST_CLASS, ViewMixin_SupportsReturnsFalseWhenTreeIsNullptr) {
		// Arrange:
		auto mixin = PatriciaTreeMixin<MemoryPatriciaTree>(nullptr);

		// Act:
		auto result = mixin.supportsMerkleRoot();

		// Assert:
		EXPECT_FALSE(result);
	}

	TEST(TEST_CLASS, ViewMixin_SupportsReturnsTrueWhenTreeIsValid) {
		// Arrange:
		tree::MemoryDataSource dataSource;
		MemoryPatriciaTree tree(dataSource);
		auto mixin = PatriciaTreeMixin<MemoryPatriciaTree>(&tree);

		// Act:
		auto result = mixin.supportsMerkleRoot();

		// Assert:
		EXPECT_TRUE(result);
	}

	// endregion

	// region PatriciaTreeMixin - tryGetMerkleRoot

	TEST(TEST_CLASS, ViewMixin_TryGetReturnsPairWithFalseComponentWhenTreeIsNullptr) {
		// Arrange:
		auto mixin = PatriciaTreeMixin<MemoryPatriciaTree>(nullptr);

		// Act:
		auto result = mixin.tryGetMerkleRoot();

		// Assert:
		EXPECT_FALSE(result.second);
	}

	TEST(TEST_CLASS, ViewMixin_TryGetReturnsRootWhenTreeIsValid) {
		// Arrange:
		tree::MemoryDataSource dataSource;
		MemoryPatriciaTree tree(dataSource);
		test::SeedTreeWithFourNodes(tree);

		auto mixin = PatriciaTreeMixin<MemoryPatriciaTree>(&tree);

		// Act:
		auto result = mixin.tryGetMerkleRoot();

		// Assert:
		EXPECT_TRUE(result.second);
		EXPECT_EQ(tree.root(), result.first);
	}

	// endregion

	// region PatriciaTreeMixin - tryLookup

	TEST(TEST_CLASS, ViewMixin_LookupReturnsFalseWhenTreeIsNullptr) {
		// Arrange:
		auto mixin = PatriciaTreeMixin<MemoryPatriciaTree>(nullptr);

		// Act:
		std::vector<tree::TreeNode> nodePath;
		auto result = mixin.tryLookup(0x64'6F'67'65, nodePath);

		// Assert:
		EXPECT_FALSE(result.second);
		EXPECT_EQ(Hash256(), result.first);
		EXPECT_TRUE(nodePath.empty());
	}

	TEST(TEST_CLASS, ViewMixin_LookupForwardsToUnderlyingTreeWhenTreeIsValidAndLookupSucceeds) {
		// Arrange:
		tree::MemoryDataSource dataSource;
		MemoryPatriciaTree tree(dataSource);
		test::SeedTreeWithFourNodes(tree);

		auto mixin = PatriciaTreeMixin<MemoryPatriciaTree>(&tree);

		// Act:
		std::vector<tree::TreeNode> nodePath;
		auto result = mixin.tryLookup(0x64'6F'67'65, nodePath);

		// Assert:
		EXPECT_TRUE(result.second);
		EXPECT_NE(Hash256(), result.first);

		ASSERT_FALSE(nodePath.empty());
		EXPECT_TRUE(nodePath.back().isLeaf());

		const auto& nodePathLeaf = nodePath.back().asLeafNode();
		EXPECT_EQ(tree::TreeNodePath(0x64'6F'67'65).subpath(7), nodePathLeaf.path());
		EXPECT_EQ(nodePathLeaf.value(), result.first);
	}

	TEST(TEST_CLASS, ViewMixin_LookupForwardsToUnderlyingTreeWhenTreeIsValidAndLookupFails) {
		// Arrange:
		tree::MemoryDataSource dataSource;
		MemoryPatriciaTree tree(dataSource);
		test::SeedTreeWithFourNodes(tree);

		auto mixin = PatriciaTreeMixin<MemoryPatriciaTree>(&tree);

		// Act:
		std::vector<tree::TreeNode> nodePath;
		auto result = mixin.tryLookup(0x64'6F'67'64, nodePath);

		// Assert:
		EXPECT_FALSE(result.second);
		EXPECT_EQ(Hash256(), result.first);

		ASSERT_FALSE(nodePath.empty());
		EXPECT_TRUE(nodePath.back().isLeaf());

		// - leaf returned is `0x64'6F'67'65` because it shares branch path with queried (non-existent) node
		const auto& nodePathLeaf = nodePath.back().asLeafNode();
		EXPECT_EQ(tree::TreeNodePath(0x64'6F'67'65).subpath(7), nodePathLeaf.path());
		EXPECT_NE(nodePathLeaf.value(), result.first);
	}

	// endregion

	// region PatriciaTreeDeltaMixin - supportsMerkleRoot

	TEST(TEST_CLASS, DeltaMixin_SupportsReturnsFalseWhenTreeIsNullptr) {
		// Arrange:
		DeltasWrapper deltaset;
		auto mixin = PatriciaTreeDeltaMixin<DeltasWrapper, test::MemoryBasePatriciaTree::DeltaType>(deltaset, nullptr);

		// Act:
		auto result = mixin.supportsMerkleRoot();

		// Assert:
		EXPECT_FALSE(result);
	}

	TEST(TEST_CLASS, DeltaMixin_SupportsReturnsTrueWhenTreeIsValid) {
		// Arrange:
		tree::MemoryDataSource dataSource;
		test::MemoryBasePatriciaTree tree(dataSource);

		DeltasWrapper deltaset;
		auto pDeltaTree = tree.rebase();
		auto mixin = PatriciaTreeDeltaMixin<DeltasWrapper, test::MemoryBasePatriciaTree::DeltaType>(deltaset, pDeltaTree);

		// Act:
		auto result = mixin.supportsMerkleRoot();

		// Assert:
		EXPECT_TRUE(result);
	}

	// endregion

	// region PatriciaTreeDeltaMixin - tryGetMerkleRoot / updateMerkleRoot

	TEST(TEST_CLASS, DeltaMixin_TryGetReturnsPairWithFalseComponentWhenTreeIsNullptr) {
		// Arrange:
		DeltasWrapper deltaset;
		auto mixin = PatriciaTreeDeltaMixin<DeltasWrapper, test::MemoryBasePatriciaTree::DeltaType>(deltaset, nullptr);
		mixin.updateMerkleRoot(Height(123));

		// Act:
		auto result = mixin.tryGetMerkleRoot();

		// Assert:
		EXPECT_FALSE(result.second);

		EXPECT_EQ(1u, deltaset.generationId());
	}

	TEST(TEST_CLASS, DeltaMixin_TryGetReturnsRootWhenTreeIsValidAndHasNoModifications) {
		// Arrange:
		tree::MemoryDataSource dataSource;
		test::MemoryBasePatriciaTree tree(dataSource);
		test::SeedTreeWithFourNodes(tree);

		DeltasWrapper deltaset;
		auto pDeltaTree = tree.rebase();
		auto mixin = PatriciaTreeDeltaMixin<DeltasWrapper, test::MemoryBasePatriciaTree::DeltaType>(deltaset, pDeltaTree);
		mixin.updateMerkleRoot(Height(123));

		// Act:
		auto result = mixin.tryGetMerkleRoot();

		// Assert:
		EXPECT_TRUE(result.second);
		EXPECT_EQ(pDeltaTree->root(), result.first);

		EXPECT_EQ(2u, deltaset.generationId());
	}

	namespace {
		Hash256 GetExpectedRootHashAfterChangeApplications() {
			return test::CalculateRootHash({
				{ 0x64'6F'00'00, "noun" },
				{ 0x64'6F'67'00, "puppy" },
				{ 0x68'6F'72'73, "stallion" },
				{ 0x26'54'32'10, "alpha" }
			});
		}
	}

	TEST(TEST_CLASS, DeltaMixin_TryGetReturnsRootWhenTreeIsValidAndHasModificationsAcrossSingleGeneration) {
		// Arrange:
		tree::MemoryDataSource dataSource;
		test::MemoryBasePatriciaTree tree(dataSource);
		test::SeedTreeWithFourNodes(tree);

		DeltasWrapper deltaset;
		deltaset.Added.emplace(0x26'54'32'10, "alpha");
		deltaset.Removed.emplace(0x64'6F'67'65, "coin");
		deltaset.Copied.emplace(0x64'6F'00'00, "noun");

		auto pDeltaTree = tree.rebase();
		auto mixin = PatriciaTreeDeltaMixin<DeltasWrapper, test::MemoryBasePatriciaTree::DeltaType>(deltaset, pDeltaTree);
		mixin.updateMerkleRoot(Height(123));

		// Act:
		auto result = mixin.tryGetMerkleRoot();

		// Assert:
		auto expectedRoot = GetExpectedRootHashAfterChangeApplications();

		EXPECT_TRUE(result.second);
		EXPECT_EQ(expectedRoot, result.first);

		EXPECT_EQ(2u, deltaset.generationId());

		// Sanity: the (delta) tree was modified
		EXPECT_EQ(expectedRoot, pDeltaTree->root());
	}

	TEST(TEST_CLASS, DeltaMixin_TryGetReturnsRootWhenTreeIsValidAndHasModificationsAcrossMultipleGenerations) {
		// Arrange:
		tree::MemoryDataSource dataSource;
		test::MemoryBasePatriciaTree tree(dataSource);
		test::SeedTreeWithFourNodes(tree);

		DeltasWrapper deltaset;
		deltaset.Added.emplace(0x26'54'32'10, "alpha");
		deltaset.Removed.emplace(0x64'6F'67'65, "coin");
		deltaset.Copied.emplace(0x64'6F'00'00, "noun");

		// - mark each modification as belonging to a different generation
		deltaset.setGenerationId(0x26'54'32'10, 1);
		deltaset.setGenerationId(0x64'6F'67'65, 2);
		deltaset.setGenerationId(0x64'6F'00'00, 3);

		auto pDeltaTree = tree.rebase();
		auto mixin = PatriciaTreeDeltaMixin<DeltasWrapper, test::MemoryBasePatriciaTree::DeltaType>(deltaset, pDeltaTree);

		// Act:
		mixin.updateMerkleRoot(Height(123)); // generation 1
		deltaset.incrementGenerationId();
		mixin.updateMerkleRoot(Height(124)); // generation 3
		auto result = mixin.tryGetMerkleRoot();

		// Assert:
		auto expectedRoot = GetExpectedRootHashAfterChangeApplications();

		EXPECT_TRUE(result.second);
		EXPECT_EQ(expectedRoot, result.first);

		EXPECT_EQ(4u, deltaset.generationId());

		// Sanity: the (delta) tree was modified
		EXPECT_EQ(expectedRoot, pDeltaTree->root());
	}

	TEST(TEST_CLASS, DeltaMixin_UpdatePreservesGenerationalRootHashes) {
		// Arrange:
		tree::MemoryDataSource dataSource;
		test::MemoryBasePatriciaTree tree(dataSource);
		test::SeedTreeWithFourNodes(tree);

		std::vector<Hash256> expectedRoots;
		{
			auto pDeltaTree = tree.rebase();
			pDeltaTree->set(0x26'54'32'10, "alpha");
			expectedRoots.push_back(pDeltaTree->root());
			pDeltaTree->unset(0x64'6F'67'65);
			expectedRoots.push_back(pDeltaTree->root());
			pDeltaTree->set(0x64'6F'00'00, "noun");
			expectedRoots.push_back(pDeltaTree->root());
		}

		DeltasWrapper deltaset;
		deltaset.Added.emplace(0x26'54'32'10, "alpha");
		deltaset.Removed.emplace(0x64'6F'67'65, "coin");
		deltaset.Copied.emplace(0x64'6F'00'00, "noun");

		// - mark each modification as belonging to a different generation
		deltaset.setGenerationId(0x26'54'32'10, 1);
		deltaset.setGenerationId(0x64'6F'67'65, 2);
		deltaset.setGenerationId(0x64'6F'00'00, 3);

		auto pDeltaTree = tree.rebase();
		auto mixin = PatriciaTreeDeltaMixin<DeltasWrapper, test::MemoryBasePatriciaTree::DeltaType>(deltaset, pDeltaTree);

		// Act:
		mixin.updateMerkleRoot(Height(123)); // generation 1
		mixin.updateMerkleRoot(Height(124)); // generation 2
		mixin.updateMerkleRoot(Height(125)); // generation 3

		// - commit changes so that nodes get saved into data source
		tree.commit();

		// Assert:
		EXPECT_FALSE(dataSource.get(expectedRoots[0]).empty());
		EXPECT_FALSE(dataSource.get(expectedRoots[1]).empty());
		EXPECT_FALSE(dataSource.get(expectedRoots[2]).empty());
	}

	// endregion

	// region PatriciaTreeDeltaMixin - setMerkleRoot

	TEST(TEST_CLASS, DeltaMixin_SetThrowsWhenTreeIsNullptr) {
		// Arrange:
		DeltasWrapper deltaset;
		auto mixin = PatriciaTreeDeltaMixin<DeltasWrapper, test::MemoryBasePatriciaTree::DeltaType>(deltaset, nullptr);

		// Act + Assert:
		EXPECT_THROW(mixin.setMerkleRoot(Hash256()), catapult_runtime_error);

		EXPECT_EQ(1u, deltaset.generationId());
	}

	namespace {
		enum class ChangeOptions { None, Pending };

		template<typename TTree>
		void AssertCanSetMerkleRoot(const std::shared_ptr<TTree>& pDeltaTree, const Hash256& merkleRoot, ChangeOptions changeOptions) {
			// Arrange:
			DeltasWrapper deltaset;
			if (ChangeOptions::Pending == changeOptions) {
				deltaset.Added.emplace(0x26'54'32'10, "alpha");
				deltaset.Copied.emplace(0x64'6F'00'00, "noun");
				deltaset.Removed.emplace(0x64'6F'67'65, "coin");
			}

			auto mixin = PatriciaTreeDeltaMixin<DeltasWrapper, test::MemoryBasePatriciaTree::DeltaType>(deltaset, pDeltaTree);
			mixin.updateMerkleRoot(Height(123));

			// Sanity:
			EXPECT_NE(merkleRoot, mixin.tryGetMerkleRoot().first);

			// Act:
			auto preSetGenerationId = deltaset.generationId();
			mixin.setMerkleRoot(merkleRoot);
			auto postSetGenerationId = deltaset.generationId();

			// Assert:
			EXPECT_EQ(merkleRoot, mixin.tryGetMerkleRoot().first);

			EXPECT_EQ(2u, preSetGenerationId);
			EXPECT_EQ(3u, postSetGenerationId);

			EXPECT_EQ(ChangeOptions::Pending == changeOptions, !deltaset.Added.empty());
			EXPECT_EQ(ChangeOptions::Pending == changeOptions, !deltaset.Copied.empty());
			EXPECT_EQ(ChangeOptions::Pending == changeOptions, !deltaset.Removed.empty());
		}

		void AssertSetToEmptyRootSucceedsWhenTreeIsValid(ChangeOptions changeOptions) {
			// Arrange:
			tree::MemoryDataSource dataSource;
			test::MemoryBasePatriciaTree tree(dataSource);
			test::SeedTreeWithFourNodes(tree);
			auto pDeltaTree = tree.rebase();

			// Act + Assert:
			AssertCanSetMerkleRoot(pDeltaTree, Hash256(), changeOptions);
		}

		void AssertSetToPreviousRootSucceedsWhenTreeIsValid(ChangeOptions changeOptions) {
			// Arrange:
			tree::MemoryDataSource dataSource;
			test::MemoryBasePatriciaTree tree(dataSource);
			test::SeedTreeWithFourNodes(tree);
			auto previousRoot = tree.root();

			// - make delta changes and commit
			auto pDeltaTree = tree.rebase();
			pDeltaTree->set(0x26'54'32'10, "alpha");
			pDeltaTree->unset(0x64'6F'67'65);
			tree.commit();

			// Act + Assert:
			AssertCanSetMerkleRoot(pDeltaTree, previousRoot, changeOptions);
		}
	}

	TEST(TEST_CLASS, DeltaMixin_SetToEmptyRootSucceedsWhenTreeIsValid) {
		AssertSetToEmptyRootSucceedsWhenTreeIsValid(ChangeOptions::None);
	}

	TEST(TEST_CLASS, DeltaMixin_SetToPreviousRootSucceedsWhenTreeIsValid) {
		AssertSetToPreviousRootSucceedsWhenTreeIsValid(ChangeOptions::None);
	}

	TEST(TEST_CLASS, DeltaMixin_SetToEmptyRootSucceedsWhenTreeIsValidWithPendingChanges) {
		AssertSetToEmptyRootSucceedsWhenTreeIsValid(ChangeOptions::Pending);
	}

	TEST(TEST_CLASS, DeltaMixin_SetToPreviousRootSucceedsWhenTreeIsValidWithPendingChanges) {
		AssertSetToPreviousRootSucceedsWhenTreeIsValid(ChangeOptions::Pending);
	}

	// endregion
}}
