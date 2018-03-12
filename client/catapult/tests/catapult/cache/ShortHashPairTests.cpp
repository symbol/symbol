#include "catapult/cache/ShortHashPair.h"
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
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
