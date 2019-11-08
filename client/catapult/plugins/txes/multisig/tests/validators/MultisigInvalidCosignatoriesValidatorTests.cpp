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
#include "src/cache/MultisigCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MultisigInvalidCosignatoriesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MultisigInvalidCosignatories,)

	namespace {
		constexpr auto CreateNotification = test::CreateMultisigCosignatoriesNotification;

		void AssertValidationResult(
				ValidationResult expectedResult,
				const cache::CatapultCache& cache,
				Height height,
				const model::MultisigCosignatoriesNotification& notification) {
			// Arrange:
			auto pValidator = CreateMultisigInvalidCosignatoriesValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache, height);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		void AssertMultisigAccountIsUnknown(
				ValidationResult expectedResult,
				const std::vector<Key>& publicKeyAdditions,
				const std::vector<Key>& publicKeyDeletions) {
			// Arrange:
			auto signer = test::GenerateRandomByteArray<Key>();
			auto notification = CreateNotification(signer, publicKeyAdditions, publicKeyDeletions);

			auto cache = test::MultisigCacheFactory::Create();

			// Assert:
			AssertValidationResult(expectedResult, cache, Height(100), notification);
		}
	}

	TEST(TEST_CLASS, CanAddCosignatoriesWhenAccountIsUnknown) {
		AssertMultisigAccountIsUnknown(ValidationResult::Success, test::GenerateRandomDataVector<Key>(3), {});
	}

	TEST(TEST_CLASS, CannotRemoveCosignatoryWhenAccountIsUnknown) {
		AssertMultisigAccountIsUnknown(
				Failure_Multisig_Unknown_Multisig_Account,
				test::GenerateRandomDataVector<Key>(3),
				test::GenerateRandomDataVector<Key>(1));
	}

	namespace {
		enum class CosignatoryModificationAction { Add, Del };
		enum class CosignatoryType { Existing, New };

		constexpr auto Add = CosignatoryModificationAction::Add;
		constexpr auto Del = CosignatoryModificationAction::Del;

		struct OperationAndType {
			CosignatoryModificationAction Operation;
			CosignatoryType Type;
		};

		void AssertCosignatoriesModifications(ValidationResult expectedResult, const std::vector<OperationAndType>& settings) {
			// Arrange: first key is a signer (multisig account key)
			auto keys = test::GenerateKeys(1 + settings.size());
			const auto& signer = keys[0];

			std::vector<Key> publicKeyAdditions;
			std::vector<Key> publicKeyDeletions;
			for (auto i = 0u; i < settings.size(); ++i) {
				if (CosignatoryModificationAction::Add == settings[i].Operation)
					publicKeyAdditions.push_back(keys[1 + i]);
				else
					publicKeyDeletions.push_back(keys[1 + i]);
			}

			auto notification = CreateNotification(signer, publicKeyAdditions, publicKeyDeletions);
			auto cache = test::MultisigCacheFactory::Create();

			// - create multisig entry in cache
			{
				auto delta = cache.createDelta();
				auto& multisigDelta = delta.sub<cache::MultisigCache>();
				const auto& multisigAccountKey = keys[0];
				multisigDelta.insert(state::MultisigEntry(multisigAccountKey));
				auto& cosignatories = multisigDelta.find(multisigAccountKey).get().cosignatoryPublicKeys();
				for (auto i = 0u; i < settings.size(); ++i) {
					if (CosignatoryType::Existing == settings[i].Type)
						cosignatories.insert(keys[1 + i]);
				}

				cache.commit(Height(1));
			}

			// Assert:
			AssertValidationResult(expectedResult, cache, Height(100), notification);
		}
	}

	// region single

	TEST(TEST_CLASS, CanAddCosignatoryWhenNotPresent) {
		AssertCosignatoriesModifications(ValidationResult::Success, { { Add, CosignatoryType::New } });
	}

	TEST(TEST_CLASS, CannotAddExistingCosignatory) {
		AssertCosignatoriesModifications(Failure_Multisig_Already_A_Cosignatory, { { Add, CosignatoryType::Existing } });
	}

	TEST(TEST_CLASS, CanRemoveExistingCosignatory) {
		AssertCosignatoriesModifications(ValidationResult::Success, { { Del, CosignatoryType::Existing } });
	}

	TEST(TEST_CLASS, CannotRemoveCosignatoryWhenNotPresent) {
		AssertCosignatoriesModifications(Failure_Multisig_Not_A_Cosignatory, { { Del, CosignatoryType::New } });
	}

	// endregion

	// region multiple success

	TEST(TEST_CLASS, CanAddCosignatoriesWhenNotPresent) {
		AssertCosignatoriesModifications(ValidationResult::Success, {
				{ Add, CosignatoryType::New },
				{ Add, CosignatoryType::New },
				{ Add, CosignatoryType::New }
		});
	}

	TEST(TEST_CLASS, CanRemoveExistingCosignatories) {
		// Assert: note that stateless validator will reject multiple deletions
		AssertCosignatoriesModifications(ValidationResult::Success, {
				{ Del, CosignatoryType::Existing },
				{ Del, CosignatoryType::Existing },
				{ Del, CosignatoryType::Existing }
		});
	}

	TEST(TEST_CLASS, CanAddNewAndRemoveExistingCosignatories) {
		AssertCosignatoriesModifications(ValidationResult::Success, {
				{ Add, CosignatoryType::New },
				{ Del, CosignatoryType::Existing },
				{ Add, CosignatoryType::New },
				{ Del, CosignatoryType::Existing }
		});
	}

	// endregion

	// region multiple successes, single failure

	TEST(TEST_CLASS, CannotAddExistingCosignatory_Multiple) {
		AssertCosignatoriesModifications(Failure_Multisig_Already_A_Cosignatory, {
				{ Add, CosignatoryType::New },
				{ Add, CosignatoryType::Existing },
				{ Add, CosignatoryType::New }
		});
	}

	TEST(TEST_CLASS, CannotRemoveCosignatoryWhenNotPresent_Multiple) {
		AssertCosignatoriesModifications(Failure_Multisig_Not_A_Cosignatory, {
				{ Del, CosignatoryType::Existing },
				{ Del, CosignatoryType::New },
				{ Del, CosignatoryType::Existing }
		});
	}

	// endregion

	// region multiple failures

	TEST(TEST_CLASS, AdditionFailuresDominateDeletionFailures) {
		AssertCosignatoriesModifications(Failure_Multisig_Already_A_Cosignatory, {
				{ Add, CosignatoryType::Existing },
				{ Del, CosignatoryType::New }
		});

		AssertCosignatoriesModifications(Failure_Multisig_Already_A_Cosignatory, {
				{ Del, CosignatoryType::New },
				{ Add, CosignatoryType::Existing }
		});
	}

	// endregion
}}
