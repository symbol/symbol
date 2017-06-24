#include "src/state/RootNamespace.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"
#include <unordered_set>

namespace catapult { namespace state {

	namespace {
		using ChildNamespaces = test::ChildNamespaces;
	}

	/// region ctor

	TEST(RootNamespaceTests, CanCreateRootNamespace) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto id = test::GenerateRandomValue<NamespaceId>();

		// Act:
		RootNamespace root(id, owner, test::CreateLifetime(234, 321));

		// Assert:
		EXPECT_EQ(id, root.id());
		EXPECT_EQ(owner, root.owner());
		EXPECT_EQ(test::CreateLifetime(234, 321), root.lifetime());
		EXPECT_TRUE(root.empty());
		EXPECT_EQ(0u, root.size());
	}

	TEST(RootNamespaceTests, CanCreateRootNamespaceWithChildren) {
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
		root.add(state::Namespace(test::CreatePath({ 123, 124 })));
		root.add(state::Namespace(test::CreatePath({ 123, 125 })));
		root.add(state::Namespace(test::CreatePath({ 123, 124, 126 })));

		// Assert:
		EXPECT_EQ(id, root.id());
		EXPECT_EQ(owner, root.owner());
		EXPECT_EQ(test::CreateLifetime(234, 321), root.lifetime());
		EXPECT_EQ(3u, root.size());
		EXPECT_FALSE(root.empty());
		EXPECT_EQ(3u, root.children().size());
		test::AssertChildren(expectedChildren, root.children());
	}

	TEST(RootNamespaceTests, CanCopyConstructRootNamespaceWithoutChildren) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto id = test::GenerateRandomValue<NamespaceId>();
		RootNamespace original(id, owner, test::CreateLifetime(234, 321));

		// Act:
		RootNamespace root(original);

