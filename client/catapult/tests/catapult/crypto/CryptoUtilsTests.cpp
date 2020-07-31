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

#include "catapult/crypto/CryptoUtils.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/utils/HexParser.h"
#include "catapult/utils/RandomGenerator.h"
#include "tests/TestHarness.h"
#include <donna/catapult.h>

namespace catapult { namespace crypto {

#define TEST_CLASS CryptoUtilsTests

	// region helpers

	namespace {
		void AssertCanonicalKey(const std::string& publicKeyString, bool expectedResult) {
			// Act:
			auto result = IsCanonicalKey(utils::ParseByteArray<Key>(publicKeyString));

			// Assert:
			EXPECT_EQ(expectedResult, result) << "for public key " << publicKeyString;
		}

		void AssertNeutralElement(const std::string& publicKeyString, bool expectedResult) {
			// Act:
			auto result = IsNeutralElement(utils::ParseByteArray<Key>(publicKeyString));

			// Assert:
			EXPECT_EQ(expectedResult, result) << "for public key " << publicKeyString;
		}

		void AssertElementInMainSubgroup(const std::string& publicKeyString, bool expectedResult) {
			// Arrange:
			auto publicKey = utils::ParseByteArray<Key>(publicKeyString);
			ge25519 A;
			EXPECT_TRUE(ge25519_unpack_negative_vartime(&A, publicKey.data()));

			// - negate A
			curve25519_neg(A.x, A.x);
			curve25519_neg(A.t, A.t);

			// Act:
			auto result = IsInMainSubgroup(A);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		void AssertUnpacking(const std::string& publicKeyString, bool expectedResult) {
			// Act:
			ge25519 A;
			auto result = UnpackNegative(A, utils::ParseByteArray<Key>(publicKeyString));

			// Assert:
			EXPECT_EQ(expectedResult, result) << "for public key " << publicKeyString;
		}

		void AssertUnpackingWithSubGroupCheck(const std::string& publicKeyString, bool expectedResult) {
			// Act:
			ge25519 A;
			auto result = UnpackNegativeAndCheckSubgroup(A, utils::ParseByteArray<Key>(publicKeyString));

			// Assert:
			EXPECT_EQ(expectedResult, result) << "for public key " << publicKeyString;
		}
	}

	// endregion

	// region IsCanonicalKey

	TEST(TEST_CLASS, IsCanonicalKeyReturnsTrueWhenYCoordinateIsSmallerThanFieldOrder) {
		AssertCanonicalKey("0000000000000000000000000000000000000000000000000000000000000000", true);
		AssertCanonicalKey("8CCB08D2A1A282AA8CC99902ECAF0F67A9F21CFFE28005CB27FCF129E963F99D", true);
		AssertCanonicalKey("8CCB08D2A1A282AA8CC9990FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7F", true);
		AssertCanonicalKey("ECFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7F", true);
	}

	TEST(TEST_CLASS, IsCanonicalKeyReturnsFalseWhenYCoordinateIsAtLeastAsLargeAsTheFieldOrder) {
		AssertCanonicalKey("EDFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7F", false);
		AssertCanonicalKey("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7F", false);
	}

	TEST(TEST_CLASS, IsCanonicalKeyIgnoresMostSignificantBit) {
		AssertCanonicalKey("ECFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", true);
		AssertCanonicalKey("EDFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", false);
	}

	// endregion

	// region IsNeutralElement

	TEST(TEST_CLASS, IsNeutralElementReturnsTrueForNeutralElement) {
		AssertNeutralElement("0100000000000000000000000000000000000000000000000000000000000000", true);
		AssertNeutralElement("0100000000000000000000000000000000000000000000000000000000000080", true);
	}

	TEST(TEST_CLASS, IsNeutralElementReturnsFalseForNonNeutralElements) {
		AssertNeutralElement("0000000000000000000000000000000000000000000000000000000000000000", false);
		AssertNeutralElement("0200000000000000000000000000000000000000000000000000000000000000", false);
		AssertNeutralElement("C8C6D604F4D7B56B57247E8686168EEBB2BF8AE40DA7B912143773A77555420E", false);
	}

	TEST(TEST_CLASS, IsNeutralElementIgnoresMostSignificantBit) {
		AssertNeutralElement("0100000000000000000000000000000000000000000000000000000000000080", true);
		AssertNeutralElement("5112BA143B78132AF616AF1A94E911EAD890FDB51B164A1B57C352ECD9CA1894", false);
	}

	// endregion

	// region IsInMainSubgroup

