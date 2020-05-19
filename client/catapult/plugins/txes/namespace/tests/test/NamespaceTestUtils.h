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

#pragma once
#include "src/state/Namespace.h"
#include "src/state/NamespaceLifetime.h"
#include "src/state/RootNamespace.h"
#include "catapult/state/AccountState.h"
#include "tests/test/nodeps/Random.h"
#include <vector>

namespace catapult { namespace state { class RootNamespaceHistory; } }

namespace catapult { namespace test {

	using ChildNamespaces = state::RootNamespace::Children;

	/// Default root namespace id used in tests.
	constexpr NamespaceId::ValueType Default_Root_Id(246);

	/// Creates a random namespace owner.
	Address CreateRandomOwner();

	/// Generates valid namespace name of length \a size.
	std::string GenerateValidName(size_t size);

	/// Creates a namespace path from \a ids.
	state::Namespace::Path CreatePath(const std::vector<NamespaceId::ValueType>& ids);

	/// Creates a namespace lifetime around \a start and \a end.
	state::NamespaceLifetime CreateLifetime(Height::ValueType start, Height::ValueType end);

	/// Creates a namespace lifetime around \a start, \a end and \a gracePeriodDuration.
	state::NamespaceLifetime CreateLifetime(Height::ValueType start, Height::ValueType end, BlockDuration::ValueType gracePeriodDuration);

	/// Adds all \a children to \a root in the specified order (\a orderedIds).
	void AddAll(state::RootNamespace& root, const ChildNamespaces& children, std::initializer_list<NamespaceId::ValueType> orderedIds);

	/// Creates children from the given \a paths.
	ChildNamespaces CreateChildren(const std::vector<state::Namespace::Path>& paths);

	/// Asserts that the expected children (\a expectedChildren) are equal to the actual children (\a actualChildren).
	void AssertChildren(const ChildNamespaces& expectedChildren, const ChildNamespaces& actualChildren);

	/// Asserts that \a expectedAlias and \a actualAlias are equal with optional \a message.
	void AssertEqualAlias(
			const state::NamespaceAlias& expectedAlias,
			const state::NamespaceAlias& actualAlias,
			const std::string& message = "");

	/// Asserts that root namespace \a actual is shallow equal to \a expected.
	/// \note This only compares the most recent root namespace in the history.
	void AssertNonHistoricalEqual(const state::RootNamespaceHistory& expected, const state::RootNamespaceHistory& actual);

	/// Asserts that root namespace \a actual is deep equal to \a expected.
	/// \note This compares all root namespaces in the history.
	void AssertHistoricalEqual(const state::RootNamespaceHistory& expected, const state::RootNamespaceHistory& actual);
}}
