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
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MultisigPermittedOperationValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MultisigPermittedOperation,)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, const cache::CatapultCache& cache, const Key& sender) {
			// Arrange:
			model::TransactionNotification notification(sender, Hash256(), model::EntityType(), Timestamp());
			auto pValidator = CreateMultisigPermittedOperationValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		auto CreateCacheWithSingleLevelMultisig(const Key& embeddedSigner, const std::vector<Key>& cosignatories) {
			auto cache = test::MultisigCacheFactory::Create();
			auto cacheDelta = cache.createDelta();

			test::MakeMultisig(cacheDelta, embeddedSigner, cosignatories, 3, 4); // make a 3-4-X multisig

			cache.commit(Height());
			return cache;
		}
	}

	TEST(TEST_CLASS, NonMultisigAccountIsAllowedToMakeAnyOperation) {
		// Arrange:
		auto multisigAccountKey = test::GenerateRandomByteArray<Key>();
		auto cosignatoryAccountKey = test::GenerateRandomByteArray<Key>();
		auto cache = CreateCacheWithSingleLevelMultisig(multisigAccountKey, { cosignatoryAccountKey });

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, test::GenerateRandomByteArray<Key>());
	}

	TEST(TEST_CLASS, CosignatoryAccountIsAllowedToMakeAnyOperation) {
		// Arrange:
		auto multisigAccountKey = test::GenerateRandomByteArray<Key>();
		auto cosignatoryAccountKey = test::GenerateRandomByteArray<Key>();
		auto cache = CreateCacheWithSingleLevelMultisig(multisigAccountKey, { cosignatoryAccountKey });

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, cosignatoryAccountKey);
	}

	TEST(TEST_CLASS, MultisigAccountIsNotAllowedToMakeAnyOperation) {
		// Arrange:
		auto multisigAccountKey = test::GenerateRandomByteArray<Key>();
		auto cosignatoryAccountKey = test::GenerateRandomByteArray<Key>();
		auto cache = CreateCacheWithSingleLevelMultisig(multisigAccountKey, { cosignatoryAccountKey });

		// Assert:
		AssertValidationResult(Failure_Multisig_Operation_Prohibited_By_Account, cache, multisigAccountKey);
	}
}}
