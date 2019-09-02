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
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MultisigMaxCosignatoriesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MultisigMaxCosignatories, 0)

	namespace {
		constexpr auto Add = model::CosignatoryModificationAction::Add;
		constexpr auto Del = model::CosignatoryModificationAction::Del;

		void AssertValidationResult(
				ValidationResult expectedResult,
				uint8_t numInitialCosignatories,
				const std::vector<model::CosignatoryModificationAction>& modificationActions,
				uint8_t maxCosignatoriesPerAccount) {
			// Arrange:
			auto signer = test::GenerateRandomByteArray<Key>();

			// - setup cache
			auto cache = test::MultisigCacheFactory::Create();
			if (numInitialCosignatories > 0) {
				auto cacheDelta = cache.createDelta();
				auto entry = state::MultisigEntry(signer);

				// - add cosignatories
				for (auto i = 0; i < numInitialCosignatories; ++i)
					entry.cosignatoryPublicKeys().insert(test::GenerateRandomByteArray<Key>());

				cacheDelta.sub<cache::MultisigCache>().insert(entry);
				cache.commit(Height());
			}

			std::vector<model::CosignatoryModification> modifications;
			for (auto modificationAction : modificationActions)
				modifications.push_back({ modificationAction, test::GenerateRandomByteArray<Key>() });

			model::MultisigCosignatoriesNotification notification(
					signer,
					static_cast<uint8_t>(modifications.size()), modifications.data());
			auto pValidator = CreateMultisigMaxCosignatoriesValidator(maxCosignatoriesPerAccount);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			auto change = utils::Sum(modificationActions, [](const auto& modificationAction) {
				return Add == modificationAction ? 1 : -1;
			});
			EXPECT_EQ(expectedResult, result)
					<< "initial " << static_cast<uint32_t>(numInitialCosignatories)
					<< ", change " << change
					<< ", max " << static_cast<uint32_t>(maxCosignatoriesPerAccount);
		}
	}

	TEST(TEST_CLASS, CanAddCosignatoriesWhenMaxCosignatoriesIsNotExceeded) {
		AssertValidationResult(ValidationResult::Success, 0, { Add, Add, Add, Add, Add }, 10);
	}

	TEST(TEST_CLASS, CanAddAndDeleteCosignatoriesWhenResultingNumberOfCosignatoryDoesNotExceedMaxCosignatories) {
		AssertValidationResult(ValidationResult::Success, 9, { Add, Add, Add, Del, Del }, 10);
		AssertValidationResult(ValidationResult::Success, 9, { Del, Del, Add, Add, Add }, 10);
		AssertValidationResult(ValidationResult::Success, 9, { Del, Add, Add, Del, Add }, 10);
	}

	TEST(TEST_CLASS, CannotAddCosignatoriesWhenMaxCosignatoriesIsExceeded) {
		AssertValidationResult(Failure_Multisig_Max_Cosignatories, 10, { Add }, 10);
		AssertValidationResult(Failure_Multisig_Max_Cosignatories, 8, { Add, Add, Add }, 10);
	}

	TEST(TEST_CLASS, CannotAddAndDeleteCosignatoriesWhenResultingNumberOfCosignatoryDoesExceedMaxCosignatories) {
		AssertValidationResult(Failure_Multisig_Max_Cosignatories, 9, { Add, Add, Add, Del }, 10);
		AssertValidationResult(Failure_Multisig_Max_Cosignatories, 9, { Del, Add, Add, Add }, 10);
		AssertValidationResult(Failure_Multisig_Max_Cosignatories, 9, { Del, Add, Del, Add, Add, Add }, 10);
	}
}}
