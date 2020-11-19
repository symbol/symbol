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

#include "src/state/Namespace.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS NamespaceTests

	// region ctor

	TEST(TEST_CLASS, CanCreateLevelOneNamespace) {
		// Arrange:
		auto path = test::CreatePath({ test::Default_Root_Id });

		// Act:
		Namespace ns(path);

		// Assert:
		EXPECT_EQ(path[0], ns.id());
		EXPECT_EQ(Namespace_Base_Id, ns.parentId());
		EXPECT_EQ(path[0], ns.rootId());
		EXPECT_TRUE(ns.isRoot());
		EXPECT_EQ(test::CreatePath({ test::Default_Root_Id }), ns.path());
	}

	TEST(TEST_CLASS, CanCreateHigherLevelNamespace) {
		// Arrange:
		auto path = test::CreatePath({ test::Default_Root_Id, 357, 468 });

		// Act:
		Namespace ns(path);

		// Assert:
		EXPECT_EQ(path[2], ns.id());
		EXPECT_EQ(path[1], ns.parentId());
		EXPECT_EQ(path[0], ns.rootId());
		EXPECT_FALSE(ns.isRoot());
		EXPECT_EQ(test::CreatePath({ test::Default_Root_Id, 357, 468 }), ns.path());
	}

	TEST(TEST_CLASS, CannotCreateNamespaceWithZeroSizedPath) {
		// Arrange:
		auto path = test::CreatePath({});

		// Act + Assert:
		EXPECT_THROW(Namespace ns(path), catapult_out_of_range);
	}

	// endregion

	// region createChild

	TEST(TEST_CLASS, CanCreateChild) {
		// Arrange:
		auto path = test::CreatePath({ test::Default_Root_Id, 357, 468 });
		Namespace ns(path);

		// Act:
		auto child = ns.createChild(NamespaceId(123));

		// Assert:
		EXPECT_EQ(NamespaceId(123), child.id());
		EXPECT_EQ(ns.id(), child.parentId());
		EXPECT_EQ(ns.rootId(), child.rootId());
		EXPECT_FALSE(child.isRoot());
		EXPECT_EQ(test::CreatePath({ test::Default_Root_Id, 357, 468, 123 }), child.path());
	}

	// endregion

	// region equality

	namespace {
		const char* Default_Key = "default";

		auto CreateDeterministicPath(uint8_t depth) {
			Namespace::Path path;
			for (auto i = 0u; i < depth; ++i)
				path.push_back(NamespaceId(i + 5));

			return path;
		}

		auto GenerateEqualityInstanceMap() {
			std::unordered_map<std::string, Namespace> map;
			auto path = CreateDeterministicPath(3);

			map.emplace(Default_Key, Namespace(path));
			map.emplace("copy", Namespace(path));
			map.emplace("diff-path", Namespace(test::CreatePath({ 5, 2, 1 })));
			return map;
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { Default_Key, "copy" };
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueForEqualObjects) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
