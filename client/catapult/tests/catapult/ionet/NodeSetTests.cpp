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

#include "catapult/ionet/NodeSet.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS NodeSetTests

	namespace {
		NodeMetadata CreateMetadata(model::NetworkIdentifier networkIdentifier) {
			return NodeMetadata(model::UniqueNetworkFingerprint(networkIdentifier));
		}

		std::unordered_map<std::string, Node> GenerateEqualityInstanceMap() {
			auto key1 = test::GenerateRandomByteArray<Key>();
			auto key2 = test::GenerateRandomByteArray<Key>();
			return {
				{ "default", { { key1, "11.22.33.44" }, { "bob.com", 1234 }, CreateMetadata(model::NetworkIdentifier::Private) } },
				{ "copy", { { key1, "11.22.33.44" }, { "bob.com", 1234 }, CreateMetadata(model::NetworkIdentifier::Private) } },
				{ "diff-key", { { key2, "11.22.33.44" }, { "bob.com", 1234 }, CreateMetadata(model::NetworkIdentifier::Private) } },
				{ "diff-host", { { key1, "99.88.77.66" }, { "bob.com", 1234 }, CreateMetadata(model::NetworkIdentifier::Private) } },
				{ "diff-key-host", { { key2, "99.88.77.66" }, { "bob.com", 1234 }, CreateMetadata(model::NetworkIdentifier::Private) } },
				{ "diff-endpoint", { { key1, "11.22.33.44" }, { "alice.com", 1234 }, CreateMetadata(model::NetworkIdentifier::Private) } },
				{ "diff-metadata", { { key1, "11.22.33.44" }, { "bob.com", 1234 }, CreateMetadata(model::NetworkIdentifier::Public) } }
			};
		}
	}

	TEST(TEST_CLASS, NodeEquality_OperatorEqualReturnsTrueForEqualObjects) {
		std::unordered_set<std::string> equalityTags{ "default", "copy", "diff-endpoint", "diff-metadata" };
		test::AssertEqualReturnsTrueForEqualObjects<Node>("default", GenerateEqualityInstanceMap(), equalityTags, [](
				const auto& lhs,
				const auto& rhs) {
			return NodeEquality()(lhs, rhs);
		});
	}

	TEST(TEST_CLASS, NodeHasher_OperatorEqualReturnsTrueForEqualObjects) {
		std::unordered_set<std::string> equalityTags{ "default", "copy", "diff-endpoint", "diff-metadata" };
		test::AssertEqualReturnsTrueForEqualObjects<Node>("default", GenerateEqualityInstanceMap(), equalityTags, [](
				const auto& lhs,
				const auto& rhs) {
			auto hasher = NodeHasher();
			return hasher(lhs) == hasher(rhs);
		});
	}
}}
