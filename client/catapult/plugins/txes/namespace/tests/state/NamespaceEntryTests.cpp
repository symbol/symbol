#include "src/state/NamespaceEntry.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS NamespaceEntryTests

	// region compatible

	TEST(TEST_CLASS, CanCreateNamespaceEntryAroundRoot) {
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

	TEST(TEST_CLASS, CanCreateNamespaceEntryAroundChild) {
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

	TEST(TEST_CLASS, CannotCreateNamespaceEntryAroundIncompatibleRoot) {
		// Arrange: root.id() != ns.rootId()
		auto owner = test::CreateRandomOwner();
		auto root = RootNamespace(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		auto ns = state::Namespace(test::CreatePath({ 124 }));

		// Act + Assert:
		EXPECT_THROW(NamespaceEntry(ns, root), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotCreateNamespaceEntryAroundIncompatibleChild) {
		// Arrange: root.id() != ns.rootId()
		auto owner = test::CreateRandomOwner();
		auto root = RootNamespace(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		auto ns = state::Namespace(test::CreatePath({ 124, 234, 456 }));

		// Act + Assert:
		EXPECT_THROW(NamespaceEntry(ns, root), catapult_invalid_argument);
	}

	// endregion
}}
