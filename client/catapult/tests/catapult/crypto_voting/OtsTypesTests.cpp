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

	// region ots key identifier operators

	namespace {
		std::vector<OtsKeyIdentifier> GenerateIncreasingOtsKeyIdentifierValues() {
			return {
					{ 5, 0 },
					{ 10, 0 },
					{ 10, 1 },
					{ 10, 4 }
			};
		}
	}

	DEFINE_EQUALITY_AND_COMPARISON_TESTS_WITH_PREFIX(TEST_CLASS, GenerateIncreasingOtsKeyIdentifierValues(), OtsKeyIdentifier_)

	TEST(TEST_CLASS, OtsKeyIdentifier_CanOutput) {
		// Arrange:
		OtsKeyIdentifier keyIdentifier{ 11, 5 };

		// Act:
		auto str = test::ToString(keyIdentifier);

		// Assert:
		EXPECT_EQ("(11, 5)", str);
	}

	// endregion

	// region ots key identifier size + alignment

#define OTS_KEY_IDENTIFIER_FIELDS FIELD(BatchId) FIELD(KeyId)

	TEST(TEST_CLASS, OtsKeyIdentifierHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(OtsKeyIdentifier::X)>();
		OTS_KEY_IDENTIFIER_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(OtsKeyIdentifier));
		EXPECT_EQ(16u, sizeof(OtsKeyIdentifier));
	}

	TEST(TEST_CLASS, OtsKeyIdentifierHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(OtsKeyIdentifier, X);
		OTS_KEY_IDENTIFIER_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(OtsKeyIdentifier) % 8);
	}

#undef OTS_KEY_IDENTIFIER_FIELDS

	// endregion

	// region ots options size + alignment

#define OTS_OPTIONS_FIELDS FIELD(Dilution) FIELD(StartKeyIdentifier) FIELD(EndKeyIdentifier)

	TEST(TEST_CLASS, OtsOptionsHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(OtsOptions::X)>();
		OTS_OPTIONS_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(OtsOptions));
		EXPECT_EQ(40u, sizeof(OtsOptions));
	}

	TEST(TEST_CLASS, OtsOptionsHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(OtsOptions, X);
		OTS_OPTIONS_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(OtsOptions) % 8);
	}

#undef OTS_OPTIONS_FIELDS

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
			MutatePair(signature.Top, (flags >> 2) & 3);
			MutatePair(signature.Bottom, (flags >> 4) & 3);
			return signature;
		}

		std::vector<OtsTreeSignature> GenerateIncreasingOtsTreeSignatureValues() {
			std::vector<OtsTreeSignature> signatures;
			for (auto flags = 0u; flags < 64; ++flags)
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

#define FIELD(X) expectedSize += SizeOf32<decltype(OtsParentPublicKeySignaturePair::X)>();
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

#define OTS_TREE_SIGNATURE_FIELDS FIELD(Root) FIELD(Top) FIELD(Bottom)

	TEST(TEST_CLASS, OtsTreeSignatureHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(OtsTreeSignature::X)>();
		OTS_TREE_SIGNATURE_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(OtsTreeSignature));
		EXPECT_EQ(288u, sizeof(OtsTreeSignature));
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
