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

#define TEST_CLASS MultisigLoopAndLevelValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MultisigLoopAndLevel, 0)

	namespace {
		constexpr auto Num_Network_Accounts = 14 + 4 + 2; // last two addresses are unassigned (and not in multisig cache)

		auto CreateCacheMultisigNetwork(const std::vector<Address>& addresses) {
			auto cache = test::MultisigCacheFactory::Create();
			auto cacheDelta = cache.createDelta();

			// D   2 \   / 7 - A - B |
			//   \ 3 - 6 - 8     \   |
			// 1 - 4 /   \ 9       C |
			//   \                   |
			// 0 - 5                 |
			test::MakeMultisig(cacheDelta, addresses[13], { addresses[4] });
			test::MakeMultisig(cacheDelta, addresses[1], { addresses[4], addresses[5] });
			test::MakeMultisig(cacheDelta, addresses[0], { addresses[5] });

			test::MakeMultisig(cacheDelta, addresses[2], { addresses[6] });
			test::MakeMultisig(cacheDelta, addresses[3], { addresses[6] });
			test::MakeMultisig(cacheDelta, addresses[4], { addresses[6] });

			test::MakeMultisig(cacheDelta, addresses[6], { addresses[7], addresses[8], addresses[9] });
			test::MakeMultisig(cacheDelta, addresses[7], { addresses[10] });
			test::MakeMultisig(cacheDelta, addresses[10], { addresses[11], addresses[12] });

			// E - F - 10 |
			//       \ 11 |
			test::MakeMultisig(cacheDelta, addresses[14], { addresses[15] });
			test::MakeMultisig(cacheDelta, addresses[15], { addresses[16], addresses[17] });

			cache.commit(Height());
			return cache;
		}

		void AssertValidationResult(
				ValidationResult expectedResult,
				int8_t maxMultisigDepth,
				size_t multisigIndex,
				size_t cosignatoryIndex) {
			// Arrange:
			auto addresses = test::GenerateRandomDataVector<Address>(Num_Network_Accounts);
			auto cache = CreateCacheMultisigNetwork(addresses);

			model::MultisigNewCosignatoryNotification notification(
					addresses[multisigIndex],
					test::UnresolveXor(addresses[cosignatoryIndex]));
			auto pValidator = CreateMultisigLoopAndLevelValidator(static_cast<uint8_t>(maxMultisigDepth));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "multisigIndex " << multisigIndex << ", cosignatoryIndex " << cosignatoryIndex;
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
		RunMaxMultisigDepthTests(ValidationResult::Success, 10);
	}

	TEST(TEST_CLASS, CanCreateMultisigChainWithMaxDepth) {
		RunMaxMultisigDepthTests(ValidationResult::Success, 0);
	}

	TEST(TEST_CLASS, CannotCreateMultisigChainWithGreaterThanMaxDepth) {
		RunMaxMultisigDepthTests(Failure_Multisig_Max_Multisig_Depth, -1);
	}

	// endregion

	// region loop check

	TEST(TEST_CLASS, CannotCreateSelfLoop) {
		AssertValidationResult(Failure_Multisig_Loop, 100, 0, 0); // in cache
		AssertValidationResult(Failure_Multisig_Loop, 100, 18, 18); // not in cache
	}

	TEST(TEST_CLASS, CannotCreateMultiAccountLoop) {
		AssertValidationResult(Failure_Multisig_Loop, 100, 6, 3);
		AssertValidationResult(Failure_Multisig_Loop, 100, 6, 13);
		AssertValidationResult(Failure_Multisig_Loop, 100, 10, 1);

		AssertValidationResult(Failure_Multisig_Loop, 100, 8, 6);
		AssertValidationResult(Failure_Multisig_Loop, 100, 12, 6);
		AssertValidationResult(Failure_Multisig_Loop, 100, 12, 2);
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
		//  (a) is allowable because 8 is cosignatory of 6 and A
		//  (b) is allowable because A is cosignatory of 7 and 8
		AssertValidationResult(ValidationResult::Success, 100, 10, 8);
		AssertValidationResult(ValidationResult::Success, 100, 8, 10);

		// - { 6, 7, A, C, 9 }
		AssertValidationResult(ValidationResult::Success, 100, 12, 9);

		// - { 1, 4, 6, 7, A, B, 5 }
		AssertValidationResult(ValidationResult::Success, 100, 11, 5);
	}

	TEST(TEST_CLASS, CanCreateMultilevelCosignatory) {
		// Assert: allow 7 to cosign 2 in addition to 6
		AssertValidationResult(ValidationResult::Success, 100, 2, 7);

		// - allow C to cosign D in addition to A
		AssertValidationResult(ValidationResult::Success, 100, 13, 12);
	}

	// endregion
}}