	TEST(TEST_CLASS, IsInMainSubgroupReturnsTrueWhenElementIsInMainSubgroup) {
		AssertElementInMainSubgroup("C8C6D604F4D7B56B57247E8686168EEBB2BF8AE40DA7B912143773A77555420E", true);
	}

	TEST(TEST_CLASS, IsInMainSubgroupReturnsFalseWhenElementIsNotInMainSubgroup) {
		AssertElementInMainSubgroup("0000000000000000000000000000000000000000000000000000000000000000", false);
		AssertElementInMainSubgroup("ECFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7F", false);
	}

	// endregion

	// region UnpackNegative

	TEST(TEST_CLASS, UnpackNegativeSucceedsForValidSubgroupElement) {
		AssertUnpacking("C8C6D604F4D7B56B57247E8686168EEBB2BF8AE40DA7B912143773A77555420E", true);
	}

	TEST(TEST_CLASS, UnpackNegativeSucceedsForElementOnCurveButNotInSubgroup) {
		// on curve but not in main subgroup
		AssertUnpacking("0000000000000000000000000000000000000000000000000000000000000000", true);
	}

	TEST(TEST_CLASS, UnpackNegativeFailsForValidGroupElementWithNonCanonicalAffineYCoordinate) {
		AssertUnpacking("EDFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7F", false);
	}

	TEST(TEST_CLASS, UnpackNegativeFailsForElementsNotOnCurve) {
		// not on curve
		AssertUnpacking("4F91BE9568552181E01968999EFC09BFEB77A736B8F3188160B7769D7B9B9F6E", false);
	}

	// endregion

	// region UnpackNegativeAndCheckSubgroup

	TEST(TEST_CLASS, UnpackNegativeAndCheckSubgroupSucceedsForValidSubgroupElement) {
		AssertUnpackingWithSubGroupCheck("C8C6D604F4D7B56B57247E8686168EEBB2BF8AE40DA7B912143773A77555420E", true);
	}

	TEST(TEST_CLASS, UnpackNegativeAndCheckSubgroupFailsForValidGroupElementWithNonCanonicalAffineYCoordinate) {
		AssertUnpackingWithSubGroupCheck("EDFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7F", false);
	}

	TEST(TEST_CLASS, UnpackNegativeAndCheckSubgroupFailsForElementsNotInSubgroup) {
		// not on curve
		AssertUnpackingWithSubGroupCheck("4F91BE9568552181E01968999EFC09BFEB77A736B8F3188160B7769D7B9B9F6E", false);

		// on curve but not in main subgroup
		AssertUnpackingWithSubGroupCheck("0000000000000000000000000000000000000000000000000000000000000000", false);
	}

	// endregion

	// region HashPrivateKey

