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
		auto path = test::CreatePath({ test::Default_Root_Id, 357 });
		Namespace ns(path);

		// Act:
		auto child = ns.createChild(NamespaceId(123));

		// Assert:
		EXPECT_EQ(NamespaceId(123), child.id());
		EXPECT_EQ(ns.id(), child.parentId());
		EXPECT_EQ(ns.rootId(), child.rootId());
		EXPECT_FALSE(child.isRoot());
		EXPECT_EQ(test::CreatePath({ test::Default_Root_Id, 357, 123 }), child.path());
	}

	TEST(TEST_CLASS, CannotCreateChildIfMaxDepthIsExceeded) {
		// Arrange:
		auto path = test::CreatePath({ test::Default_Root_Id, 357, 468 });
		Namespace ns(path);

		// Act + Assert:
		EXPECT_THROW(ns.createChild(NamespaceId(123)), catapult_out_of_range);
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
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
