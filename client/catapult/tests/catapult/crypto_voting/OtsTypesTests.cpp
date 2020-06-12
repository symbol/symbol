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

#include "catapult/crypto_voting/OtsTypes.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS OtsTypesTests

	// region ots options operators

	namespace {
		std::vector<OtsOptions> GenerateIncreasingOtsOptionsValues() {
			return {
					{ 0, 0 },
					{ 1, 0 },
					{ 4, 0 },
					{ 4, 5 }
			};
		}

		std::string OtsOptionsToString(const OtsOptions& options) {
			std::stringstream out;
			out << "(" << options.MaxRounds << ", " << options.MaxSubRounds << ")";
			return out.str();
		}
	}

	DEFINE_EQUALITY_TESTS_WITH_CUSTOM_FORMATTER_WITH_PREFIX(
			TEST_CLASS,
			GenerateIncreasingOtsOptionsValues(),
			OtsOptionsToString,
			OtsOptions_)

	// endregion

	// region step identifier operators

	namespace {
		std::vector<StepIdentifier> GenerateIncreasingStepIdValues() {
			return {
					{ 5, 0, 0 },
					{ 10, 0, 0 },
					{ 11, 0, 0 },
					{ 11, 1, 0 },
					{ 11, 4, 0 },
					{ 11, 4, 5 }
			};
		}
	}

	DEFINE_EQUALITY_AND_COMPARISON_TESTS_WITH_PREFIX(TEST_CLASS, GenerateIncreasingStepIdValues(), StepIdentifier_)

	TEST(TEST_CLASS, StepIdentifier_CanOutputStepIdentifier) {
		// Arrange:
		StepIdentifier stepdIdentifier{ 11, 5, 215 };

		// Act:
		auto str = test::ToString(stepdIdentifier);

		// Assert:
		EXPECT_EQ("(11, 5, 215)", str);
	}

	// endregion

	// region step identifier size + alignment

#define STEP_IDENTIFIER_FIELDS FIELD(Point) FIELD(Round) FIELD(SubRound)

	TEST(TEST_CLASS, StepIdentifierHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += sizeof(StepIdentifier::X);
		STEP_IDENTIFIER_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(StepIdentifier));
		EXPECT_EQ(24u, sizeof(StepIdentifier));
	}

	TEST(TEST_CLASS, StepIdentifierHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(StepIdentifier, X);
		STEP_IDENTIFIER_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(StepIdentifier) % 8);
	}

#undef STEP_IDENTIFIER_FIELDS

	// endregion

	// region ots tree signature operators

	namespace {
		void MutatePair(OtsParentPublicKeySignaturePair& pair, uint8_t flags) {
			if (flags & 1)
				pair.ParentPublicKey[0] = 1;

			if (flags & 2)
				pair.Signature[0] = 1;
		}

		OtsTreeSignature GenerateOtsTreeSignature(uint8_t flags) {
			auto signature = OtsTreeSignature();
			MutatePair(signature.Root, flags & 3);
			MutatePair(signature.Middle, (flags >> 2) & 3);
			MutatePair(signature.Top, (flags >> 4) & 3);
			MutatePair(signature.Bottom, (flags >> 6) & 3);
			return signature;
		}

		std::vector<OtsTreeSignature> GenerateIncreasingOtsTreeSignatureValues() {
			std::vector<OtsTreeSignature> signatures;
			for (auto flags = 0u; flags < 256; ++flags)
				signatures.push_back(GenerateOtsTreeSignature(static_cast<uint8_t>(flags)));

			return signatures;
		}

		auto& Format(std::ostream& out, const OtsParentPublicKeySignaturePair& pair) {
			out << static_cast<int>(pair.ParentPublicKey[0]) << "," << static_cast<int>(pair.Signature[0]);
			return out;
		}

		std::string SignatureToString(const OtsTreeSignature& signature) {
			std::stringstream out;
			out << "signature{ ";
			Format(out, signature.Root) << ";";
			Format(out, signature.Top) << ";";
			Format(out, signature.Middle) << ";";
			Format(out, signature.Bottom) << " }";
			return out.str();
		}
	}

	DEFINE_EQUALITY_TESTS_WITH_CUSTOM_FORMATTER_WITH_PREFIX(
			TEST_CLASS,
			GenerateIncreasingOtsTreeSignatureValues(),
			SignatureToString,
			OtsTreeSignature_)

	// endregion

	// region ots tree signature size + alignment

#define PUBLIC_KEY_SIGNATURE_PAIR_FIELDS FIELD(ParentPublicKey) FIELD(Signature)

	TEST(TEST_CLASS, OtsParentPublicKeySignaturePairHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += sizeof(OtsParentPublicKeySignaturePair::X);
		PUBLIC_KEY_SIGNATURE_PAIR_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(OtsParentPublicKeySignaturePair));
		EXPECT_EQ(96u, sizeof(OtsParentPublicKeySignaturePair));
	}

	TEST(TEST_CLASS, OtsParentPublicKeySignaturePairHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(OtsParentPublicKeySignaturePair, X);
		PUBLIC_KEY_SIGNATURE_PAIR_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(OtsParentPublicKeySignaturePair) % 8);
	}

#undef PUBLIC_KEY_SIGNATURE_PAIR_FIELDS

#define OTS_TREE_SIGNATURE_FIELDS FIELD(Root) FIELD(Top) FIELD(Middle) FIELD(Bottom)

	TEST(TEST_CLASS, OtsTreeSignatureHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += sizeof(OtsTreeSignature::X);
		OTS_TREE_SIGNATURE_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(OtsTreeSignature));
		EXPECT_EQ(384u, sizeof(OtsTreeSignature));
	}

	TEST(TEST_CLASS, OtsTreeSignatureHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(OtsTreeSignature, X);
		OTS_TREE_SIGNATURE_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(OtsTreeSignature) % 8);
	}

#undef OTS_TREE_SIGNATURE_FIELDS

	// endregion
}}