	// the purpose of this test is to verify that result of HashPrivateKey matches sha512 hash
	TEST(TEST_CLASS, HashPrivateKey_PassesShaVector) {
		// Arrange:
		auto privateKeyString = std::string("8CCB08D2A1A282AA8CC99902ECAF0F67A9F21CFFE28005CB27FCF129E963F99D");
		auto privateKey = PrivateKey::FromString(privateKeyString);

		// Act:
		Hash512 hash;
		HashPrivateKey(privateKey, hash);

		// Assert:
		Hash512 expectedHash;
		Sha512(test::HexStringToVector(privateKeyString), expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}

	// endregion

	// region ExtractMultiplier

	namespace {
		// data taken from: http://git.gnupg.org/cgi-bin/gitweb.cgi?p=libgcrypt.git;a=blob;f=tests/t-ed25519.inp
		// secret keys
		std::vector<std::string> ExtractMultiplierSamplesInput() {
			return {
				"9D61B19DEFFD5A60BA844AF492EC2CC44449C5697B326919703BAC031CAE7F60",
				"4CCD089B28FF96DA9DB6C346EC114E0F5B8A319F35ABA624DA8CF6ED4FB8A6FB",
				"C5AA8DF43F9F837BEDB7442F31DCB7B166D38535076F094B85CE3A2E0B4458F7",
				"0D4A05B07352A5436E180356DA0AE6EFA0345FF7FB1572575772E8005ED978E9",
				"6DF9340C138CC188B5FE4464EBAA3F7FC206A2D55C3434707E74C9FC04E20EBB"
			};
		}

		// public keys
		std::vector<std::string> ExtractMultiplierSamplesOutput() {
			return {
				"D75A980182B10AB7D54BFED3C964073A0EE172F3DAA62325AF021A68F707511A",
				"3D4017C3E843895A92B70AA74D1B7EBC9C982CCF2EC4968CC0CD55F12AF4660C",
				"FC51CD8E6218A1A38DA47ED00230F0580816ED13BA3303AC5DEB911548908025",
				"E61A185BCEF2613A6C7CB79763CE945D3B245D76114DD440BCF5F2DC1AA57057",
				"C0DAC102C4533186E25DC43128472353EAABDB878B152AEB8E001F92D90233A7"
			};
		}

		Key ScalarMultiplyWithBasePoint(const bignum256modm& multiplier) {
			ge25519 ALIGN(16) A;
			Key publicKey;
			ge25519_scalarmult_base_niels(&A, ge25519_niels_base_multiples, multiplier);
			ge25519_pack(publicKey.data(), &A);
			return publicKey;
		}
	}

	TEST(TEST_CLASS, ExtractMultiplierSampleTestVectors) {
		// Arrange:
		auto testVectorsInput = ExtractMultiplierSamplesInput();
		auto testVectorsOutput = ExtractMultiplierSamplesOutput();

		// Sanity:
		ASSERT_EQ(testVectorsInput.size(), testVectorsOutput.size());

		auto i = 0u;
		for (const auto& input : testVectorsInput) {
			// Arrange:
			auto keyPair = KeyPair::FromPrivate(PrivateKey::FromString(input));

			ScalarMultiplier multiplier;
			ExtractMultiplier(keyPair.privateKey(), multiplier);

			// Assert: multiplication with base point should yield the correct public key
			bignum256modm expandedMultiplier;
			expand256_modm(expandedMultiplier, multiplier, sizeof(ScalarMultiplier));
			auto publicKey = ScalarMultiplyWithBasePoint(expandedMultiplier);
			EXPECT_EQ(utils::ParseByteArray<Key>(testVectorsOutput[i]), publicKey) << "at index " << i;

			++i;
		}
	}

	// endregion

	// region GenerateNonce

	namespace {
		struct GenerateNonceTestVectorsInput {
			std::string SecretKey;
			std::string Message;
		};

		// data taken from: http://git.gnupg.org/cgi-bin/gitweb.cgi?p=libgcrypt.git;a=blob;f=tests/t-ed25519.inp
		std::vector<GenerateNonceTestVectorsInput> GenerateNonceSamplesInput() {
			return {
				{
					"9D61B19DEFFD5A60BA844AF492EC2CC44449C5697B326919703BAC031CAE7F60",
					""
				},
				{
					"4CCD089B28FF96DA9DB6C346EC114E0F5B8A319F35ABA624DA8CF6ED4FB8A6FB",
					"72"
				},
				{
					"C5AA8DF43F9F837BEDB7442F31DCB7B166D38535076F094B85CE3A2E0B4458F7",
					"Af82"
				},
				{
					"B322D46577A2A991A4D1698287832A39C487EF776B4BFF037A05C7F1812BDEEC",
					"19F1BF5DCF1750C611F1C4A2865200504D82298EDD72671F62A7B1471AC3D4A30F7DE9E5DA4108C52A4CE70A3E114A52A3B3C5"
				}
			};
		}

		// output is the R part of the signature
		std::vector<std::string> GenerateNonceSamplesOutput() {
			return {
				"E5564300C360AC729086E2CC806E828A84877F1EB8E5D974D873E06522490155",
				"92A009A9F0D4CAB8720E820B5F642540A2B27B5416503F8FB3762223EBDB69DA",
				"6291D657DEEC24024827E69C3ABE01A30CE548A284743A445E3680D7DB5AC3AC",
				"C7B55137317CA21E33489FF6A9BFAB97C855DC6F85684A70A9125A261B56D5E6"
			};
		}
	}

	TEST(TEST_CLASS, GenerateNonceSampleTestVectors) {
		// Arrange:
		auto testVectorsInput = GenerateNonceSamplesInput();
		auto testVectorsOutput = GenerateNonceSamplesOutput();

		// Sanity:
		ASSERT_EQ(testVectorsInput.size(), testVectorsOutput.size());

		auto i = 0u;
		for (const auto& input : testVectorsInput) {
			// Arrange:
			auto privateKey = PrivateKey::FromString(input.SecretKey);

			// Act:
			bignum256modm nonce;
			GenerateNonce(privateKey, { test::HexStringToVector(input.Message) }, nonce);

			// Assert: nonce * base point should yield the R part of the signature
			auto publicKey = ScalarMultiplyWithBasePoint(nonce);
			EXPECT_EQ(utils::ParseByteArray<Key>(testVectorsOutput[i]), publicKey) << "at index " << i;

			++i;
		}
	}

	TEST(TEST_CLASS, GenerateNonceCanHandleMultipleBuffers) {
		// Arrange: message is from sample at index 3
		auto privateKey = PrivateKey::FromString("B322D46577A2A991A4D1698287832A39C487EF776B4BFF037A05C7F1812BDEEC");
		auto buffer1 = test::HexStringToVector("19F1BF5DCF1750C611F1");
		auto buffer2 = test::HexStringToVector("C4A2865200504D82298EDD72671F62A7B1471AC3D4A30F7DE9E5DA41");
		auto buffer3 = test::HexStringToVector("08C52A4CE70A3E114A52A3B3C5");

		bignum256modm nonce;
		GenerateNonce(privateKey, { buffer1, buffer2, buffer3 }, nonce);

		// Assert: nonce * base point should yield the R part of the signature
		auto publicKey = ScalarMultiplyWithBasePoint(nonce);
		EXPECT_EQ(utils::ParseByteArray<Key>(GenerateNonceSamplesOutput()[3]), publicKey);
	}

	// endregion

	// region ScalarMult

	namespace {
		struct ScalarMultTestVectorsInput {
			std::string Multiplier;
			std::string PublicKey;
		};

		std::vector<ScalarMultTestVectorsInput> ScalarMultSamplesInput() {
			return {
				{
					"4F91BE9568552181E01968999EFC09BFEB77A736B8F3188160B7769D7B9B9F6E",
					"C8C6D604F4D7B56B57247E8686168EEBB2BF8AE40DA7B912143773A77555420E"
				},
				{
					"1CE3047B251E0B3A4E7975D220A302FE2F19D84936B2E77469BC8B43D7C91F5D",
					"7013667A4BC05C53721A9AEBA1E7429057729DA79F30985C65C054BDE19E7070"
				},
				{
					"ABF74755DC9DE65EE40F1646788D1F02EFC8363B6CF16234C664EACFAAC3EF20",
					"77554A21F62BCB6C68F3CEA5B2D62C7146706BA369B99BE80FCABFB799C807F9"
				}
			};
		}

		std::vector<std::string> ScalarMultSamplesOutput() {
			return {
				"0E4FE00C66999D8D4E8855AABA89A11ED887BECA44580612EF99246E84C56F97",
				"FA5CE9E4964C02E7341129CA5E234D3E5D7B851C2718FBDD135D839EC0442C1B",
				"ECAB44A0819F8ACE7EBB75B9CC9D8D5303FB89940024A67F559D8FCDDF21866F"
			};
		}
	}

	TEST(TEST_CLASS, ScalarMultSampleTestVectors) {
		// Arrange:
		auto testVectorsInput = ScalarMultSamplesInput();
		auto testVectorsOutput = ScalarMultSamplesOutput();

		// Sanity:
		ASSERT_EQ(testVectorsInput.size(), testVectorsOutput.size());

		auto i = 0u;
		for (const auto& input : testVectorsInput) {
			// Arrange:
			auto publicKey = utils::ParseByteArray<Key>(input.PublicKey);
			auto multiplier = utils::ParseByteArray<Key>(input.Multiplier);
			ScalarMultiplier encodedMultiplier;
			std::memcpy(encodedMultiplier, multiplier.data(), 32);

			// Act:
			Key result;
			bool success = ScalarMult(encodedMultiplier, publicKey, result);

			// Assert:
			EXPECT_TRUE(success);
			EXPECT_EQ(utils::ParseByteArray<Key>(testVectorsOutput[i]), result) << "at index " << i;

			++i;
		}
	}

	TEST(TEST_CLASS, ScalarMultReturnsFalseWhenPublicKeyIsNotOnTheCurve) {
		// Arrange:
		auto publicKey = utils::ParseByteArray<Key>("4F91BE9568552181E01968999EFC09BFEB77A736B8F3188160B7769D7B9B9F6E");
		uint8_t multiplier[32];
		multiplier[0] = 1;

		// Sanity:
		ge25519 A;
		EXPECT_FALSE(ge25519_unpack_negative_vartime(&A, publicKey.data()));

		// Act:
		Key result;
		bool success = ScalarMult(multiplier, publicKey, result);

		// Assert:
		EXPECT_FALSE(success);
		EXPECT_EQ(Key(), result);
	}

	TEST(TEST_CLASS, ScalarMultReturnsFalseWhenResultIsTheNeutralElement) {
		// Arrange: multiply neutral element with some value
		auto publicKey = utils::ParseByteArray<Key>("0100000000000000000000000000000000000000000000000000000000000000");
		uint8_t multiplier[32];
		multiplier[14] = 123;

		// Act:
		Key result;
		bool success = ScalarMult(multiplier, publicKey, result);

		// Assert:
		EXPECT_FALSE(success);
	}

	// endregion
}}
