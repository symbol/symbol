#include "src/validators/Validators.h"
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS ModifyMultisigLoopAndLevelValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(ModifyMultisigLoopAndLevel, 0)

	namespace {
		constexpr auto Num_Network_Accounts = 14 + 4 + 2; // last two keys are unassigned (and not in multisig cache)

		auto CreateCacheMultisigNetwork(const std::vector<Key>& keys) {
			auto cache = test::MultisigCacheFactory::Create();
			auto cacheDelta = cache.createDelta();

			// D   2 \   / 7 - A - B |
			//   \ 3 - 6 - 8     \   |
			// 1 - 4 /   \ 9       C |
			//   \                   |
			// 0 - 5                 |
			test::MakeMultisig(cacheDelta, keys[13], { keys[4] });
			test::MakeMultisig(cacheDelta, keys[1], { keys[4], keys[5] });
			test::MakeMultisig(cacheDelta, keys[0], { keys[5] });

			test::MakeMultisig(cacheDelta, keys[2], { keys[6] });
			test::MakeMultisig(cacheDelta, keys[3], { keys[6] });
			test::MakeMultisig(cacheDelta, keys[4], { keys[6] });

			test::MakeMultisig(cacheDelta, keys[6], { keys[7], keys[8], keys[9] });
			test::MakeMultisig(cacheDelta, keys[7], { keys[10] });
			test::MakeMultisig(cacheDelta, keys[10], { keys[11], keys[12] });

			// E - F - 10 |
			//       \ 11 |
			test::MakeMultisig(cacheDelta, keys[14], { keys[15] });
			test::MakeMultisig(cacheDelta, keys[15], { keys[16], keys[17] });

			cache.commit(Height());
			return cache;
		}

		void AssertValidationResult(
				ValidationResult expectedResult,
				int8_t maxMultisigDepth,
				size_t multisigAccountIndex,
				size_t cosignatoryKeyIndex) {
			// Arrange:
			// - setup cache
			auto keys = test::GenerateKeys(Num_Network_Accounts);
			auto cache = CreateCacheMultisigNetwork(keys);

			// - create the validator context
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(Height(), readOnlyCache);

			model::ModifyMultisigNewCosignerNotification notification(keys[multisigAccountIndex], keys[cosignatoryKeyIndex]);
			auto pValidator = CreateModifyMultisigLoopAndLevelValidator(static_cast<uint8_t>(maxMultisigDepth));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "multisigAccountIndex " << multisigAccountIndex
					<< ", cosignatoryKeyIndex " << cosignatoryKeyIndex;
		}
	}

	// region max multisig depth

	namespace {
		void RunMaxMultisigDepthTests(ValidationResult expectedResult, int8_t offset) {
			// Assert: basic tests (note that all indexes below in comments are hex)
			//   0 - 5 => E - F - 10 => 4 levels
			AssertValidationResult(expectedResult, 4 + offset, 5, 14);

			//   E - F - 10 => 0 - 5 => 4 levels
			AssertValidationResult(expectedResult, 4 + offset, 16, 0);

			//   E - F - 10 => D - 4 - 6 - 7 - A - B => 8 levels
			AssertValidationResult(expectedResult, 8 + offset, 16, 13);

			// - single nodes (in cache)
			//  D => E - F - 10 => 3 levels
			AssertValidationResult(expectedResult, 3 + offset, 13, 14);

			//  E - F - 10 => C => 3 levels
			AssertValidationResult(expectedResult, 3 + offset, 16, 12);

			// - single nodes (not in cache)
			//  12 => E - F - 10 => 3 levels
			AssertValidationResult(expectedResult, 3 + offset, 18, 14);

			//  E - F - 10 => 12 => 3 levels
			AssertValidationResult(expectedResult, 3 + offset, 16, 18);
		}
	}

	TEST(TEST_CLASS, CanCreateMultisigChainWithLessThanMaxDepth) {
		// Assert:
		RunMaxMultisigDepthTests(ValidationResult::Success, 10);
	}

	TEST(TEST_CLASS, CanCreateMultisigChainWithMaxDepth) {
		// Assert:
		RunMaxMultisigDepthTests(ValidationResult::Success, 0);
	}

	TEST(TEST_CLASS, CannotCreateMultisigChainWithGreaterThanMaxDepth) {
		// Assert:
		RunMaxMultisigDepthTests(Failure_Multisig_Modify_Max_Multisig_Depth, -1);
	}

	// endregion

	// region loop check

	TEST(TEST_CLASS, CannotCreateSelfLoop) {
		// Assert:
		AssertValidationResult(Failure_Multisig_Modify_Loop, 100, 0, 0); // in cache
		AssertValidationResult(Failure_Multisig_Modify_Loop, 100, 18, 18); // not in cache
	}

	TEST(TEST_CLASS, CannotCreateMultiAccountLoop) {
		// Assert:
		AssertValidationResult(Failure_Multisig_Modify_Loop, 100, 6, 3);
		AssertValidationResult(Failure_Multisig_Modify_Loop, 100, 6, 13);
		AssertValidationResult(Failure_Multisig_Modify_Loop, 100, 10, 1);

		AssertValidationResult(Failure_Multisig_Modify_Loop, 100, 8, 6);
		AssertValidationResult(Failure_Multisig_Modify_Loop, 100, 12, 6);
		AssertValidationResult(Failure_Multisig_Modify_Loop, 100, 12, 2);
	}

	TEST(TEST_CLASS, CanCreateClosedCircuit) {
		// Assert: { 6, 7, 8, A }
		//  there are two possible arrangments:
		//   (a)      |      (b)
		//  6 -+ 7    |    6 -+ 7
		//  |    |    |    |    |
		//  +    +    |    +    +
		//  8 +- A    |    8 -+ A
		//
		//  (a) is allowable because 8 is cosigner of 6 and A
		//  (b) is allowable because A is cosigner of 7 and 8
		AssertValidationResult(ValidationResult::Success, 100, 10, 8);
		AssertValidationResult(ValidationResult::Success, 100, 8, 10);

		// - { 6, 7, A, C, 9 }
		AssertValidationResult(ValidationResult::Success, 100, 12, 9);

		// - { 1, 4, 6, 7, A, B, 5 }
		AssertValidationResult(ValidationResult::Success, 100, 11, 5);
	}

	TEST(TEST_CLASS, CanCreateMultilevelCosigner) {
		// Assert: allow 7 to cosign 2 in addition to 6
		AssertValidationResult(ValidationResult::Success, 100, 2, 7);

		// - allow C to cosign D in addition to A
		AssertValidationResult(ValidationResult::Success, 100, 13, 12);
	}

	// endregion
}}
