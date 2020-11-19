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

#include "catapult/tree/TreeNodePath.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace tree {

#define TEST_CLASS TreeNodePathTests

	namespace {
		void AssertPath(const TreeNodePath& path, size_t expectedSize, std::initializer_list<uint8_t> expectedNibbles) {
			// Assert:
			EXPECT_EQ(0 == expectedSize, path.empty());
			ASSERT_EQ(expectedSize, path.size());

			auto i = 0u;
			for (auto expectedNibble : expectedNibbles) {
				EXPECT_EQ(expectedNibble, path.nibbleAt(i)) << "nibble at index " << i;
				++i;
			}
		}
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateEmptyPath) {
		// Act:
		TreeNodePath path;

		// Assert:
		AssertPath(path, 0, {});
	}

	TEST(TEST_CLASS, CanCreatePathAroundIntegralKey) {
		// Act:
		TreeNodePath path(static_cast<uint32_t>(0x12C05437));

		// Assert:
		AssertPath(path, 8, { 1, 2, 0xC, 0, 5, 4, 3, 7 });
	}

	TEST(TEST_CLASS, CanCreatePathAroundArrayKey) {
		// Act:
		TreeNodePath path(std::array<uint8_t, 4>{ { 0x12, 0xC0, 0x54, 0x37 } });

		// Assert:
		AssertPath(path, 8, { 1, 2, 0xC, 0, 5, 4, 3, 7 });
	}

	TEST(TEST_CLASS, CanCreatePathAroundVectorKey) {
		// Act:
		TreeNodePath path(std::vector<uint8_t>{ { 0x12, 0xC0, 0x54, 0x37 } });

		// Assert:
		AssertPath(path, 8, { 1, 2, 0xC, 0, 5, 4, 3, 7 });
	}

	TEST(TEST_CLASS, CanCreatePathAroundEmptyVectorKey) {
		// Act:
		TreeNodePath path(std::vector<uint8_t>{});

		// Assert:
		AssertPath(path, 0, {});
	}

	// endregion

	// region equality

	namespace {
		template<typename TEqualityTest>
		void RunEqualityTest(TEqualityTest test) {
			// Arrange:
			using KeyType = std::array<uint8_t, 12>;
			TreeNodePath path(KeyType{ { 0x00, 0x11, 0x22, 0x33, 0xFF, 0xDD, 0xBB, 0x99, 0x76, 0x98, 0x12, 0x34 } });
			TreeNodePath nibbleShiftedPath(KeyType{ { 0x01, 0x12, 0x23, 0x3F, 0xFD, 0xDB, 0xB9, 0x97, 0x69, 0x81, 0x23, 0x40 } });
			std::unordered_map<std::string, TreeNodePath> equalityMap{
				{ "default", path.subpath(2, 12) },
				{ "copy", path.subpath(2, 12) },
				{ "diff adjustment", nibbleShiftedPath.subpath(1, 12) },
				{ "shorter", path.subpath(2, 11) },
				{ "longer", path.subpath(2, 14) },
				{ "different", path.subpath(3, 12) }
			};

			// Assert:
			test("default", equalityMap, std::unordered_set<std::string>{ "default", "copy", "diff adjustment" });
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueForEqualObjects) {
		RunEqualityTest([](const auto& defaultKey, const auto& descToEntityMap, const auto& equalityTags) {
			test::AssertOperatorEqualReturnsTrueForEqualObjects(defaultKey, descToEntityMap, equalityTags);
		});
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		RunEqualityTest([](const auto& defaultKey, const auto& descToEntityMap, const auto& equalityTags) {
			test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(defaultKey, descToEntityMap, equalityTags);
		});
	}

	// endregion

	// region subpath

	TEST(TEST_CLASS, CanCreateEmptySubpathFromEmptyPath) {
		// Act:
		TreeNodePath path;
		auto subpath = path.subpath(0);

		// Assert:
		AssertPath(subpath, 0, {});
	}

	TEST(TEST_CLASS, CanCreateEmptySubpathFromPath) {
		// Act:
		TreeNodePath path(static_cast<uint32_t>(0x12C05437));
		auto subpath = path.subpath(3, 0);

		// Assert:
		AssertPath(subpath, 0, {});
	}

	TEST(TEST_CLASS, CanCreateCopyViaSubpath) {
		// Act:
		TreeNodePath path(static_cast<uint32_t>(0x12C05437));
		auto subpath = path.subpath(0);

		// Assert:
		AssertPath(subpath, 8, { 1, 2, 0xC, 0, 5, 4, 3, 7 });
	}

	TEST(TEST_CLASS, CanCreateSubpathWithEvenOffsetAndRemainingSize) {
		// Act:
		TreeNodePath path(static_cast<uint32_t>(0x12C05437));
		auto subpath = path.subpath(6);

		// Assert:
		AssertPath(subpath, 2, { 3, 7 });
	}

	TEST(TEST_CLASS, CanCreateSubpathWithEvenOffsetAndPartialSize) {
		// Act:
		TreeNodePath path(static_cast<uint32_t>(0x12C05437));
		auto subpath = path.subpath(4, 3);

		// Assert:
		AssertPath(subpath, 3, { 5, 4, 3 });
	}

	TEST(TEST_CLASS, CanCreateSubpathWithOddOffsetAndRemainingSize) {
		// Act:
		TreeNodePath path(static_cast<uint32_t>(0x12C05437));
		auto subpath = path.subpath(5);

		// Assert:
		AssertPath(subpath, 3, { 4, 3, 7});
	}

	TEST(TEST_CLASS, CanCreateSubpathWithOddOffsetAndPartialSize) {
		// Act:
		TreeNodePath path(static_cast<uint32_t>(0x12C05437));
		auto subpath = path.subpath(1, 4);

		// Assert:
		AssertPath(subpath, 4, { 2, 0xC, 0, 5 });
	}

	TEST(TEST_CLASS, CanTakeMultipleSubpaths) {
		// Act:
		TreeNodePath path(static_cast<uint32_t>(0x01234596));
		auto path1 = path.subpath(1);
		auto path2 = path1.subpath(1);
		auto path3 = path2.subpath(1);
		auto path4 = path3.subpath(1);

		// Assert:
		AssertPath(path, 8, { 0, 1, 2, 3, 4, 5, 9, 6 });
		AssertPath(path1, 7, { 1, 2, 3, 4, 5, 9, 6 });
		AssertPath(path2, 6, { 2, 3, 4, 5, 9, 6 });
		AssertPath(path3, 5, { 3, 4, 5, 9, 6 });
		AssertPath(path4, 4, { 4, 5, 9, 6 });
	}

	// endregion

	// region Join

	TEST(TEST_CLASS, CanJoinTwoEvenLengthPaths) {
		// Arrange:
		TreeNodePath path1(static_cast<uint32_t>(0x01234596));
		TreeNodePath path2(static_cast<uint16_t>(0x5437));

		// Act:
		auto joinedPath1 = TreeNodePath::Join(path1, path2);
		auto joinedPath2 = TreeNodePath::Join(path2, path1);

		// Assert:
		AssertPath(joinedPath1, 12, { 0, 1, 2, 3, 4, 5, 9, 6, 5, 4, 3, 7 });
		AssertPath(joinedPath2, 12, { 5, 4, 3, 7, 0, 1, 2, 3, 4, 5, 9, 6 });
	}

	TEST(TEST_CLASS, CanJoinTwoOddLengthPaths) {
		// Arrange:
		TreeNodePath path1(static_cast<uint32_t>(0x01234596));
		TreeNodePath path2(static_cast<uint16_t>(0x5437));

		// Act:
		auto joinedPath1 = TreeNodePath::Join(path1.subpath(1), path2.subpath(1));
		auto joinedPath2 = TreeNodePath::Join(path2.subpath(1), path1.subpath(1));

		// Assert:
		AssertPath(joinedPath1, 10, { 1, 2, 3, 4, 5, 9, 6, 4, 3, 7 });
		AssertPath(joinedPath2, 10, { 4, 3, 7, 1, 2, 3, 4, 5, 9, 6 });
	}

	TEST(TEST_CLASS, CanJoinEvenAndOddLengthPaths) {
		// Arrange:
		TreeNodePath path1(static_cast<uint32_t>(0x01234596));
		TreeNodePath path2(static_cast<uint16_t>(0x5437));

		// Act:
		auto joinedPath1 = TreeNodePath::Join(path1, path2.subpath(1));
		auto joinedPath2 = TreeNodePath::Join(path2.subpath(1), path1);

		// Assert:
		AssertPath(joinedPath1, 11, { 0, 1, 2, 3, 4, 5, 9, 6, 4, 3, 7 });
		AssertPath(joinedPath2, 11, { 4, 3, 7, 0, 1, 2, 3, 4, 5, 9, 6 });
	}

	TEST(TEST_CLASS, CanJoinTwoEvenLengthPathsAndNibble) {
		// Arrange:
		TreeNodePath path1(static_cast<uint32_t>(0x01234596));
		TreeNodePath path2(static_cast<uint16_t>(0x5437));

		// Act:
		auto joinedPath1 = TreeNodePath::Join(path1, 0xDE, path2);
		auto joinedPath2 = TreeNodePath::Join(path2, 0xDE, path1);

		// Assert:
		AssertPath(joinedPath1, 13, { 0, 1, 2, 3, 4, 5, 9, 6, 0xE, 5, 4, 3, 7 });
		AssertPath(joinedPath2, 13, { 5, 4, 3, 7, 0xE, 0, 1, 2, 3, 4, 5, 9, 6 });
	}

	TEST(TEST_CLASS, CanJoinTwoOddLengthPathsAndNibble) {
		// Arrange:
		TreeNodePath path1(static_cast<uint32_t>(0x01234596));
		TreeNodePath path2(static_cast<uint16_t>(0x5437));

		// Act:
		auto joinedPath1 = TreeNodePath::Join(path1.subpath(1), 0xDE, path2.subpath(1));
		auto joinedPath2 = TreeNodePath::Join(path2.subpath(1), 0xDE, path1.subpath(1));

		// Assert:
		AssertPath(joinedPath1, 11, { 1, 2, 3, 4, 5, 9, 6, 0xE, 4, 3, 7 });
		AssertPath(joinedPath2, 11, { 4, 3, 7, 0xE, 1, 2, 3, 4, 5, 9, 6 });
	}

	TEST(TEST_CLASS, CanJoinEvenAndOddLengthPathsAndNibble) {
		// Arrange:
		TreeNodePath path1(static_cast<uint32_t>(0x01234596));
		TreeNodePath path2(static_cast<uint16_t>(0x5437));

		// Act:
		auto joinedPath1 = TreeNodePath::Join(path1, 0xDE, path2.subpath(1));
		auto joinedPath2 = TreeNodePath::Join(path2.subpath(1), 0xDE, path1);

		// Assert:
		AssertPath(joinedPath1, 12, { 0, 1, 2, 3, 4, 5, 9, 6, 0xE, 4, 3, 7 });
		AssertPath(joinedPath2, 12, { 4, 3, 7, 0xE, 0, 1, 2, 3, 4, 5, 9, 6 });
	}

	// endregion

	// region insertion operator

	TEST(TEST_CLASS, CanOutputEmptyPath) {
		// Act:
		TreeNodePath path;
		auto str = test::ToString(path);

		// Assert:
		EXPECT_EQ("( )", str);
	}

	TEST(TEST_CLASS, CanOutputFullPath) {
		// Act:
		TreeNodePath path(static_cast<uint32_t>(0x12C05437));
		auto str = test::ToString(path);

		// Assert:
		EXPECT_EQ("( 1 2 C 0 5 4 3 7 )", str);
	}

	TEST(TEST_CLASS, CanOutputPathWithEvenOffsetAndPartialSize) {
		// Act:
		TreeNodePath path(static_cast<uint32_t>(0x12C05437));
		auto subpath = path.subpath(4, 3);
		auto str = test::ToString(subpath);

		// Assert:
		EXPECT_EQ("( 5 4 3 )", str);
	}

	TEST(TEST_CLASS, CanOutputPathWithOddOffsetAndPartialSize) {
		// Act:
		TreeNodePath path(static_cast<uint32_t>(0x12C05437));
		auto subpath = path.subpath(1, 4);
		auto str = test::ToString(subpath);

		// Assert:
		EXPECT_EQ("( 2 C 0 5 )", str);
	}

	// endregion

	// region FindFirstDifferenceIndex

	namespace {
		void AssertDifferenceIndex(const TreeNodePath& path1, const TreeNodePath& path2, size_t expectedIndex) {
			// Assert:
			EXPECT_EQ(expectedIndex, FindFirstDifferenceIndex(path1, path2));
			EXPECT_EQ(expectedIndex, FindFirstDifferenceIndex(path2, path1));
		}
	}

	TEST(TEST_CLASS, FindFirstDifferenceIndexCanCompareEmptyPaths) {
		// Arrange:
		TreeNodePath path1;
		TreeNodePath path2;

		// Assert:
		AssertDifferenceIndex(path1, path2, 0);
	}

	TEST(TEST_CLASS, FindFirstDifferenceIndexCanCompareSamePathInstance) {
		// Arrange:
		TreeNodePath path(std::array<uint8_t, 12>{ { 0x00, 0x11, 0x22, 0x33, 0xFF, 0xDD, 0xBB, 0x99, 0x76, 0x98, 0x12, 0x34 } });

		// Assert:
		AssertDifferenceIndex(path, path, 24);
	}

	TEST(TEST_CLASS, FindFirstDifferenceIndexCanCompareIdenticalPaths) {
		// Arrange:
		auto pathArray = std::array<uint8_t, 12>{ { 0x00, 0x11, 0x22, 0x33, 0xFF, 0xDD, 0xBB, 0x99, 0x76, 0x98, 0x12, 0x34 } };
		TreeNodePath path1(pathArray);
		TreeNodePath path2(pathArray);

		// Assert:
		AssertDifferenceIndex(path1, path2, 24);
	}

	TEST(TEST_CLASS, FindFirstDifferenceIndexCanComparePathsWithDifferentSizes) {
		// Arrange:
		TreeNodePath path(std::array<uint8_t, 12>{ { 0x00, 0x11, 0x22, 0x33, 0xFF, 0xDD, 0xBB, 0x99, 0x76, 0x98, 0x12, 0x34 } });

		// Assert:
		AssertDifferenceIndex(path, path.subpath(0, 8), 8);
	}

	TEST(TEST_CLASS, FindFirstDifferenceIndexCanComparePathsWithDifferentContents) {
		// Arrange:
		TreeNodePath path1(std::array<uint8_t, 12>{ { 0x00, 0x11, 0x22, 0x33, 0xFF, 0xDD, 0xBB, 0x99, 0x76, 0x98, 0x12, 0x34 } });
		TreeNodePath path2(std::array<uint8_t, 12>{ { 0x00, 0x11, 0x22, 0x33, 0xFF, 0xDD, 0xBC, 0x99, 0x76, 0x98, 0x12, 0x34 } });

		// Assert:
		AssertDifferenceIndex(path1, path2, 13);
	}

	// endregion
}}
