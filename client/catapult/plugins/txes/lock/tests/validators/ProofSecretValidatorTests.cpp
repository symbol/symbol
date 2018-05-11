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

#include "src/validators/Validators.h"
#include "catapult/crypto/Hashes.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS ProofSecretValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(ProofSecret, 0, 0)

	namespace {
		struct NotificationBuilder {
		public:
			NotificationBuilder(model::LockHashAlgorithm algorithm = model::LockHashAlgorithm::Op_Sha3)
					: m_algorithm(algorithm) {
				setProofSize(50);
				test::FillWithRandomData(m_secret);
			}

		public:
			auto notification() {
				return model::ProofSecretNotification(m_algorithm, m_secret, m_proof);
			}

			void setProofSize(size_t proofSize) {
				m_proof.resize(proofSize);
				test::FillWithRandomData(m_proof);
			}

			void setValidHash() {
				crypto::Sha3_512(m_proof, m_secret);
			}

		private:
			model::LockHashAlgorithm m_algorithm;
			std::vector<uint8_t> m_proof;
			Hash512 m_secret;
		};

		auto CreateDefaultProofSecretValidator() {
			return CreateProofSecretValidator(10, 100);
		}

		void AssertFailureIfHashAlgorithmIsNotSupported(model::LockHashAlgorithm lockHashAlgorithm) {
			// Arrange:
			NotificationBuilder notificationBuilder(lockHashAlgorithm);
			auto pValidator = CreateDefaultProofSecretValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notificationBuilder.notification());

			// Assert:
			EXPECT_EQ(Failure_Lock_Hash_Not_Implemented, result) << "hash algorithm: " << utils::to_underlying_type(lockHashAlgorithm);
		}
	}

	TEST(TEST_CLASS, FailureIfHashAlgorithmIsNotSupported) {
		using model::LockHashAlgorithm;

		// Assert:
		for (auto unsupportedAlgorithm : { LockHashAlgorithm::Op_Hash_160, LockHashAlgorithm::Op_Hash_256, LockHashAlgorithm::Op_Keccak })
			AssertFailureIfHashAlgorithmIsNotSupported(unsupportedAlgorithm);
	}

	TEST(TEST_CLASS, FailureIfSecretDoesNotMatchProof) {
		NotificationBuilder notificationBuilder;
		auto pValidator = CreateDefaultProofSecretValidator();

		// Act:
		auto result = test::ValidateNotification(*pValidator, notificationBuilder.notification());

		// Assert:
		EXPECT_EQ(Failure_Lock_Secret_Mismatch, result);
	}

	TEST(TEST_CLASS, SuccessIfSecretMatchesProof) {
		NotificationBuilder notificationBuilder;
		notificationBuilder.setValidHash();
		auto pValidator = CreateDefaultProofSecretValidator();

		// Act:
		auto result = test::ValidateNotification(*pValidator, notificationBuilder.notification());

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
	}

	namespace {
		void AssertProofSize(ValidationResult expectedResult, size_t proofSize) {
			// Arrange:
			NotificationBuilder notificationBuilder;
			notificationBuilder.setProofSize(proofSize);
			notificationBuilder.setValidHash();
			auto pValidator = CreateDefaultProofSecretValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notificationBuilder.notification());

			// Assert:
			EXPECT_EQ(expectedResult, result) << "proof size: " << proofSize;
		}
	}

	TEST(TEST_CLASS, FailureIfProofIsOutOfBounds) {
		// Assert: minimum size is 10, maximum is 100, so all should fail
		for (auto proofSize : { 3u, 9u, 101u, 105u })
			AssertProofSize(Failure_Lock_Proof_Size_Out_Of_Bounds, proofSize);
	}

	TEST(TEST_CLASS, SuccessIfProofIsWithinBounds) {
		// Assert: minimum size is 10, maximum is 100, so all should succeed
		for (auto proofSize : { 10u, 40u, 100u })
			AssertProofSize(ValidationResult::Success, proofSize);
	}
}}
