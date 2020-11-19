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

#include "catapult/model/NodeIdentity.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"
#include <unordered_map>

namespace catapult { namespace model {

#define TEST_CLASS NodeIdentityTests

	// region NodeIdentity

	namespace {
		void AssertOutputOperator(const NodeIdentity& identity, const std::string& expected) {
			// Act:
			auto str = test::ToString(identity);

			// Assert:
			EXPECT_EQ(expected, str);
		}
	}

	TEST(TEST_CLASS, CanOutputNodeWithHost) {
		auto identityKey = utils::ParseByteArray<Key>("1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751");
		AssertOutputOperator(
				{ identityKey, "11.22.33.44" },
				"1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751 @ 11.22.33.44");
	}

	TEST(TEST_CLASS, CanOutputNodeWithoutHost) {
		auto identityKey = utils::ParseByteArray<Key>("1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751");
		AssertOutputOperator({ identityKey, "" }, "1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751");
	}

	// endregion

	// region NodeIdentityEqualityStrategy

	TEST(TEST_CLASS, CanParseValidNodeIdentityEqualityStrategy) {
		test::AssertParse("public-key", NodeIdentityEqualityStrategy::Key, TryParseValue);
		test::AssertParse("host", NodeIdentityEqualityStrategy::Host, TryParseValue);
	}

	// endregion

	// region NodeIdentityEquality

	namespace {
		std::unordered_map<std::string, NodeIdentity> GenerateEqualityInstanceMap() {
			auto key1 = test::GenerateRandomByteArray<Key>();
			auto key2 = test::GenerateRandomByteArray<Key>();
			return {
				{ "default", { key1, "alice.com" } },
				{ "copy", { key1, "alice.com" } },
				{ "diff-key", { key2, "alice.com" } },
				{ "diff-host", { key1, "bob.com" } },
				{ "diff-key-host", { key2, "bob.com" } }
			};
		}

		void RunNodeIdentityEqualityTest(NodeIdentityEqualityStrategy strategy, const std::unordered_set<std::string>& equalityTags) {
			test::AssertEqualReturnsTrueForEqualObjects<NodeIdentity>("default", GenerateEqualityInstanceMap(), equalityTags, [strategy](
					const auto& lhs,
					const auto& rhs) {
				return NodeIdentityEquality(strategy)(lhs, rhs);
			});
		}
	}

	TEST(TEST_CLASS, NodeIdentityEquality_OperatorEqualReturnsTrueForEqualObjects_Key) {
		RunNodeIdentityEqualityTest(NodeIdentityEqualityStrategy::Key, { "default", "copy", "diff-host" });
	}

	TEST(TEST_CLASS, NodeIdentityEquality_OperatorEqualReturnsTrueForEqualObjects_Host) {
		RunNodeIdentityEqualityTest(NodeIdentityEqualityStrategy::Host, { "default", "copy", "diff-key" });
	}

	TEST(TEST_CLASS, NodeIdentityEquality_OperatorEqualReturnsTrueForEqualObjects_KeyAndHost) {
		RunNodeIdentityEqualityTest(NodeIdentityEqualityStrategy::Key_And_Host, { "default", "copy" });
	}

	// endregion

	// region NodeIdentityHasher

	namespace {
		void RunNodeIdentityHasherTest(NodeIdentityEqualityStrategy strategy, const std::unordered_set<std::string>& equalityTags) {
			test::AssertEqualReturnsTrueForEqualObjects<NodeIdentity>("default", GenerateEqualityInstanceMap(), equalityTags, [strategy](
					const auto& lhs,
					const auto& rhs) {
				auto hasher = NodeIdentityHasher(strategy);
				return hasher(lhs) == hasher(rhs);
			});
		}
	}

	TEST(TEST_CLASS, NodeIdentityHasher_OperatorEqualReturnsTrueForEqualObjects_Key) {
		RunNodeIdentityHasherTest(NodeIdentityEqualityStrategy::Key, { "default", "copy", "diff-host" });
	}

	TEST(TEST_CLASS, NodeIdentityHasher_OperatorEqualReturnsTrueForEqualObjects_Host) {
		RunNodeIdentityHasherTest(NodeIdentityEqualityStrategy::Host, { "default", "copy", "diff-key" });
	}

	TEST(TEST_CLASS, NodeIdentityHasher_OperatorEqualReturnsTrueForEqualObjects_KeyAndHost) {
		RunNodeIdentityHasherTest(NodeIdentityEqualityStrategy::Key_And_Host, { "default", "copy" });
	}

	// endregion

