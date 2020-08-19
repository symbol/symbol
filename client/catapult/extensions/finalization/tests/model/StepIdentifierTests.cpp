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

#include "finalization/src/model/StepIdentifier.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Functional.h"

namespace catapult { namespace model {

#define TEST_CLASS StepIdentifierTests

	// region step identifier operators

	namespace {
		auto CreateStepIdentifier(uint64_t point, uint64_t round) {
			return StepIdentifier{ FinalizationPoint(point), round };
		}

		std::vector<StepIdentifier> GenerateIncreasingStepIdentifierValues() {
			return {
				CreateStepIdentifier(5, 0),
				CreateStepIdentifier(10, 0),
				CreateStepIdentifier(11, 0),
				CreateStepIdentifier(11, 1),
				CreateStepIdentifier(11, 4)
			};
		}
	}

	DEFINE_EQUALITY_AND_COMPARISON_TESTS(TEST_CLASS, GenerateIncreasingStepIdentifierValues())

	TEST(TEST_CLASS, StepIdentifier_CanOutput) {
		// Arrange:
		auto stepIdentifier = CreateStepIdentifier(11, 5);

		// Act:
		auto str = test::ToString(stepIdentifier);

		// Assert:
		EXPECT_EQ("(11, 5)", str);
	}

	// endregion

	// region step identifier size + alignment

#define STEP_IDENTIFIER_FIELDS FIELD(Point) FIELD(Round)

	TEST(TEST_CLASS, StepIdentifierHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(StepIdentifier::X)>();
		STEP_IDENTIFIER_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(StepIdentifier));
		EXPECT_EQ(16u, sizeof(StepIdentifier));
	}

	TEST(TEST_CLASS, StepIdentifierHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(StepIdentifier, X);
		STEP_IDENTIFIER_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(StepIdentifier) % 8);
	}

#undef STEP_IDENTIFIER_FIELDS

	// endregion

	// region StepIdentifierToOtsKeyIdentifier

	namespace {
		std::vector<StepIdentifier> GenerateValidStepIdentifierValues() {
			return {
				CreateStepIdentifier(5, 1),
				CreateStepIdentifier(10, 1),
				CreateStepIdentifier(10, 2),
				CreateStepIdentifier(11, 1),
				CreateStepIdentifier(11, 2)
			};
		}
	}

	TEST(TEST_CLASS, StepIdentifierToOtsKeyIdentifierProducesCorrectValues) {
		// Arrange:
		auto identifiers = GenerateValidStepIdentifierValues();
		auto expectedKeyIdentifiers = std::vector<crypto::OtsKeyIdentifier>{
			{ 1, 3 }, { 2, 6 }, { 3, 0 }, { 3, 1 }, { 3, 2 }
		};

		// Act:
		auto keyIdentifiers = test::Apply(true, identifiers, [](const auto& stepIdentifier) {
			return StepIdentifierToOtsKeyIdentifier(stepIdentifier, 7);
		});

		// Assert:
		EXPECT_EQ(expectedKeyIdentifiers, keyIdentifiers);
	}

	TEST(TEST_CLASS, StepIdentifierToOtsKeyIdentifierProducesConflictingValuesForInvalidStepIdentifiers) {
		// Arrange: invalid, because round is greater than number of stages
		auto validIdentifier = CreateStepIdentifier(10, 1);
		auto invalidIdentifier = CreateStepIdentifier(8, 5);

		// Act:
		auto validKeyIdentifier = StepIdentifierToOtsKeyIdentifier(validIdentifier, 7);
		auto invalidKeyIdentifier = StepIdentifierToOtsKeyIdentifier(invalidIdentifier, 7);

		// Assert:
		EXPECT_EQ(validKeyIdentifier, invalidKeyIdentifier);
	}

	TEST(TEST_CLASS, StepIdentifierToOtsKeyIdentifierProducesCorrectValuesWhenDilutionIsOne) {
		// Arrange:
		auto identifiers = GenerateValidStepIdentifierValues();
		auto expectedKeyIdentifiers = std::vector<crypto::OtsKeyIdentifier>{
			{ 10, 0 }, { 20, 0 }, { 21, 0 }, { 22, 0 }, { 23, 0 }
		};

		// Act:
		auto keyIdentifiers = test::Apply(true, identifiers, [](const auto& stepIdentifier) {
			return StepIdentifierToOtsKeyIdentifier(stepIdentifier, 1);
		});

		// Assert:
		EXPECT_EQ(expectedKeyIdentifiers, keyIdentifiers);
	}

	// endregion
}}
