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

#include "src/model/LockHashUtils.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/utils/Casting.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS LockHashUtilsTests

	// region CalculateHash

	namespace {
		struct OpSha3_256_Traits {
			using HashType = Hash256;
			static constexpr auto HashAlgorithm = LockHashAlgorithm::Op_Sha3_256;
			static constexpr auto HashFunc = crypto::Sha3_256;
		};

		struct OpHash_160_Traits {
			using HashType = Hash160;
			static constexpr auto HashAlgorithm = LockHashAlgorithm::Op_Hash_160;
			static constexpr auto HashFunc = crypto::Bitcoin160;
		};

		struct OpHash_256_Traits {
			using HashType = Hash256;
			static constexpr auto HashAlgorithm = LockHashAlgorithm::Op_Hash_256;
			static constexpr auto HashFunc = crypto::Sha256Double;
		};

		template<typename TTraits>
		void AssertCalculateHashReturnsProperHash() {
			// Arrange:
			auto dataBuffer = test::GenerateRandomVector(123);
			typename TTraits::HashType expected;
			TTraits::HashFunc(dataBuffer, expected);

			// Act:
			auto result = CalculateHash(TTraits::HashAlgorithm, dataBuffer);

			// Assert: if expected hash is shorter than 256bits, validate that rest is zero initialized
			EXPECT_EQ_MEMORY(expected.data(), result.data(), expected.size());
			if constexpr (!std::is_same_v<Hash256, typename TTraits::HashType>) {
				std::array<uint8_t, Hash256::Size - expected.size()> zeroMemory{};
				EXPECT_EQ_MEMORY(zeroMemory.data(), result.data() + expected.size(), zeroMemory.size());
			}
		}
	}

#define MAKE_CALCULATE_HASH_TEST(OPERATION_NAME) \
	TEST(TEST_CLASS, CalculateHashReturnsHashFor##OPERATION_NAME) { \
		AssertCalculateHashReturnsProperHash<OPERATION_NAME##_Traits>(); \
	}

	MAKE_CALCULATE_HASH_TEST(OpSha3_256)

	MAKE_CALCULATE_HASH_TEST(OpHash_160)

	MAKE_CALCULATE_HASH_TEST(OpHash_256)

	TEST(TEST_CLASS, CalculateHashThrowsForInvalidAlgorithm) {
		// Arrange:
		auto dataBuffer = test::GenerateRandomVector(123);

		// Act + Assert:
		auto algorithm = static_cast<LockHashAlgorithm>(utils::to_underlying_type(LockHashAlgorithm::Op_Hash_256) + 1);
		EXPECT_THROW(CalculateHash(algorithm, dataBuffer), catapult_invalid_argument);
	}

	// endregion

	// region CalculateSecretLockInfoHash

	TEST(TEST_CLASS, CalculateSecretLockInfoHash_DifferentSecretsYieldDifferentHashes) {
		// Arrange:
		auto secret1 = test::GenerateRandomByteArray<Hash256>();
		auto secret2 = test::GenerateRandomByteArray<Hash256>();
		auto recipient = test::GenerateRandomByteArray<Address>();

		// Act:
		auto hash1 = CalculateSecretLockInfoHash(secret1, recipient);
		auto hash2 = CalculateSecretLockInfoHash(secret2, recipient);

		// Assert:
		EXPECT_NE(hash1, hash2);
	}

	TEST(TEST_CLASS, CalculateSecretLockInfoHash_DifferentRecipientsYieldDifferentHashes) {
		// Arrange:
		auto secret = test::GenerateRandomByteArray<Hash256>();
		auto recipient1 = test::GenerateRandomByteArray<Address>();
		auto recipient2 = test::GenerateRandomByteArray<Address>();

		// Act:
		auto hash1 = CalculateSecretLockInfoHash(secret, recipient1);
		auto hash2 = CalculateSecretLockInfoHash(secret, recipient2);

		// Assert:
		EXPECT_NE(hash1, hash2);
	}

	TEST(TEST_CLASS, CalculateSecretLockInfoHash_IsDeterministic) {
		// Arrange:
		auto secret = test::GenerateRandomByteArray<Hash256>();
		auto recipient = test::GenerateRandomByteArray<Address>();

		// Act:
		auto hash1 = CalculateSecretLockInfoHash(secret, recipient);
		auto hash2 = CalculateSecretLockInfoHash(secret, recipient);

		// Assert:
		EXPECT_EQ(hash1, hash2);
	}

	// endregion
}}
