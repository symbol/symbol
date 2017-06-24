#include "src/state/NamespaceEntry.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

	// region compatible

	TEST(NamespaceEntryTests, CanCreateNamespaceEntryAroundRoot) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = RootNamespace(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		auto ns = state::Namespace(test::CreatePath({ 123 }));

		// Act:
		auto entry = NamespaceEntry(ns, root);

		// Assert:
		EXPECT_EQ(ns, entry.ns());
		EXPECT_EQ(root, entry.root());
		EXPECT_TRUE(entry.ns().isRoot());
	}

	TEST(NamespaceEntryTests, CanCreateNamespaceEntryAroundChild) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = RootNamespace(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		auto ns = state::Namespace(test::CreatePath({ 123, 234, 456 }));

		// Act:
		auto entry = NamespaceEntry(ns, root);

		// Assert:
		EXPECT_EQ(ns, entry.ns());
		EXPECT_EQ(root, entry.root());
		EXPECT_FALSE(entry.ns().isRoot());
	}

	// endregion

	// region incompatible

	TEST(NamespaceEntryTests, CannotCreateNamespaceEntryAroundIncompatibleRoot) {
		// Arrange: root.id() != ns.rootId()
		auto owner = test::CreateRandomOwner();
		auto root = RootNamespace(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		auto ns = state::Namespace(test::CreatePath({ 124 }));

		// Act:
		EXPECT_THROW(NamespaceEntry(ns, root), catapult_invalid_argument);
	}

	TEST(NamespaceEntryTests, CannotCreateNamespaceEntryAroundIncompatibleChild) {
		// Arrange: root.id() != ns.rootId()
		auto owner = test::CreateRandomOwner();
		auto root = RootNamespace(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		auto ns = state::Namespace(test::CreatePath({ 124, 234, 456 }));

		// Act:
		EXPECT_THROW(NamespaceEntry(ns, root), catapult_invalid_argument);
	}

	// endregion
}}
