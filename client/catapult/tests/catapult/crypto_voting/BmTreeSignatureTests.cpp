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

#include "catapult/crypto_voting/BmTreeSignature.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS BmTreeSignatureTests

	// region size + alignment (BmTreeSignature::ParentPublicKeySignaturePair)

#define PUBLIC_KEY_SIGNATURE_PAIR_FIELDS FIELD(ParentPublicKey) FIELD(Signature)

	TEST(TEST_CLASS, ParentPublicKeySignaturePairHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(BmTreeSignature::ParentPublicKeySignaturePair::X)>();
		PUBLIC_KEY_SIGNATURE_PAIR_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(BmTreeSignature::ParentPublicKeySignaturePair));
		EXPECT_EQ(144u, sizeof(BmTreeSignature::ParentPublicKeySignaturePair));
	}

	TEST(TEST_CLASS, ParentPublicKeySignaturePairHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(BmTreeSignature::ParentPublicKeySignaturePair, X);
		PUBLIC_KEY_SIGNATURE_PAIR_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(BmTreeSignature::ParentPublicKeySignaturePair) % 8);
	}

#undef PUBLIC_KEY_SIGNATURE_PAIR_FIELDS

	// endregion

	// region size + alignment (BmTreeSignature)

#define TREE_SIGNATURE_FIELDS FIELD(Root) FIELD(Top) FIELD(Bottom)

	TEST(TEST_CLASS, TreeSignatureHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(BmTreeSignature::X)>();
		TREE_SIGNATURE_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(BmTreeSignature));
		EXPECT_EQ(432u, sizeof(BmTreeSignature));
	}

	TEST(TEST_CLASS, TreeSignatureHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(BmTreeSignature, X);
		TREE_SIGNATURE_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(BmTreeSignature) % 8);
	}

#undef TREE_SIGNATURE_FIELDS

	// endregion

	// region operators (BmTreeSignature)

	namespace {
		void MutatePair(BmTreeSignature::ParentPublicKeySignaturePair& pair, uint8_t flags) {
			if (flags & 1)
				pair.ParentPublicKey[0] = 1;

			if (flags & 2)
				pair.Signature[0] = 1;
		}

		BmTreeSignature GenerateTreeSignature(uint8_t flags) {
			auto signature = BmTreeSignature();
			MutatePair(signature.Root, flags & 3);
			MutatePair(signature.Top, (flags >> 2) & 3);
			MutatePair(signature.Bottom, (flags >> 4) & 3);
			return signature;
		}

		std::vector<BmTreeSignature> GenerateIncreasingTreeSignatureValues() {
			std::vector<BmTreeSignature> signatures;
			for (auto flags = 0u; flags < 64; ++flags)
				signatures.push_back(GenerateTreeSignature(static_cast<uint8_t>(flags)));

			return signatures;
		}

		auto& Format(std::ostream& out, const BmTreeSignature::ParentPublicKeySignaturePair& pair) {
			out << static_cast<int>(pair.ParentPublicKey[0]) << "," << static_cast<int>(pair.Signature[0]);
			return out;
		}

		std::string SignatureToString(const BmTreeSignature& signature) {
			std::stringstream out;
			out << "signature{ ";
			Format(out, signature.Root) << ";";
			Format(out, signature.Top) << ";";
			Format(out, signature.Bottom) << " }";
			return out.str();
		}
	}

	DEFINE_EQUALITY_TESTS_WITH_CUSTOM_FORMATTER(TEST_CLASS, GenerateIncreasingTreeSignatureValues(), SignatureToString)

	// endregion
}}
