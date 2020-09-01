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

#include "finalization/src/chain/FinalizationHashTree.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS FinalizationHashTreeTests

	// region size / contains / addBranch

	TEST(TEST_CLASS, TreeIsInitiallyEmpty) {
		// Act:
		FinalizationHashTree tree;

		// Assert:
		EXPECT_EQ(0u, tree.size());
	}

	TEST(TEST_CLASS, ContainsReturnsTrueWhenAnyTreeNodeMatches) {
		// Arrange:
		auto hashes = test::GenerateRandomDataVector<Hash256>(3);

		FinalizationHashTree tree;
		tree.addBranch(Height(7), hashes.data(), hashes.size());

		// Sanity:
		EXPECT_EQ(3u, tree.size());

		// Act + Assert:
		EXPECT_TRUE(tree.contains({ Height(7), hashes[0] }));
		EXPECT_TRUE(tree.contains({ Height(8), hashes[1] }));
		EXPECT_TRUE(tree.contains({ Height(9), hashes[2] }));
	}

	TEST(TEST_CLASS, ContainsReturnsFalseWhenNoTreeNodeMatches) {
		// Arrange:
		auto hashes = test::GenerateRandomDataVector<Hash256>(3);
		auto otherHash = test::GenerateRandomByteArray<Hash256>();

		FinalizationHashTree tree;
		tree.addBranch(Height(7), hashes.data(), hashes.size());

		// Sanity:
		EXPECT_EQ(3u, tree.size());

		// Act + Assert:
		EXPECT_FALSE(tree.contains({ Height(11), hashes[0] })); // wrong height
		EXPECT_FALSE(tree.contains({ Height(8), otherHash })); // wrong hash
		EXPECT_FALSE(tree.contains({ Height(8), hashes[0] })); // wrong height or hash
		EXPECT_FALSE(tree.contains({ Height(11), otherHash })); // wrong height and hash
	}

	TEST(TEST_CLASS, CanAddNonOverlappingBranchesToTree) {
		// Arrange:
		auto hashes1 = test::GenerateRandomDataVector<Hash256>(3);
		auto hashes2 = test::GenerateRandomDataVector<Hash256>(4);

		// Act:
		FinalizationHashTree tree;
		tree.addBranch(Height(7), hashes1.data(), hashes1.size());
		tree.addBranch(Height(8), hashes2.data(), hashes2.size());

		// Assert:
		EXPECT_EQ(7u, tree.size());
		EXPECT_TRUE(tree.contains({ Height(7), hashes1[0] }));
		EXPECT_TRUE(tree.contains({ Height(8), hashes1[1] }));
		EXPECT_TRUE(tree.contains({ Height(9), hashes1[2] }));
		EXPECT_TRUE(tree.contains({ Height(8), hashes2[0] }));
		EXPECT_TRUE(tree.contains({ Height(9), hashes2[1] }));
		EXPECT_TRUE(tree.contains({ Height(10), hashes2[2] }));
		EXPECT_TRUE(tree.contains({ Height(11), hashes2[3] }));
	}

	TEST(TEST_CLASS, CanAddNonOverlappingBranchesToTreeWithSameHashesAtDifferentHeights) {
		// Arrange:
		auto hashes = test::GenerateRandomDataVector<Hash256>(3);

		// Act:
		FinalizationHashTree tree;
		tree.addBranch(Height(7), hashes.data(), hashes.size());
		tree.addBranch(Height(8), hashes.data(), hashes.size());

		// Assert:
		EXPECT_EQ(6u, tree.size());
		EXPECT_TRUE(tree.contains({ Height(7), hashes[0] }));
		EXPECT_TRUE(tree.contains({ Height(8), hashes[1] }));
		EXPECT_TRUE(tree.contains({ Height(9), hashes[2] }));
		EXPECT_TRUE(tree.contains({ Height(8), hashes[0] }));
		EXPECT_TRUE(tree.contains({ Height(9), hashes[1] }));
		EXPECT_TRUE(tree.contains({ Height(10), hashes[2] }));
	}

	template<typename TAction>
	void RunOverlappingBranchesTest(TAction action) {
		// Arrange:
		//  7 - 8 - 9 - 10      | (hashes1)
		//     ||| |||          | (common hashes)
		//      8 - 9 - 10 - 11 | (hashes2)
		auto hashes1 = test::GenerateRandomDataVector<Hash256>(4);
		auto hashes2 = test::GenerateRandomDataVector<Hash256>(4);
		hashes2[0] = hashes1[1];
		hashes2[1] = hashes1[2];

		FinalizationHashTree tree;
		tree.addBranch(Height(7), hashes1.data(), hashes1.size());
		tree.addBranch(Height(8), hashes2.data(), hashes2.size());

		// Act + Assert:
		action(tree, hashes1, hashes2);
	}

	TEST(TEST_CLASS, CanAddOverlappingBranchesToTree) {
		// Arrange:
		RunOverlappingBranchesTest([](const auto& tree, const auto& hashes1, const auto& hashes2) {
			// Assert:
			EXPECT_EQ(6u, tree.size());
			EXPECT_TRUE(tree.contains({ Height(7), hashes1[0] }));
			EXPECT_TRUE(tree.contains({ Height(8), hashes1[1] }));
			EXPECT_TRUE(tree.contains({ Height(9), hashes1[2] }));
			EXPECT_TRUE(tree.contains({ Height(10), hashes1[3] }));
			EXPECT_TRUE(tree.contains({ Height(10), hashes2[2] }));
			EXPECT_TRUE(tree.contains({ Height(11), hashes2[3] }));
		});
	}

	// endregion

	// region isDescendant

	TEST(TEST_CLASS, IsDescendantReturnsFalseForUnknownNode) {
		// Arrange:
		RunOverlappingBranchesTest([](const auto& tree, const auto& hashes1, const auto&) {
			// Act + Assert:
			EXPECT_FALSE(tree.isDescendant({ Height(9), hashes1[2] }, { Height(10), hashes1[2] })); // child unknown
			EXPECT_FALSE(tree.isDescendant({ Height(9), hashes1[1] }, { Height(10), hashes1[3] })); // parent unknown
			EXPECT_FALSE(tree.isDescendant({ Height(9), hashes1[1] }, { Height(10), hashes1[2] })); // both unknown (different)
			EXPECT_FALSE(tree.isDescendant({ Height(9), hashes1[1] }, { Height(9), hashes1[1] })); // both unknown (same)
		});
	}

	TEST(TEST_CLASS, IsDescendantReturnsFalseForAncestors) {
		// Arrange:
		RunOverlappingBranchesTest([](const auto& tree, const auto& hashes1, const auto& hashes2) {
			auto parentKey = model::HeightHashPair{ Height(9), hashes1[2] };

			// Act + Assert:
			EXPECT_FALSE(tree.isDescendant({ Height(10), hashes1[3] }, parentKey));
			EXPECT_FALSE(tree.isDescendant({ Height(10), hashes2[2] }, parentKey));
			EXPECT_FALSE(tree.isDescendant({ Height(11), hashes2[3] }, parentKey));
		});
	}

	TEST(TEST_CLASS, IsDescendantReturnsTrueForDescendantOfRootNode) {
		// Arrange:
		RunOverlappingBranchesTest([](const auto& tree, const auto& hashes1, const auto& hashes2) {
			auto parentKey = model::HeightHashPair{ Height(7), hashes1[0] };

			// Act + Assert:
			EXPECT_TRUE(tree.isDescendant(parentKey, { Height(7), hashes1[0] }));
			EXPECT_TRUE(tree.isDescendant(parentKey, { Height(8), hashes1[1] }));
			EXPECT_TRUE(tree.isDescendant(parentKey, { Height(9), hashes1[2] }));
			EXPECT_TRUE(tree.isDescendant(parentKey, { Height(10), hashes1[3] }));
			EXPECT_TRUE(tree.isDescendant(parentKey, { Height(10), hashes2[2] }));
			EXPECT_TRUE(tree.isDescendant(parentKey, { Height(11), hashes2[3] }));
		});
	}

	TEST(TEST_CLASS, IsDescendantReturnsTrueForDescendantOfArbitraryNode) {
		// Arrange:
		RunOverlappingBranchesTest([](const auto& tree, const auto& hashes1, const auto& hashes2) {
			auto parentKey = model::HeightHashPair{ Height(9), hashes1[2] };

			// Act + Assert:
			EXPECT_TRUE(tree.isDescendant(parentKey, { Height(9), hashes1[2] }));
			EXPECT_TRUE(tree.isDescendant(parentKey, { Height(10), hashes1[3] }));
			EXPECT_TRUE(tree.isDescendant(parentKey, { Height(10), hashes2[2] }));
			EXPECT_TRUE(tree.isDescendant(parentKey, { Height(11), hashes2[3] }));
		});
	}

	TEST(TEST_CLASS, IsDescendantReturnsTrueForDescendantOfLeafNode) {
		// Arrange:
		RunOverlappingBranchesTest([](const auto& tree, const auto&, const auto& hashes2) {
			auto parentKey = model::HeightHashPair{ Height(11), hashes2[3] };

			// Act + Assert:
			EXPECT_TRUE(tree.isDescendant(parentKey, { Height(11), hashes2[3] }));
		});
	}

	// endregion

	// region findAncestors

	TEST(TEST_CLASS, FindAncestorsReturnsEmptyForUnknownNode) {
		// Arrange:
		RunOverlappingBranchesTest([](const auto& tree, const auto&, const auto&) {
			// Act:
			auto ancestorKeys = tree.findAncestors({ Height(9), test::GenerateRandomByteArray<Hash256>() });

			// Assert:
			EXPECT_TRUE(ancestorKeys.empty());
		});
	}

	TEST(TEST_CLASS, FindAncestorsCanReturnNodesStartingAtRootNode) {
		// Arrange:
		RunOverlappingBranchesTest([](const auto& tree, const auto& hashes1, const auto&) {
			// Act:
			auto ancestorKeys = tree.findAncestors({ Height(7), hashes1[0] });

			// Assert:
			std::vector<model::HeightHashPair> expectedAncestorKeys{ { Height(7), hashes1[0] } };
			EXPECT_EQ(expectedAncestorKeys, ancestorKeys);
		});
	}

	TEST(TEST_CLASS, FindAncestorsCanReturnNodesStartingAtArbitraryNode) {
		// Arrange:
		RunOverlappingBranchesTest([](const auto& tree, const auto& hashes1, const auto&) {
			// Act:
			auto ancestorKeys = tree.findAncestors({ Height(9), hashes1[2] });

			// Assert:
			std::vector<model::HeightHashPair> expectedAncestorKeys{
				{ Height(9), hashes1[2] },
				{ Height(8), hashes1[1] },
				{ Height(7), hashes1[0] }
			};
			EXPECT_EQ(expectedAncestorKeys, ancestorKeys);
		});
	}

	TEST(TEST_CLASS, FindAncestorsCanReturnNodesStartingAtLeafNode) {
		// Arrange:
		RunOverlappingBranchesTest([](const auto& tree, const auto& hashes1, const auto& hashes2) {
			// Act:
			auto ancestorKeys = tree.findAncestors({ Height(11), hashes2[3] });

			// Assert:
			std::vector<model::HeightHashPair> expectedAncestorKeys{
				{ Height(11), hashes2[3] },
				{ Height(10), hashes2[2] },
				{ Height(9), hashes1[2] },
				{ Height(8), hashes1[1] },
				{ Height(7), hashes1[0] }
			};
			EXPECT_EQ(expectedAncestorKeys, ancestorKeys);
		});
	}

	// endregion
}}
