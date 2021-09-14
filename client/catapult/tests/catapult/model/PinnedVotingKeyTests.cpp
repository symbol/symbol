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

#include "catapult/model/PinnedVotingKey.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS PinnedVotingKeyTests

	// region equality

	namespace {
		std::unordered_map<std::string, PinnedVotingKey> GenerateEqualityInstanceMap() {
			auto key1 = test::GenerateRandomByteArray<VotingKey>();
			auto key2 = test::GenerateRandomByteArray<VotingKey>();
			return {
				{ "default", { key1, FinalizationEpoch(0), FinalizationEpoch(50) } },
				{ "copy", { key1, FinalizationEpoch(0), FinalizationEpoch(50) } },
				{ "diff-key", { key2, FinalizationEpoch(0), FinalizationEpoch(50) } },
				{ "diff-start", { key1, FinalizationEpoch(21), FinalizationEpoch(50) } },
				{ "diff-end", { key1, FinalizationEpoch(0), FinalizationEpoch(43) } },
				{ "diff-start-end", { key1, FinalizationEpoch(21), FinalizationEpoch(42) } }
			};
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { "default", "copy" };
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion

	// region to string

	TEST(TEST_CLASS, CanOutputPinnedVotingKey) {
		// Arrange:
		constexpr auto Pub_Key_String = "7F280CFB82753B5CB0EAF9253087FDBBBB06E298F8045D2874C3D83E33AE27C4";
		PinnedVotingKey pinnedVotingKey{
			utils::ParseByteArray<VotingKey>(Pub_Key_String),
			FinalizationEpoch(100),
			FinalizationEpoch(321)
		};

		// Act:
		auto str = test::ToString(pinnedVotingKey);

		// Assert:
		EXPECT_EQ(std::string(Pub_Key_String) + " @ [100, 321]", str);
	}

	// endregion

	// region FindVotingPublicKeyForEpoch

	namespace {
		VotingKey RunFindVotingPublicKeyForEpoch(FinalizationEpoch::ValueType epoch) {
			return FindVotingPublicKeyForEpoch({
				{ { { 1 } }, FinalizationEpoch(125), FinalizationEpoch(130) },
				{ { { 2 } }, FinalizationEpoch(150), FinalizationEpoch(170) },
				{ { { 3 } }, FinalizationEpoch(171), FinalizationEpoch(180) }
			}, FinalizationEpoch(epoch));
		}
	}

	TEST(TEST_CLASS, FindVotingPublicKeyForEpoch_ReturnsZeroKeyWhenNoPinnedVotingKeys) {
		EXPECT_EQ(VotingKey(), FindVotingPublicKeyForEpoch({}, FinalizationEpoch(123)));
	}

	TEST(TEST_CLASS, FindVotingPublicKeyForEpoch_ReturnsVotingKeyWhenRegisteredForEpoch) {
		EXPECT_EQ(VotingKey{ { 1 } }, RunFindVotingPublicKeyForEpoch(125));
		EXPECT_EQ(VotingKey{ { 2 } }, RunFindVotingPublicKeyForEpoch(150));
		EXPECT_EQ(VotingKey{ { 2 } }, RunFindVotingPublicKeyForEpoch(160));
		EXPECT_EQ(VotingKey{ { 2 } }, RunFindVotingPublicKeyForEpoch(170));
		EXPECT_EQ(VotingKey{ { 3 } }, RunFindVotingPublicKeyForEpoch(180));
	}

	TEST(TEST_CLASS, FindVotingPublicKeyForEpoch_ReturnsZeroKeyWhenNotRegisteredForEpoch) {
		for (auto epoch : std::initializer_list<FinalizationEpoch::ValueType>{ 70, 124, 131, 140, 149, 181, 300 })
			EXPECT_EQ(VotingKey(), RunFindVotingPublicKeyForEpoch(epoch)) << "epoch " << epoch;
	}

	// endregion
}}
