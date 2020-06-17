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

#include "catapult/types.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult {

#define TEST_CLASS PinnedVotingKeyTests

	namespace {
		std::unordered_map<std::string, PinnedVotingKey> GenerateEqualityInstanceMap() {
			auto key1 = test::GenerateRandomByteArray<VotingKey>();
			auto key2 = test::GenerateRandomByteArray<VotingKey>();
			return {
				{ "default", { key1, FinalizationPoint(0), FinalizationPoint(50) } },
				{ "copy", { key1, FinalizationPoint(0), FinalizationPoint(50) } },
				{ "diff-key", { key2, FinalizationPoint(0), FinalizationPoint(50) } },
				{ "diff-start", { key1, FinalizationPoint(21), FinalizationPoint(50) } },
				{ "diff-end", { key1, FinalizationPoint(0), FinalizationPoint(43) } },
				{ "diff-start-end", { key1, FinalizationPoint(21), FinalizationPoint(42) } }
			};
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { "default", "copy", "diff-start", "diff-end", "diff-start-end" };
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}
}
