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

#include "src/state/RootNamespace.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"
#include <unordered_set>

namespace catapult { namespace state {

#define TEST_CLASS RootNamespaceTests

	namespace {
		using ChildNamespaces = test::ChildNamespaces;
	}

	// region ctor

	TEST(TEST_CLASS, CanCreateRootNamespace) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto id = test::GenerateRandomValue<NamespaceId>();

		// Act:
		RootNamespace root(id, owner, test::CreateLifetime(234, 321));

		// Assert:
		EXPECT_EQ(id, root.id());
		EXPECT_EQ(owner, root.ownerAddress());
		EXPECT_EQ(test::CreateLifetime(234, 321), root.lifetime());
		EXPECT_TRUE(root.empty());
		EXPECT_EQ(0u, root.size());
	}

	TEST(TEST_CLASS, CanCreateRootNamespaceWithChildren) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto id = NamespaceId(123);
		auto expectedChildren = test::CreateChildren({
			test::CreatePath({ 123, 124 }),
			test::CreatePath({ 123, 125 }),
			test::CreatePath({ 123, 124, 126 })
		});

		// Act:
		RootNamespace root(id, owner, test::CreateLifetime(234, 321));
		root.add(Namespace(test::CreatePath({ 123, 124 })));
		root.add(Namespace(test::CreatePath({ 123, 125 })));
		root.add(Namespace(test::CreatePath({ 123, 124, 126 })));