	// region CreateNodeIdentitySet

	namespace {
		template<typename TSetTraits>
		void RunNodeIdentitySetTest(NodeIdentityEqualityStrategy strategy, const std::unordered_set<size_t>& expectedIdentityIndexes) {
			// Arrange:
			auto key1 = test::GenerateRandomByteArray<Key>();
			auto key2 = test::GenerateRandomByteArray<Key>();
			auto seedIdentities = std::vector<NodeIdentity>{
				{ key1, "alice.com" },
				{ key2, "alice.com" },
				{ key1, "bob.com" },
				{ key2, "bob.com" }
			};

			// Act: create a set and seed it
			auto identities = TSetTraits::CreateSet(strategy);
			for (const auto& identity : seedIdentities)
				TSetTraits::Insert(identities, identity);

			// Assert: check set contents
			EXPECT_EQ(expectedIdentityIndexes.size(), identities.size());

			auto i = 0u;
			auto equality = NodeIdentityEquality(NodeIdentityEqualityStrategy::Key_And_Host);
			for (const auto& identity : seedIdentities) {
				// - find is using equality from set, so all seedIdentities should be equal to *something* in set
				//   but not necessarily deep equal to that *something*
				auto tryFindResult = TSetTraits::TryFind(identities, identity);
				ASSERT_TRUE(tryFindResult.second) << "i = " << i;

				// - deep equal is only expected if index is in `expectedIdentityIndexes`
				auto isExpectedInSet = expectedIdentityIndexes.cend() != expectedIdentityIndexes.find(i);
				EXPECT_EQ(isExpectedInSet, equality(identity, tryFindResult.first)) << "i = " << i;
				++i;
			}
		}

		struct SetTraits {
			static NodeIdentitySet CreateSet(NodeIdentityEqualityStrategy strategy) {
				return CreateNodeIdentitySet(strategy);
			}

			static void Insert(NodeIdentitySet& set, const model::NodeIdentity& identity) {
				set.insert(identity);
			}

			static std::pair<NodeIdentity, bool> TryFind(const NodeIdentitySet& set, const model::NodeIdentity& identity) {
				auto iter = set.find(identity);
				return set.cend() == iter
						? std::make_pair(model::NodeIdentity(), false)
						: std::make_pair(*iter, true);
			}
		};
	}

	TEST(TEST_CLASS, CanCreateNodeIdentitySetWithStrategy_Key) {
		RunNodeIdentitySetTest<SetTraits>(NodeIdentityEqualityStrategy::Key, { 0, 1 });
	}

	TEST(TEST_CLASS, CanCreateNodeIdentitySetWithStrategy_Host) {
		RunNodeIdentitySetTest<SetTraits>(NodeIdentityEqualityStrategy::Host, { 0, 2 });
	}

	TEST(TEST_CLASS, CanCreateNodeIdentitySetWithStrategy_KeyAndHost) {
		RunNodeIdentitySetTest<SetTraits>(NodeIdentityEqualityStrategy::Key_And_Host, { 0, 1, 2, 3 });
	}

	// endregion

	// region CreateNodeIdentityMap

	namespace {
		struct MapTraits {
			static NodeIdentityMap<size_t> CreateSet(NodeIdentityEqualityStrategy strategy) {
				return CreateNodeIdentityMap<size_t>(strategy);
			}

			static void Insert(NodeIdentityMap<size_t>& map, const model::NodeIdentity& identity) {
				map.emplace(identity, map.size());
			}

			static std::pair<NodeIdentity, bool> TryFind(const NodeIdentityMap<size_t>& map, const model::NodeIdentity& identity) {
				auto iter = map.find(identity);
				return map.cend() == iter
						? std::make_pair(model::NodeIdentity(), false)
						: std::make_pair(iter->first, true);
			}
		};
	}

	TEST(TEST_CLASS, CanCreateNodeIdentityMapWithStrategy_Key) {
		RunNodeIdentitySetTest<MapTraits>(NodeIdentityEqualityStrategy::Key, { 0, 1 });
	}

	TEST(TEST_CLASS, CanCreateNodeIdentityMapWithStrategy_Host) {
		RunNodeIdentitySetTest<MapTraits>(NodeIdentityEqualityStrategy::Host, { 0, 2 });
	}

	TEST(TEST_CLASS, CanCreateNodeIdentityMapWithStrategy_KeyAndHost) {
		RunNodeIdentitySetTest<MapTraits>(NodeIdentityEqualityStrategy::Key_And_Host, { 0, 1, 2, 3 });
	}

	// endregion
}}
