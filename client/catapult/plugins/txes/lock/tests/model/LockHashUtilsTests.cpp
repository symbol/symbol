#include "src/model/LockHashUtils.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/utils/Casting.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS LockHashUtilsTests

	TEST(TEST_CLASS, CalculateHashReturnsHashForOpSha3) {
		// Arrange:
		auto data = test::GenerateRandomVector(123);
		Hash512 expected;
		crypto::Sha3_512(data, expected);

		// Act:
		auto result = CalculateHash(LockHashAlgorithm::Op_Sha3, data);

		// Assert:
		EXPECT_EQ(expected, result);
	}

	namespace {
		void AssertUnsupportedAlgorithm(LockHashAlgorithm algorithm) {
			// Arrange:
			auto data = test::GenerateRandomVector(123);

			// Act + Assert:
			EXPECT_THROW(CalculateHash(algorithm, data), catapult_invalid_argument)
					<< "algorithm: " << utils::to_underlying_type(algorithm);
		}
	}

	TEST(TEST_CLASS, CalculateHashReturnsZeroHashForUnsupportedAlgorithm) {
		// Act + Assert:
		for (auto algorithm : { LockHashAlgorithm::Op_Hash_160, LockHashAlgorithm::Op_Hash_256, LockHashAlgorithm::Op_Keccak })
			AssertUnsupportedAlgorithm(algorithm);
	}
}}
