/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Functional.h"

namespace catapult { namespace model {

#define TEST_CLASS StepIdentifierTests

	// region constructor

	TEST(TEST_CLASS, CanCreateDefaultStepIdentifier) {
		// Act:
		auto stepIdentifier = StepIdentifier();

		// Assert:
		EXPECT_EQ(FinalizationEpoch(), stepIdentifier.Epoch);
		EXPECT_EQ(StepIdentifier::FinalizationPointStage(), stepIdentifier.PointStage);

		EXPECT_EQ(test::CreateFinalizationRound(0, 0), stepIdentifier.Round());
		EXPECT_EQ(FinalizationStage::Prevote, stepIdentifier.Stage());
	}

	TEST(TEST_CLASS, CanCreatePrevoteStepIdentifier) {
		// Act:
		auto stepIdentifier = StepIdentifier{ FinalizationEpoch(7), FinalizationPoint(11), FinalizationStage::Prevote };

		// Assert:
		EXPECT_EQ(FinalizationEpoch(7), stepIdentifier.Epoch);
		EXPECT_EQ(StepIdentifier::FinalizationPointStage(22), stepIdentifier.PointStage);

		EXPECT_EQ(test::CreateFinalizationRound(7, 11), stepIdentifier.Round());
		EXPECT_EQ(FinalizationStage::Prevote, stepIdentifier.Stage());
	}

	TEST(TEST_CLASS, CanCreatePrecommitStepIdentifier) {
		// Act:
		auto stepIdentifier = StepIdentifier{ FinalizationEpoch(7), FinalizationPoint(11), FinalizationStage::Precommit };

		// Assert:
		EXPECT_EQ(FinalizationEpoch(7), stepIdentifier.Epoch);
		EXPECT_EQ(StepIdentifier::FinalizationPointStage(23), stepIdentifier.PointStage);

		EXPECT_EQ(test::CreateFinalizationRound(7, 11), stepIdentifier.Round());
		EXPECT_EQ(FinalizationStage::Precommit, stepIdentifier.Stage());
	}

	// endregion

	// region size + alignment

#define STEP_IDENTIFIER_FIELDS FIELD(Epoch) FIELD(PointStage)

	TEST(TEST_CLASS, StepIdentifierHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(StepIdentifier::X)>();
		STEP_IDENTIFIER_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(StepIdentifier));
		EXPECT_EQ(8u, sizeof(StepIdentifier));
	}

	TEST(TEST_CLASS, StepIdentifierHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(StepIdentifier, X);
		STEP_IDENTIFIER_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(StepIdentifier) % 8);
	}

#undef STEP_IDENTIFIER_FIELDS

	// endregion

	// region operators

	namespace {
		std::vector<StepIdentifier> GenerateIncreasingStepIdentifierValues() {
			return {
				{ FinalizationEpoch(7), FinalizationPoint(5), FinalizationStage::Prevote },
				{ FinalizationEpoch(7), FinalizationPoint(10), FinalizationStage::Prevote },
				{ FinalizationEpoch(7), FinalizationPoint(11), FinalizationStage::Prevote },
				{ FinalizationEpoch(7), FinalizationPoint(11), FinalizationStage::Precommit },
				{ FinalizationEpoch(8), FinalizationPoint(11), FinalizationStage::Prevote },
				{ FinalizationEpoch(8), FinalizationPoint(11), FinalizationStage::Precommit}
			};
		}
	}

	DEFINE_EQUALITY_AND_COMPARISON_TESTS(TEST_CLASS, GenerateIncreasingStepIdentifierValues())

	TEST(TEST_CLASS, StepIdentifier_CanOutputPrevote) {
		// Arrange:
		auto stepIdentifier = StepIdentifier{ FinalizationEpoch(7), FinalizationPoint(11), FinalizationStage::Prevote };

		// Act:
		auto str = test::ToString(stepIdentifier);

		// Assert:
		EXPECT_EQ("(7, 11) prevote", str);
	}

	TEST(TEST_CLASS, StepIdentifier_CanOutputPrecommit) {
		// Arrange:
		auto stepIdentifier = StepIdentifier{ FinalizationEpoch(7), FinalizationPoint(11), FinalizationStage::Precommit };

		// Act:
		auto str = test::ToString(stepIdentifier);

		// Assert:
		EXPECT_EQ("(7, 11) precommit", str);
	}

	// endregion

	// region StepIdentifierToBmKeyIdentifier

	namespace {
		std::vector<StepIdentifier> GenerateValidStepIdentifierValues() {
			return {
				{ FinalizationEpoch(10), FinalizationPoint(1), FinalizationStage::Prevote },
				{ FinalizationEpoch(20), FinalizationPoint(2), FinalizationStage::Prevote },
				{ FinalizationEpoch(21), FinalizationPoint(3), FinalizationStage::Precommit },
				{ FinalizationEpoch(22), FinalizationPoint(4), FinalizationStage::Prevote },
				{ FinalizationEpoch(23), FinalizationPoint(5), FinalizationStage::Precommit }
			};
		}
	}

	TEST(TEST_CLASS, StepIdentifierToBmKeyIdentifierProducesCorrectValues) {
		// Arrange:
		auto identifiers = GenerateValidStepIdentifierValues();
		auto expectedKeyIdentifiers = std::vector<crypto::BmKeyIdentifier>{
			{ 10 }, { 20 }, { 21 }, { 22 }, { 23 }
		};

		// Act:
		auto keyIdentifiers = test::Apply(true, identifiers, [](const auto& stepIdentifier) {
			return StepIdentifierToBmKeyIdentifier(stepIdentifier);
		});

		// Assert:
		EXPECT_EQ(expectedKeyIdentifiers, keyIdentifiers);
	}

	// endregion
}}
