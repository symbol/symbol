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

#include "NamespaceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	Key CreateRandomOwner() {
		return test::GenerateRandomData<Key_Size>();
	}

	std::string GenerateValidName(size_t size) {
		static constexpr auto Valid_Alphabet = "abcdefghijklmnopqrstuvwxyz0123456789";
		auto alphabetLength = strlen(Valid_Alphabet);

		std::string name(size, '\0');
		std::generate(name.begin(), name.end(), [alphabetLength]() {
			return Valid_Alphabet[test::Random() % alphabetLength];
		});
		return name;
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

	state::NamespaceLifetime CreateLifetime(Height::ValueType start, Height::ValueType end, BlockDuration::ValueType gracePeriodDuration) {
		return state::NamespaceLifetime(Height(start), Height(end), BlockDuration(gracePeriodDuration));
	}

	void AddAll(state::RootNamespace& root, const ChildNamespaces& children, std::initializer_list<NamespaceId::ValueType> orderedIds) {
		for (auto childId : orderedIds)
			root.add(state::Namespace(children.at(NamespaceId(childId)).Path));
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

			const auto& actualData = actualChildren.at(pair.first);
			EXPECT_EQ(pair.second.Path, actualData.Path) << "path of child " << i;
			AssertEqualAlias(pair.second.Alias, actualData.Alias);
			++i;
		}
	}

	void AssertEqualAlias(
			const state::NamespaceAlias& expectedAlias,
			const state::NamespaceAlias& actualAlias,
			const std::string& message) {
		ASSERT_EQ(expectedAlias.type(), actualAlias.type()) << message;

		switch (expectedAlias.type()) {
		case state::AliasType::Mosaic:
			EXPECT_EQ(expectedAlias.mosaicId(), actualAlias.mosaicId()) << message;
			break;

		case state::AliasType::Address:
			EXPECT_EQ(expectedAlias.address(), actualAlias.address()) << message;
			break;

		default:
			break;
		}
	}
}}
