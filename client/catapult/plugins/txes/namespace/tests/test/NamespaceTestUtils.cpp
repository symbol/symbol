#include "NamespaceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	Key CreateRandomOwner() {
		return test::GenerateRandomData<Key_Size>();
	}

	state::Namespace::Path CreatePath(const std::vector<NamespaceId::ValueType>& ids) {
		state::Namespace::Path path;
		for (auto id : ids)
			path.push_back(NamespaceId(id));

		return path;
	}

	state::NamespaceLifetime CreateLifetime(Height::ValueType start, Height::ValueType end) {
		return state::NamespaceLifetime(Height(start), Height(end));
	}

	void AddAll(state::RootNamespace& root, const ChildNamespaces& children, std::initializer_list<NamespaceId::ValueType> orderedIds) {
		for (auto childId : orderedIds)
			root.add(state::Namespace(children.at(NamespaceId(childId))));
	}

	ChildNamespaces CreateChildren(const std::vector<state::Namespace::Path>& paths) {
		auto children = ChildNamespaces();
		for (const auto& path : paths)
			children.emplace(path[path.size() - 1], path);

		return children;
	}

	void AssertChildren(const ChildNamespaces& expectedChildren, const ChildNamespaces& actualChildren) {
		// Assert:
		ASSERT_EQ(expectedChildren.size(), actualChildren.size());

		auto i = 0u;
		for (const auto& pair : expectedChildren) {
			ASSERT_EQ(1u, actualChildren.count(pair.first)) << "id of child " << i;
			EXPECT_EQ(pair.second, actualChildren.at(pair.first)) << "path of child " << i;
			++i;
		}
	}
}}
