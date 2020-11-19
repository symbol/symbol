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

#include "catapult/cache_tx/ShortHashPair.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ShortHashPairTests

	// region equality operators

	namespace {
		std::unordered_set<std::string> GetEqualTags() {
			return { "default", "copy" };
		}

		std::unordered_map<std::string, ShortHashPair> GenerateEqualityInstanceMap() {
			auto pair = ShortHashPair{ test::GenerateRandomValue<utils::ShortHash>(), test::GenerateRandomValue<utils::ShortHash>() };
			return {
				{ "default", pair },
				{ "copy", { pair.TransactionShortHash, pair.CosignaturesShortHash } },

				{ "diff tx hash", { test::GenerateRandomValue<utils::ShortHash>(), pair.CosignaturesShortHash } },
				{ "diff cosigs hash", { pair.TransactionShortHash, test::GenerateRandomValue<utils::ShortHash>() } },
				{ "diff all hashes", { test::GenerateRandomValue<utils::ShortHash>(), test::GenerateRandomValue<utils::ShortHash>() } }
			};
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
