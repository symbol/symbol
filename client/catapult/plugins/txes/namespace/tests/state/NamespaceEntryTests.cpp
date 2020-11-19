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

#include "src/state/NamespaceEntry.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS NamespaceEntryTests

	// region basic

	TEST(TEST_CLASS, CanCreateUninitializedNamespaceEntry) {
		// Act + Assert: no exceptions
		NamespaceEntry();
	}

	// endregion

	// region compatible

	TEST(TEST_CLASS, CanCreateNamespaceEntryAroundRoot) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto root = RootNamespace(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		auto ns = Namespace(test::CreatePath({ 123 }));

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
		auto ns = Namespace(test::CreatePath({ 123, 234, 456 }));

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
		auto ns = Namespace(test::CreatePath({ 124 }));

		// Act + Assert:
		EXPECT_THROW(NamespaceEntry(ns, root), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotCreateNamespaceEntryAroundIncompatibleChild) {
		// Arrange: root.id() != ns.rootId()
		auto owner = test::CreateRandomOwner();
		auto root = RootNamespace(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		auto ns = Namespace(test::CreatePath({ 124, 234, 456 }));

		// Act + Assert:
		EXPECT_THROW(NamespaceEntry(ns, root), catapult_invalid_argument);
	}

	// endregion
}}
