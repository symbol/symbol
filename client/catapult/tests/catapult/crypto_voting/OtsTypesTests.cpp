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
#include "tests/test/nodeps/Comparison.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

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

	DEFINE_EQUALITY_AND_COMPARISON_TESTS(StepIdentifierTests, GenerateIncreasingStepIdValues())

	TEST(TEST_CLASS, CanOutputStepIdentifier) {
		// Arrange:
		StepIdentifier stepdIdentifier{ 11, 5, 215 };

		// Act:
		auto str = test::ToString(stepdIdentifier);

		// Assert:
		EXPECT_EQ("(11, 5, 215)", str);
	}

	// endregion

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

	DEFINE_EQUALITY_TESTS_WITH_CUSTOM_FORMATTER(OtsOptionsTests, GenerateIncreasingOtsOptionsValues(), OtsOptionsToString)

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

	DEFINE_EQUALITY_TESTS_WITH_CUSTOM_FORMATTER(OtsTreeSignatureTests, GenerateIncreasingOtsTreeSignatureValues(), SignatureToString)

	// endregion
}}