		// Assert:
		EXPECT_EQ(id, root.id());
		EXPECT_EQ(owner, root.owner());
		EXPECT_EQ(test::CreateLifetime(234, 321), root.lifetime());
		EXPECT_TRUE(root.empty());
		EXPECT_EQ(0u, root.size());
	}

	TEST(RootNamespaceTests, CanCopyConstructRootNamespaceWithChildren) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto id = NamespaceId(123);
		auto expectedChildren = test::CreateChildren({
			test::CreatePath({ 123, 124 }),
			test::CreatePath({ 123, 125 }),
			test::CreatePath({ 123, 124, 126 })
		});

		RootNamespace original(id, owner, test::CreateLifetime(234, 321));
		original.add(state::Namespace(test::CreatePath({ 123, 124 })));
		original.add(state::Namespace(test::CreatePath({ 123, 125 })));
		original.add(state::Namespace(test::CreatePath({ 123, 124, 126 })));

		// Act:
		RootNamespace root(original);

		// Assert:
		EXPECT_EQ(id, root.id());
		EXPECT_EQ(owner, root.owner());
		EXPECT_EQ(test::CreateLifetime(234, 321), root.lifetime());
		EXPECT_EQ(3u, root.size());
		EXPECT_EQ(3u, root.children().size());
		test::AssertChildren(expectedChildren, root.children());

		// copy constructor copies children
		EXPECT_NE(&original.children(), &root.children());
	}

	TEST(RootNamespaceTests, CanMoveConstructRootNamespaceWithoutChildren) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto id = test::GenerateRandomValue<NamespaceId>();
		RootNamespace original(id, owner, test::CreateLifetime(234, 321));

		// Act:
		RootNamespace root(std::move(original));

		// Assert:
		EXPECT_EQ(id, root.id());
		EXPECT_EQ(owner, root.owner());
		EXPECT_EQ(test::CreateLifetime(234, 321), root.lifetime());
		EXPECT_TRUE(root.empty());
		EXPECT_EQ(0u, root.size());
	}

	TEST(RootNamespaceTests, CanMoveConstructRootNamespaceWithChildren) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto id = NamespaceId(123);
		auto expectedChildren = test::CreateChildren({
			test::CreatePath({ 123, 124 }),
			test::CreatePath({ 123, 125 }),
			test::CreatePath({ 123, 124, 126 })
		});

		RootNamespace original(id, owner, test::CreateLifetime(234, 321));
		original.add(state::Namespace(test::CreatePath({ 123, 124 })));
		original.add(state::Namespace(test::CreatePath({ 123, 125 })));
		original.add(state::Namespace(test::CreatePath({ 123, 124, 126 })));
		const auto* pOriginalChildren = &original.children();

		// Act:
		RootNamespace root(std::move(original));

		// Assert:
		EXPECT_EQ(id, root.id());
		EXPECT_EQ(owner, root.owner());
		EXPECT_EQ(test::CreateLifetime(234, 321), root.lifetime());
		EXPECT_EQ(3u, root.size());
		EXPECT_EQ(3u, root.children().size());
		test::AssertChildren(expectedChildren, root.children());

		// move constructor does not copy children
		EXPECT_EQ(pOriginalChildren, &root.children());
	}

	// endregion

	namespace {
		auto CreateDefaultRoot(const Key& owner, NamespaceId::ValueType id) {
			return RootNamespace(NamespaceId(id), owner, test::CreateLifetime(234, 321));
		}

		auto CreateDefaultRootWithChildren(const Key& owner) {
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

	TEST(RootNamespaceTests, CanGetExistingChild) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act:
		auto childNamespace = root.child(NamespaceId(128));

		// Assert:
		EXPECT_EQ(test::CreatePath({ 123, 124, 128 }), childNamespace.path());
		EXPECT_EQ(root.id(), childNamespace.rootId());
	}

	TEST(RootNamespaceTests, CanGetExistingChildWithDescendents) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act:
		auto childNamespace = root.child(NamespaceId(124));

		// Assert:
		EXPECT_EQ(test::CreatePath({ 123, 124 }), childNamespace.path());
		EXPECT_EQ(root.id(), childNamespace.rootId());
	}

	TEST(RootNamespaceTests, CannotGetUnknownChild) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Assert:
		EXPECT_THROW(root.child(NamespaceId(1337)), catapult_invalid_argument);
	}

	// endregion

	// region add

	TEST(RootNamespaceTests, CanAddSingleChild) {
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

	TEST(RootNamespaceTests, CanAddMultipleChilddren) {
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

	TEST(RootNamespaceTests, CannotAddSameChildTwice) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRoot(owner, 123);
		root.add(Namespace(test::CreatePath({ 123, 124 })));

		// Assert:
		EXPECT_THROW(root.add(Namespace(test::CreatePath({ 123, 124 }))), catapult_invalid_argument);
	}

	TEST(RootNamespaceTests, CannotAddChildWithUnknownParent) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Assert: parent 129 does not exist
		EXPECT_THROW(root.add(Namespace(test::CreatePath({ 123, 129, 130 }))), catapult_invalid_argument);
	}

	TEST(RootNamespaceTests, CannotAddChildWithIncorrectPath) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Assert: parent 128 has level 2 but in child map parent has level 3
		EXPECT_THROW(root.add(Namespace(test::CreatePath({ 123, 128, 130 }))), catapult_invalid_argument);
	}

	TEST(RootNamespaceTests, CannotAddChildWithDifferentRoot) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Assert: root is different
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

	TEST(RootNamespaceTests, CanRemoveSingleChild) {
		// Assert:
		AssertCanRemoveChildren({ 128 });
	}

	TEST(RootNamespaceTests, CanRemoveMultipleChildren) {
		// Assert:
		AssertCanRemoveChildren({ 357, 128, 125 });
	}

	TEST(RootNamespaceTests, CanRemoveAllChildren) {
		// Assert:
		AssertCanRemoveChildren({ 357, 128, 125, 124 });
	}

	TEST(RootNamespaceTests, CannotRemoveUnknownChild) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Assert:
		EXPECT_THROW(root.remove(NamespaceId(1337)), catapult_invalid_argument);
	}

	TEST(RootNamespaceTests, CannotRemoveChildWithDescendents) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Assert:
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

	TEST(RootNamespaceTests, OperatorEqualReturnsTrueForEqualObjects) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(RootNamespaceTests, OperatorNotEqualReturnsTrueForUnequalObjects) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion

	// region renew

	TEST(RootNamespaceTests, CanRenewRoot) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = CreateDefaultRootWithChildren(owner);

		// Act:
		auto renewedRoot = root.renew(test::CreateLifetime(468, 579));

		// Assert:
		EXPECT_EQ(root.id(), renewedRoot.id());
		EXPECT_EQ(root, renewedRoot);
		EXPECT_EQ(test::CreateLifetime(468, 579), renewedRoot.lifetime());
		EXPECT_EQ(owner, renewedRoot.owner());
		EXPECT_EQ(4u, root.size());
		EXPECT_EQ(&root.children(), &renewedRoot.children());
	}

	// endregion
}}
