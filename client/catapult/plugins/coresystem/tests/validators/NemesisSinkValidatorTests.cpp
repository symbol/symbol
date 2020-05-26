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
#include "catapult/crypto/KeyPair.h"
#include "catapult/validators/ValidatorContext.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(NemesisSink,)

#define TEST_CLASS NemesisSinkValidatorTests

	namespace {
		constexpr auto Success_Result = ValidationResult::Success;
		constexpr auto Failure_Result = Failure_Core_Nemesis_Account_Signed_After_Nemesis_Block;

		crypto::KeyPair GetNemesisAccount() {
			// note that the nemesis account is fake in order to ensure that it is being retrieved from the context
			return crypto::KeyPair::FromString("0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF");
		}

		void AssertValidationResult(ValidationResult expectedResult, const Key& signer, Height::ValueType height) {
			// Arrange:
			auto cache = test::CreateEmptyCatapultCache();
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			model::NetworkInfo networkInfo;
			networkInfo.NemesisSignerPublicKey = GetNemesisAccount().publicKey();

			auto pValidator = CreateNemesisSinkValidator();
			auto context = test::CreateValidatorContext(Height(height), networkInfo, readOnlyCache);

			auto signature = test::GenerateRandomByteArray<Signature>();
			model::SignatureNotification notification(signer, signature, {});

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingEntityNotSignedByTheNemesisAccount) {
		// Arrange:
		auto signer = test::GenerateRandomByteArray<Key>();

		// Sanity:
		EXPECT_NE(GetNemesisAccount().publicKey(), signer);

		// Assert: signer is allowed at all heights
		AssertValidationResult(Success_Result, signer, 1);
		AssertValidationResult(Success_Result, signer, 10);
		AssertValidationResult(Success_Result, signer, 100);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingEntityAtHeightOneSignedByNemesisAccount) {
		// Arrange:
		auto signer = GetNemesisAccount().publicKey();

		// Sanity:
		EXPECT_EQ(GetNemesisAccount().publicKey(), signer);

		// Assert: allowed at height one
		AssertValidationResult(Success_Result, signer, 1);
	}

	TEST(TEST_CLASS, FailureWhenValidatingEntityNotAtHeightOneSignedByNemesisAccount) {
		// Arrange:
		auto signer = GetNemesisAccount().publicKey();

		// Sanity:
		EXPECT_EQ(GetNemesisAccount().publicKey(), signer);

		// Assert: not allowed at heights greater than one
		AssertValidationResult(Failure_Result, signer, 2);
		AssertValidationResult(Failure_Result, signer, 10);
		AssertValidationResult(Failure_Result, signer, 100);
	}
}}