		// Assert:
		EXPECT_EQ(id, root.id());
		EXPECT_EQ(owner, root.ownerAddress());
		EXPECT_EQ(test::CreateLifetime(234, 321), root.lifetime());
		EXPECT_EQ(3u, root.size());
		EXPECT_FALSE(root.empty());
		EXPECT_EQ(3u, root.children().size());
		test::AssertChildren(expectedChildren, root.children());
	}

	TEST(TEST_CLASS, CanCreateRootNamespaceWithExistingChildren) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto id = NamespaceId(123);
		auto pExpectedChildren = std::make_shared<RootNamespace::Children>(test::CreateChildren({
			test::CreatePath({ 123, 124 }),
			test::CreatePath({ 123, 125 }),
			test::CreatePath({ 123, 124, 126 })
		}));

		// Act:
		RootNamespace root(id, owner, test::CreateLifetime(234, 321), pExpectedChildren);

		// Assert:
		EXPECT_EQ(id, root.id());
		EXPECT_EQ(owner, root.ownerAddress());
		EXPECT_EQ(test::CreateLifetime(234, 321), root.lifetime());
		EXPECT_EQ(3u, root.size());
		EXPECT_FALSE(root.empty());
		EXPECT_EQ(3u, root.children().size());
		EXPECT_EQ(pExpectedChildren.get(), &root.children());
		test::AssertChildren(*pExpectedChildren, root.children());
	}

	// endregion

	namespace {
		auto CreateDefaultRoot(const Address& owner, NamespaceId::ValueType id) {
			return RootNamespace(NamespaceId(id), owner, test::CreateLifetime(234, 321));
		}

		auto CreateDefaultRootWithChildren(const Address& owner) {
			auto root = CreateDefaultRoot(owner, 123);
			auto children = test::CreateChildren({
				test::CreatePath({ 123, 357 }),
				test::CreatePath({ 123, 124 }),
				test::CreatePath({ 123, 124, 125 }),
				test::CreatePath({ 123, 124, 128 })
			});

			test::AddAll(root, children, { 357, 124, 125, 128 });

			// Sanity:
			EXPECT_EQ(4u, root.children().size());
			test::AssertChildren(children, root.children());
			return root;
		}
	}

	// region child

	TEST(TEST_CLASS, CanGetExistingChild) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act:
		auto childNamespace = root.child(NamespaceId(128));

		// Assert:
		EXPECT_EQ(test::CreatePath({ 123, 124, 128 }), childNamespace.path());
		EXPECT_EQ(root.id(), childNamespace.rootId());
	}

	TEST(TEST_CLASS, CanGetExistingChildWithDescendents) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act:
		auto childNamespace = root.child(NamespaceId(124));

		// Assert:
		EXPECT_EQ(test::CreatePath({ 123, 124 }), childNamespace.path());
		EXPECT_EQ(root.id(), childNamespace.rootId());
	}

	TEST(TEST_CLASS, CannotGetUnknownChild) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act + Assert:
		EXPECT_THROW(root.child(NamespaceId(1337)), catapult_invalid_argument);
	}

	// endregion

	// region add

	TEST(TEST_CLASS, CanAddSingleChild) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRoot(owner, 123);
		auto children = test::CreateChildren({ test::CreatePath({ 123, 124 }) });

		// Act:
		root.add(Namespace(test::CreatePath({ 123, 124 })));

		// Assert:
		EXPECT_EQ(1u, root.children().size());
		test::AssertChildren(children, root.children());
	}

	TEST(TEST_CLASS, CanAddMultipleChilddren) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRoot(owner, 123);
		auto children = test::CreateChildren({
			test::CreatePath({ 123, 357 }),
			test::CreatePath({ 123, 124 }),
			test::CreatePath({ 123, 124, 125 }),
			test::CreatePath({ 123, 124, 128 })
		});

		// Act:
		test::AddAll(root, children, { 357, 124, 125, 128 });

		// Assert:
		EXPECT_EQ(4u, root.children().size());
		test::AssertChildren(children, root.children());
	}

	TEST(TEST_CLASS, CannotAddSameChildTwice) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRoot(owner, 123);
		root.add(Namespace(test::CreatePath({ 123, 124 })));

		// Act + Assert:
		EXPECT_THROW(root.add(Namespace(test::CreatePath({ 123, 124 }))), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotAddChildWithUnknownParent) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act + Assert: parent 129 does not exist
		EXPECT_THROW(root.add(Namespace(test::CreatePath({ 123, 129, 130 }))), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotAddChildWithIncorrectPath) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act + Assert: parent 128 has level 2 but in child map parent has level 3
		EXPECT_THROW(root.add(Namespace(test::CreatePath({ 123, 128, 130 }))), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotAddChildWithDifferentRoot) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act + Assert: root is different
		EXPECT_THROW(root.add(Namespace(test::CreatePath({ 122, 124, 136 }))), catapult_invalid_argument);
	}

	// endregion

	// region remove

	namespace {
		void AssertCanRemoveChildren(const std::vector<NamespaceId::ValueType>& idsToRemove) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			auto root = CreateDefaultRoot(owner, 123);
			auto children = test::CreateChildren({
				test::CreatePath({ 123, 357 }),
				test::CreatePath({ 123, 124 }),
				test::CreatePath({ 123, 124, 125 }),
				test::CreatePath({ 123, 124, 128 })
			});

			test::AddAll(root, children, { 357, 124, 125, 128 });

			// Sanity:
			EXPECT_EQ(4u, root.children().size());
			test::AssertChildren(children, root.children());

			// Act:
			for (auto id : idsToRemove) {
				root.remove(NamespaceId(id));
				children.erase(NamespaceId(id));
			}

			// Assert:
			EXPECT_EQ(4u - idsToRemove.size(), root.children().size());
			test::AssertChildren(children, root.children());
		}
	}

	TEST(TEST_CLASS, CanRemoveSingleChild) {
		AssertCanRemoveChildren({ 128 });
	}

	TEST(TEST_CLASS, CanRemoveMultipleChildren) {
		AssertCanRemoveChildren({ 357, 128, 125 });
	}

	TEST(TEST_CLASS, CanRemoveAllChildren) {
		AssertCanRemoveChildren({ 357, 128, 125, 124 });
	}

	TEST(TEST_CLASS, CannotRemoveUnknownChild) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act + Assert:
		EXPECT_THROW(root.remove(NamespaceId(1337)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotRemoveChildWithDescendents) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act + Assert:
		EXPECT_THROW(root.remove(NamespaceId(124)), catapult_invalid_argument);
	}

	// endregion

	// region equality

	namespace {
		const char* Default_Key = "default";

		auto GenerateEqualityInstanceMap() {
			std::unordered_map<std::string, RootNamespace> map;
			auto owner = test::CreateRandomOwner();
			auto diffOwner = test::CreateRandomOwner();

			map.emplace(Default_Key, RootNamespace(NamespaceId(123), owner, test::CreateLifetime(234, 321)));
			map.emplace("copy", RootNamespace(NamespaceId(123), owner, test::CreateLifetime(234, 321)));
			map.emplace("diff-lifetime", RootNamespace(NamespaceId(123), owner, test::CreateLifetime(456, 654)));
			map.emplace("diff-id", RootNamespace(NamespaceId(234), owner, test::CreateLifetime(234, 321)));
			map.emplace("diff-owner", RootNamespace(NamespaceId(123), diffOwner, test::CreateLifetime(234, 321)));
			return map;
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { Default_Key, "copy", "diff-lifetime" };
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueForEqualObjects) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion

	// region renew

	TEST(TEST_CLASS, CanRenewRoot) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act:
		auto renewedRoot = root.renew(test::CreateLifetime(468, 579));

		// Assert:
		EXPECT_EQ(root.id(), renewedRoot.id());
		EXPECT_EQ(root, renewedRoot);
		EXPECT_EQ(test::CreateLifetime(468, 579), renewedRoot.lifetime());
		EXPECT_EQ(owner, renewedRoot.ownerAddress());
		EXPECT_EQ(4u, root.size());
		EXPECT_EQ(&root.children(), &renewedRoot.children());
	}

	// endregion

	// region sortedChildPaths

	namespace {
		void SeedWithOutOfOrderChildren(RootNamespace& root) {
			std::vector<std::vector<NamespaceId::ValueType>> paths{
				{ 123, 753 },
				{ 123, 753, 129 },
				{ 123, 753, 127 },
				{ 123, 124 },
				{ 123, 124, 122 },
				{ 123, 124, 121 }
			};

			for (const auto& rawPath : paths)
				root.add(Namespace(test::CreatePath(rawPath)));
		}
	}

	TEST(TEST_CLASS, CanSortChildPaths) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRoot(owner, 123);
		SeedWithOutOfOrderChildren(root);

		// Sanity: child with path size 3 is first in map
		const auto& children = root.children();
		EXPECT_EQ(3u, children.cbegin()->second.Path.size());

		// Act:
		auto sortedChildPaths = root.sortedChildPaths();

		// Assert:
		Namespace::Path path;
		for (const auto& childPath : sortedChildPaths) {
			EXPECT_TRUE(std::lexicographical_compare(path.cbegin(), path.cend(), childPath.cbegin(), childPath.cend()));
			path = childPath;
		}
	}

	// endregion

	// region alias (unset)

	TEST(TEST_CLASS, RootAliasIsInitiallyUnset) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act:
		const auto& alias = root.alias(NamespaceId(123));

		// Assert:
		EXPECT_EQ(AliasType::None, alias.type());
	}

	TEST(TEST_CLASS, ChildAliasIsInitiallyUnset) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act:
		const auto& alias = root.alias(NamespaceId(125));

		// Assert:
		EXPECT_EQ(AliasType::None, alias.type());
	}

	TEST(TEST_CLASS, CannotGetAliasForUnknownNamespace) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act + Assert:
		EXPECT_THROW(root.alias(NamespaceId(127)), catapult_invalid_argument);
	}

	// endregion

	// region setAlias

	namespace {
		using NamespaceIdSet = std::unordered_set<NamespaceId, utils::BaseValueHasher<NamespaceId>>;

		NamespaceIdSet GetChildNamespaceIdsWithAliases(const RootNamespace& root) {
			NamespaceIdSet namespaceIds;
			for (const auto& pair : root.children()) {
				if (AliasType::None != pair.second.Alias.type())
					namespaceIds.insert(pair.first);
			}

			return namespaceIds;
		}
	}

	TEST(TEST_CLASS, CanSetRootAlias) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act:
		root.setAlias(NamespaceId(123), NamespaceAlias(MosaicId(444)));
		auto alias = root.alias(NamespaceId(123));

		// Assert:
		EXPECT_EQ(AliasType::Mosaic, alias.type());
		EXPECT_EQ(MosaicId(444), alias.mosaicId());

		// - no child namespace aliases
		EXPECT_EQ(NamespaceIdSet(), GetChildNamespaceIdsWithAliases(root));
	}

	TEST(TEST_CLASS, CanSetChildAlias) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act:
		root.setAlias(NamespaceId(125), NamespaceAlias(MosaicId(444)));
		auto alias = root.alias(NamespaceId(125));

		// Assert:
		EXPECT_EQ(AliasType::Mosaic, alias.type());
		EXPECT_EQ(MosaicId(444), alias.mosaicId());

		// - aliases are accessible via children
		EXPECT_EQ(NamespaceIdSet{ NamespaceId(125) }, GetChildNamespaceIdsWithAliases(root));
	}

	TEST(TEST_CLASS, CannotSetAliasForUnknownNamespace) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act + Assert:
		EXPECT_THROW(root.setAlias(NamespaceId(127), NamespaceAlias(MosaicId(444))), catapult_invalid_argument);
	}

	// endregion
}}
