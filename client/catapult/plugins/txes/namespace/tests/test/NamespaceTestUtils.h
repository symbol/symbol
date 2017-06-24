#pragma once
#include "src/state/Namespace.h"
#include "src/state/NamespaceLifetime.h"
#include "src/state/RootNamespace.h"
#include "catapult/state/AccountState.h"
#include "tests/test/nodeps/Random.h"
#include <vector>

namespace catapult { namespace test {

	using ChildNamespaces = state::RootNamespace::Children;

	/// The default root namespace id used in tests.
	constexpr NamespaceId::ValueType Default_Root_Id(246);

	/// Creates a random namespace owner.
	Key CreateRandomOwner();

	/// Creates a namespace path from \a ids.
	state::Namespace::Path CreatePath(const std::vector<NamespaceId::ValueType>& ids);

	/// Creates a namespace lifetime around \a start and \a end.
	state::NamespaceLifetime CreateLifetime(Height::ValueType start, Height::ValueType end);

	/// Adds all \a children to \a root in the specified order (\a orderedIds).
	void AddAll(state::RootNamespace& root, const ChildNamespaces& children, std::initializer_list<NamespaceId::ValueType> orderedIds);

	/// Creates children from the given \a paths.
	ChildNamespaces CreateChildren(const std::vector<state::Namespace::Path>& paths);

	/// Asserts that the expected children (\a expectedChildren) are equal to the actual children (\a actualChildren).
	void AssertChildren(const ChildNamespaces& expectedChildren, const ChildNamespaces& actualChildren);
}}
