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

#include "catapult/crypto/Vrf.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/SharedKey.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/crypto/CurveUtils.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS VrfTests

	// region size + alignment

#define VRF_PROOF_FIELDS FIELD(Gamma) FIELD(VerificationHash) FIELD(Scalar)

	TEST(TEST_CLASS, VrfProofHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += sizeof(VrfProof::X);
		VRF_PROOF_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(VrfProof));
		EXPECT_EQ(80u, sizeof(VrfProof));
	}

	TEST(TEST_CLASS, VrfProofHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(VrfProof, X);
		VRF_PROOF_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(VrfProof) % 8);
	}

#undef VRF_PROOF_FIELDS

	// endregion

	// region test vectors

	namespace {
		struct TestVectorsInput {
			std::string SK;
			std::string Alpha;
		};

		struct TestVectorOutput {
			std::string Gamma;
			std::string VerificationHash;
			std::string Scalar;
			std::string Beta;
		};

		// data taken from: https://www.ietf.org/id/draft-irtf-cfrg-vrf-05.txt
		static std::vector<TestVectorsInput> SampleTestVectorsInput() {
			return {
				{ "9D61B19DEFFD5A60BA844AF492EC2CC44449C5697B326919703BAC031CAE7F60", ""},
				{ "4CCD089B28FF96DA9DB6C346EC114E0F5B8A319F35ABA624DA8CF6ED4FB8A6FB", "72" },
				{ "C5AA8DF43F9F837BEDB7442F31DCB7B166D38535076F094B85CE3A2E0B4458F7", "af82" }
			};
		}

		static std::vector<TestVectorOutput> SampleTestVectorsOutput() {
			return {
				{
					"9275DF67A68C8745C0FF97B48201EE6DB447F7C93B23AE24CDC2400F52FDB08A",
					"1A6AC7EC71BF9C9C76E96EE4675EBFF6",
					"0625AF28718501047BFD87B810C2D2139B73C23BD69DE66360953A642C2A330A",
					"A64C292EC45F6B252828AFF9A02A0FE88D2FCC7F5FC61BB328F03F4C6C0657A9"
					"D26EFB23B87647FF54F71CD51A6FA4C4E31661D8F72B41FF00AC4D2EEC2EA7B3"
				},
				{
					"84A63E74ECA8FDD64E9972DCDA1C6F33D03CE3CD4D333FD6CC789DB12B5A7B9D",
					"03F1CB6B2BF7CD81A2A20BACF6E1C04E",
					"59F2FA16D9119C73A45A97194B504FB9A5C8CF37F6DA85E03368D6882E511008",
					"CDDAA399BB9C56D3BE15792E43A6742FB72B1D248A7F24FD5CC585B232C26C93"
					"4711393B4D97284B2BCCA588775B72DC0B0F4B5A195BC41F8D2B80B6981C784E"
				},
				{
					"ACA8ADE9B7F03E2B149637629F95654C94FC9053C225EC21E5838F193AF2B727",
					"B84AD849B0039AD38B41513FE5A66CDD",
					"2367737A84B488D62486BD2FB110B4801A46BFCA770AF98E059158AC563B690F",
					"D938B2012F2551B0E13A49568612EFFCBDCA2AED5D1D3A13F47E180E01218916"
					"E049837BD246F66D5058E56D3413DBBBAD964F5E9F160A81C9A1355DCD99B453"
				}
			};
		}
	}

	TEST(TEST_CLASS, VrfSampleTestVectors) {
		// Arrange:
		auto testVectorsInput = SampleTestVectorsInput();
		auto testVectorsOutput = SampleTestVectorsOutput();

		// Sanity:
		ASSERT_EQ(testVectorsInput.size(), testVectorsOutput.size());

		auto i = 0u;
		for (const auto& input : testVectorsInput) {
			// Arrange:
			auto keyPair = KeyPair::FromString(input.SK);
			auto alpha = test::HexStringToVector(input.Alpha);
			auto message = "at index " + std::to_string(i);

			// Act: compute proof
			auto vrfProof = GenerateVrfProof(alpha, keyPair);

			// Assert:
			auto verificationHash = testVectorsOutput[i].VerificationHash;
			EXPECT_EQ(utils::ParseByteArray<ProofGamma>(testVectorsOutput[i].Gamma), vrfProof.Gamma) << message;
			EXPECT_EQ(utils::ParseByteArray<ProofVerificationHash>(verificationHash), vrfProof.VerificationHash) << message;
			EXPECT_EQ(utils::ParseByteArray<ProofScalar>(testVectorsOutput[i].Scalar), vrfProof.Scalar) << message;

			// Act: verify proof and compute beta
			auto proofHash = VerifyVrfProof(vrfProof, alpha, keyPair.publicKey());

			// Assert:
			EXPECT_EQ(utils::ParseByteArray<Hash512>(testVectorsOutput[i].Beta), proofHash) << message;
			++i;
		}
	}

	// endregion

	// region VerifyVrfProof

	namespace {
		void AssertVerifyVrfProofFailsWhenProofIsCorrupted(const consumer<VrfProof&>& transform) {
			auto keyPair = KeyPair::FromString("9D61B19DEFFD5A60BA844AF492EC2CC44449C5697B326919703BAC031CAE7F60");
			auto alpha = test::HexStringToVector("af82");
			auto vrfProof = GenerateVrfProof(alpha, keyPair);

			// Sanity:
			auto proofHash = VerifyVrfProof(vrfProof, alpha, keyPair.publicKey());
			EXPECT_NE(Hash512(), proofHash);

			// Act: corrupt proof
			transform(vrfProof);
			proofHash = VerifyVrfProof(vrfProof, alpha, keyPair.publicKey());

			// Assert:
			EXPECT_EQ(Hash512(), proofHash);
		}
	}

	TEST(TEST_CLASS, VerifyVrfProofFailsWhenGammaIsNotOnTheCurve) {
		// Act: corrupt Gamma
		AssertVerifyVrfProofFailsWhenProofIsCorrupted([](auto& vrfProof) {
			vrfProof.Gamma = utils::ParseByteArray<ProofGamma>("4F91BE9568552181E01968999EFC09BFEB77A736B8F3188160B7769D7B9B9F6E");
		});
	}

	TEST(TEST_CLASS, VerifyVrfProofFailsWhenGammaIsWrongPointOnCurve) {
		// Act: corrupt Gamma
		AssertVerifyVrfProofFailsWhenProofIsCorrupted([](auto& vrfProof) {
			// valid public key
			vrfProof.Gamma = utils::ParseByteArray<ProofGamma>("C8C6D604F4D7B56B57247E8686168EEBB2BF8AE40DA7B912143773A77555420E");
		});
	}

	TEST(TEST_CLASS, VerifyVrfProofFailsWhenVerificationHashIsWrong) {
		// Act: corrupt VerificationHash
		AssertVerifyVrfProofFailsWhenProofIsCorrupted([](auto& vrfProof) {
			vrfProof.VerificationHash[4] ^= 0xFF;
		});
	}

	TEST(TEST_CLASS, VerifyVrfProofFailsWhenScalarIsWrong) {
		// Act: corrupt Scalar
		AssertVerifyVrfProofFailsWhenProofIsCorrupted([](auto& vrfProof) {
			vrfProof.Scalar[14] ^= 0xFF;
		});
	}

	TEST(TEST_CLASS, VerifyVrfProofFailsWhenScalarIsNotReduced) {
		// Act: add group order to Scalar
		AssertVerifyVrfProofFailsWhenProofIsCorrupted([](auto& vrfProof) {
			test::ScalarAddGroupOrder(vrfProof.Scalar.data());
		});
	}

	// endregion

	// region GenerateVrfProofHash

	TEST(TEST_CLASS, VrfSampleTestVectors_GenerateVrfProofHash) {
		// Arrange:
		auto testVectorsInput = SampleTestVectorsInput();
		auto testVectorsOutput = SampleTestVectorsOutput();

		// Sanity:
		ASSERT_EQ(testVectorsInput.size(), testVectorsOutput.size());

		auto i = 0u;
		for (const auto& input : testVectorsInput) {
			// Arrange:
			auto keyPair = KeyPair::FromString(input.SK);
			auto alpha = test::HexStringToVector(input.Alpha);
			auto message = "at index " + std::to_string(i);

			// - compute proof hash
			auto vrfProof = GenerateVrfProof(alpha, keyPair);
			auto proofHash = VerifyVrfProof(vrfProof, alpha, keyPair.publicKey());

			// Act: compute proof hash from gamma
			auto proofHashFromGamma = GenerateVrfProofHash(vrfProof.Gamma);

			// Assert:
			EXPECT_EQ(proofHash, proofHashFromGamma) << message;
			++i;
		}
	}

	// endregion
}}
