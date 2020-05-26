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
#include "catapult/utils/Functional.h"
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MultisigMaxCosignatoriesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MultisigMaxCosignatories, 0)

	namespace {
		void AssertValidationResult(
				ValidationResult expectedResult,
				uint8_t numInitialCosignatories,
				uint8_t numAdditions,
				uint8_t numDeletions,
				uint8_t maxCosignatoriesPerAccount) {
			// Arrange:
			auto multisig = test::GenerateRandomByteArray<Address>();

			// - setup cache
			auto cache = test::MultisigCacheFactory::Create();
			if (numInitialCosignatories > 0) {
				auto cacheDelta = cache.createDelta();
				auto entry = state::MultisigEntry(multisig);

				// - add cosignatories
				for (auto i = 0; i < numInitialCosignatories; ++i)
					entry.cosignatoryAddresses().insert(test::GenerateRandomByteArray<Address>());

				cacheDelta.sub<cache::MultisigCache>().insert(entry);
				cache.commit(Height());
			}

			auto addressAdditions = test::GenerateRandomDataVector<UnresolvedAddress>(numAdditions);
			auto addressDeletions = test::GenerateRandomDataVector<UnresolvedAddress>(numDeletions);
			auto notification = test::CreateMultisigCosignatoriesNotification(multisig, addressAdditions, addressDeletions);
			auto pValidator = CreateMultisigMaxCosignatoriesValidator(maxCosignatoriesPerAccount);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "initial " << static_cast<uint32_t>(numInitialCosignatories)
					<< ", numAdditions " << numAdditions
					<< ", numDeletions " << numDeletions
					<< ", max " << static_cast<uint32_t>(maxCosignatoriesPerAccount);
		}
	}

	TEST(TEST_CLASS, CanAddCosignatoriesWhenMaxCosignatoriesIsNotExceeded) {
		AssertValidationResult(ValidationResult::Success, 0, 5, 0, 10);
	}

	TEST(TEST_CLASS, CanAddAndDeleteCosignatoriesWhenResultingNumberOfCosignatoryDoesNotExceedMaxCosignatories) {
		AssertValidationResult(ValidationResult::Success, 9, 3, 2, 10);
	}

	TEST(TEST_CLASS, CannotAddCosignatoriesWhenMaxCosignatoriesIsExceeded) {
		AssertValidationResult(Failure_Multisig_Max_Cosignatories, 10, 1, 0, 10);
		AssertValidationResult(Failure_Multisig_Max_Cosignatories, 8, 3, 0, 10);
	}

	TEST(TEST_CLASS, CannotAddAndDeleteCosignatoriesWhenResultingNumberOfCosignatoryDoesExceedMaxCosignatories) {
		AssertValidationResult(Failure_Multisig_Max_Cosignatories, 9, 3, 1, 10);
		AssertValidationResult(Failure_Multisig_Max_Cosignatories, 9, 4, 2, 10);
	}
}}
