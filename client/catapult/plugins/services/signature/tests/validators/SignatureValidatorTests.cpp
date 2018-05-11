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
#include "catapult/crypto/Signer.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(Signature,)

#define TEST_CLASS SignatureValidatorTests

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, const model::SignatureNotification& notification) {
			// Arrange:
			auto pValidator = CreateSignatureValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingValidSignature) {
		// Arrange:
		auto signer = test::GenerateKeyPair();
		auto data = test::GenerateRandomVector(55);
		Signature signature;
		crypto::Sign(signer, data, signature);

		model::SignatureNotification notification(signer.publicKey(), signature, data);

		// Assert:
		AssertValidationResult(ValidationResult::Success, notification);
	}

	TEST(TEST_CLASS, FailureWhenSignatureIsAltered) {
		// Arrange:
		auto signer = test::GenerateKeyPair();
		auto data = test::GenerateRandomVector(55);
		Signature signature;
		crypto::Sign(signer, data, signature);

		model::SignatureNotification notification(signer.publicKey(), signature, data);
		signature[0] ^= 0xFF;

		// Assert:
		AssertValidationResult(Failure_Signature_Not_Verifiable, notification);
	}

	TEST(TEST_CLASS, FailureWhenDataIsAltered) {
		// Arrange:
		auto signer = test::GenerateKeyPair();
		auto data = test::GenerateRandomVector(55);
		Signature signature;
		crypto::Sign(signer, data, signature);

		model::SignatureNotification notification(signer.publicKey(), signature, data);
		data[10] ^= 0xFF;

		// Assert:
		AssertValidationResult(Failure_Signature_Not_Verifiable, notification);
	}
}}
