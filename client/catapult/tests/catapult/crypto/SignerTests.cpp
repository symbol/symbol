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

#include "catapult/crypto/Signer.h"
#include "tests/TestHarness.h"
#include <numeric>

namespace catapult { namespace crypto {

#define TEST_CLASS SignerTests

	namespace {
		template<typename TArray>
		Signature SignPayload(const KeyPair& keyPair, const TArray& payload) {
			Signature signature{};
			EXPECT_NO_THROW(Sign(keyPair, payload, signature));
			return signature;
		}

		const char* Default_Key_String = "CBD84EF8F5F38A25C01308785EA99627DE897D151AFDFCDA7AB07EFD8ED98534";
		KeyPair GetDefaultKeyPair() {
			return KeyPair::FromString(Default_Key_String);
		}

		KeyPair GetAlteredKeyPair() {
			return KeyPair::FromString("CBD84EF8F5F38A25C01308785EA99627DE897D151AFDFCDA7AB07EFD8ED98535");
		}
	}

	TEST(TEST_CLASS, SignFillsTheSignature) {
		// Arrange:
		auto payload = test::GenerateRandomArray<100>();

		// Act:
		Signature signature;
		std::iota(signature.begin(), signature.end(), static_cast<uint8_t>(0));
		Sign(GetDefaultKeyPair(), payload, signature);

		// Assert: the signature got overwritten in call to Sign() above
		Signature invalid;
		std::iota(invalid.begin(), invalid.end(), static_cast<uint8_t>(0));
		EXPECT_NE(invalid, signature);
	}

	TEST(TEST_CLASS, SignaturesGeneratedForSameDataBySameKeyPairsAreEqual) {
		// Arrange:
		auto keyPair1 = KeyPair::FromString(Default_Key_String);
		auto keyPair2 = KeyPair::FromString(Default_Key_String);
		auto payload = test::GenerateRandomArray<100>();

		// Act:

		auto signature1 = SignPayload(keyPair1, payload);
		auto signature2 = SignPayload(keyPair2, payload);

		// Assert:
		EXPECT_EQ(signature1, signature2);
	}

	TEST(TEST_CLASS, SignaturesGeneratedForSameDataByDifferentKeyPairsAreDifferent) {
		// Arrange:
		auto payload = test::GenerateRandomArray<100>();

		// Act:
		auto signature1 = SignPayload(GetDefaultKeyPair(), payload);
		auto signature2 = SignPayload(GetAlteredKeyPair(), payload);

		// Assert:
		EXPECT_NE(signature1, signature2);
	}

	TEST(TEST_CLASS, SignedDataCanBeVerified) {
		// Arrange:
		auto payload = test::GenerateRandomArray<100>();
		auto signature = SignPayload(GetDefaultKeyPair(), payload);

		// Act:
		bool isVerified = Verify(GetDefaultKeyPair().publicKey(), payload, signature);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	TEST(TEST_CLASS, SignedDataCannotBeVerifiedWithDifferentKeyPair) {
		// Arrange:
		auto payload = test::GenerateRandomArray<100>();
		auto signature = SignPayload(GetDefaultKeyPair(), payload);

		// Act:
		bool isVerified = Verify(GetAlteredKeyPair().publicKey(), payload, signature);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	namespace {
		void AssertSignatureChangeInvalidatesSignature(size_t position) {
			// Arrange:
			auto keyPair = GetDefaultKeyPair();
			auto payload = test::GenerateRandomArray<100>();

			auto signature = SignPayload(keyPair, payload);
			signature[position] ^= 0xFF;

			// Act:
			bool isVerified = Verify(keyPair.publicKey(), payload, signature);

			// Assert:
			EXPECT_FALSE(isVerified);
		}
	}

	TEST(TEST_CLASS, SignatureDoesNotVerifyWhenRPartOfSignatureIsModified) {
		// Assert:
		for (auto i = 0u; i < Signature_Size / 2; ++i)
			AssertSignatureChangeInvalidatesSignature(i);
	}

	TEST(TEST_CLASS, SignatureDoesNotVerifyWhenSPartOfSignatureIsModified) {
		// Assert:
		for (auto i = Signature_Size / 2; i < Signature_Size; ++i)
			AssertSignatureChangeInvalidatesSignature(i);
	}

	TEST(TEST_CLASS, SignatureDoesNotVerifyWhenPayloadIsModified) {
		// Arrange:
		auto keyPair = GetDefaultKeyPair();
		auto payload = test::GenerateRandomArray<100>();
		for (auto i = 0u; i < payload.size(); ++i) {
			auto signature = SignPayload(keyPair, payload);
			payload[i] ^= 0xFF;

			// Act:
			bool isVerified = Verify(keyPair.publicKey(), payload, signature);

			// Assert:
			EXPECT_FALSE(isVerified);
		}
	}

	TEST(TEST_CLASS, PublicKeyNotOnACurveCausesVerifyToFail) {
		// Arrange:
		auto hackedKeyPair = GetDefaultKeyPair();
		auto payload = test::GenerateRandomArray<100>();

		// hack the key, to an invalid one (not on a curve)
		auto& hackPublic = const_cast<Key&>(hackedKeyPair.publicKey());
		std::fill(hackPublic.begin(), hackPublic.end(), static_cast<uint8_t>(0));
		hackPublic[hackPublic.size() - 1] = 0x01;

		auto signature = SignPayload(hackedKeyPair, payload);

		// Act:
		bool isVerified = Verify(hackedKeyPair.publicKey(), payload, signature);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	TEST(TEST_CLASS, VerificationFailsWhenPublicKeyDoesNotCorrespondToPrivateKey) {
		// Arrange:
		auto hackedKeyPair = GetDefaultKeyPair();
		auto payload = test::GenerateRandomArray<100>();

		// hack the key, to an invalid one
		auto& hackPublic = const_cast<Key&>(hackedKeyPair.publicKey());
		std::transform(hackPublic.begin(), hackPublic.end(), hackPublic.begin(), [](uint8_t x) {
			return static_cast<uint8_t>(x ^ 0xFF);
		});

		auto signature = SignPayload(hackedKeyPair, payload);

		// Act:
		bool isVerified = Verify(hackedKeyPair.publicKey(), payload, signature);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	TEST(TEST_CLASS, VerifyRejectsZeroPublicKey) {
		// Arrange:
		auto hackedKeyPair = GetDefaultKeyPair();
		auto payload = test::GenerateRandomArray<100>();

		// hack the key, to an invalid one
		auto& hackPublic = const_cast<Key&>(hackedKeyPair.publicKey());
		std::fill(hackPublic.begin(), hackPublic.end(), static_cast<uint8_t>(0));

		auto signature = SignPayload(hackedKeyPair, payload);

		// Act:
		// keep in mind, there's no good way to make this test, as right now, we have
		// no way (and I don't think we need one), to check why verify failed
		bool isVerified = Verify(hackedKeyPair.publicKey(), payload, signature);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	namespace {
		void ScalarAddGroupOrder(uint8_t* scalar) {
			// 2^252 + 27742317777372353535851937790883648493, little endian.
			const auto Group_Order = test::ToArray<Signature_Size / 2>("EDD3F55C1A631258D69CF7A2DEF9DE1400000000000000000000000000000010");
			uint8_t r = 0;
			for (auto i = 0u; i < Signature_Size / 2; ++i) {
				auto t = static_cast<uint16_t>(scalar[i]) + static_cast<uint16_t>(Group_Order[i]);
				scalar[i] += Group_Order[i] + r;
				r = static_cast<uint8_t>(t >> 8);
			}
		}
	}

	TEST(TEST_CLASS, CannotVerifyNonCanonicalSignature) {
		// Arrange:
		std::array<uint8_t, 10> payload{ { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 } };

		auto keyPair = GetDefaultKeyPair();
		auto canonicalSignature = SignPayload(keyPair, payload);
		// this is signature with group order added to 'encodedS' part of signature
		auto nonCanonicalSignature = canonicalSignature;
		ScalarAddGroupOrder(nonCanonicalSignature.data() + Signature_Size / 2);

		// Act:
		bool isCanonicalVerified = Verify(keyPair.publicKey(), payload, canonicalSignature);
		bool isNonCanonicalVerified = Verify(keyPair.publicKey(), payload, nonCanonicalSignature);

		// Assert:
		EXPECT_TRUE(isCanonicalVerified);
		EXPECT_FALSE(isNonCanonicalVerified);
	}

	namespace {
		struct TestVectorsInput {
			std::vector<std::string> InputData;
			std::vector<std::string> PrivateKeys;
			std::vector<std::string> ExpectedPublicKeys;
			std::vector<std::string> ExpectedSignatures;
		};

		TestVectorsInput GetTestVectorsInput() {
			TestVectorsInput input;
			input.InputData = {
				"8CE03CD60514233B86789729102EA09E867FC6D964DEA8C2018EF7D0A2E0E24BF7E348E917116690B9",
				"E4A92208A6FC52282B620699191EE6FB9CF04DAF48B48FD542C5E43DAA9897763A199AAA4B6F10546109F47AC3564FADE0",
				"13ED795344C4448A3B256F23665336645A853C5C44DBFF6DB1B9224B5303B6447FBF8240A2249C55",
				"A2704638434E9F7340F22D08019C4C8E3DBEE0DF8DD4454A1D70844DE11694F4C8CA67FDCB08FED0CEC9ABB2112B5E5F89",
				"D2488E854DBCDFDB2C9D16C8C0B2FDBC0ABB6BAC991BFE2B14D359A6BC99D66C00FD60D731AE06D0",
			};

#ifdef SIGNATURE_SCHEME_NIS1
			// reversed private keys
			input.PrivateKeys = {
				"ABF4CF55A2B3F742D7543D9CC17F50447B969E6E06F5EA9195D428AB12B7318D",
				"6AA6DAD25D3ACB3385D5643293133936CDDDD7F7E11818771DB1FF2F9D3F9215",
				"8E32BC030A4C53DE782EC75BA7D5E25E64A2A072A56E5170B77A4924EF3C32A9",
				"C83CE30FCB5B81A51BA58FF827CCBC0142D61C13E2ED39E78E876605DA16D8D7",
				"2DA2A0AAE0F37235957B51D15843EDDE348A559692D8FA87B94848459899FC27"
			};
			input.ExpectedPublicKeys = {
				"8A558C728C21C126181E5E654B404A45B4F0137CE88177435A69978CC6BEC1F4",
				"BBC8CBB43DDA3ECF70A555981A351A064493F09658FFFE884C6FAB2A69C845C6",
				"72D0E65F1EDE79C4AF0BA7EC14204E10F0F7EA09F2BC43259CD60EA8C3A087E2",
				"3EC8923F9EA5EA14F8AAA7E7C2784653ED8C7DE44E352EF9FC1DEE81FC3FA1A3",
				"D73D0B14A9754EEC825FCB25EF1CFA9AE3B1370074EDA53FC64C22334A26C254",
			};
			input.ExpectedSignatures = {
				"D9CEC0CC0E3465FAB229F8E1D6DB68AB9CC99A18CB0435F70DEB6100948576CD5C0AA1FEB550BDD8693EF81EB10A556A622DB1F9301986827B96716A7134230C",
				"98BCA58B075D1748F1C3A7AE18F9341BC18E90D1BEB8499E8A654C65D8A0B4FBD2E084661088D1E5069187A2811996AE31F59463668EF0F8CB0AC46A726E7902",
				"EF257D6E73706BB04878875C58AA385385BF439F7040EA8297F7798A0EA30C1C5EFF5DDC05443F801849C68E98111AE65D088E726D1D9B7EECA2EB93B677860C",
				"0C684E71B35FED4D92B222FC60561DB34E0D8AFE44BDD958AAF4EE965911BEF5991236F3E1BCED59FC44030693BCAC37F34D29E5AE946669DC326E706E81B804",
				"6F17F7B21EF9D6907A7AB104559F77D5A2532B557D95EDFFD6D88C073D87AC00FC838FC0D05282A0280368092A4BD67E95C20F3E14580BE28D8B351968C65E03"
			};
#else
			input.PrivateKeys = {
				"8D31B712AB28D49591EAF5066E9E967B44507FC19C3D54D742F7B3A255CFF4AB",
				"15923F9D2FFFB11D771818E1F7D7DDCD363913933264D58533CB3A5DD2DAA66A",
				"A9323CEF24497AB770516EA572A0A2645EE2D5A75BC72E78DE534C0A03BC328E",
				"D7D816DA0566878EE739EDE2131CD64201BCCC27F88FA51BA5815BCB0FE33CC8",
				"27FC9998454848B987FAD89296558A34DEED4358D1517B953572F3E0AAA0A22D"
			};
			input.ExpectedPublicKeys = {
				"53C659B47C176A70EB228DE5C0A0FF391282C96640C2A42CD5BBD0982176AB1B",
				"3FE4A1AA148F5E76891CE924F5DC05627A87047B2B4AD9242C09C0ECED9B2338",
				"F398C0A2BDACDBD7037D2F686727201641BBF87EF458F632AE2A04B4E8F57994",
				"6A283A241A8D8203B3A1E918B1E6F0A3E14E75E16D4CFFA45AE4EF89E38ED6B5",
				"4DC62B38215826438DE2369743C6BBE6D13428405025DFEFF2857B9A9BC9D821"
			};
			input.ExpectedSignatures = {
				"C9B1342EAB27E906567586803DA265CC15CCACA411E0AEF44508595ACBC47600D02527F2EED9AB3F28C856D27E30C3808AF7F22F5F243DE698182D373A9ADE03",
				"0755E437ED4C8DD66F1EC29F581F6906AB1E98704ECA94B428A25937DF00EC64796F08E5FEF30C6F6C57E4A5FB4C811D617FA661EB6958D55DAE66DDED205501",
				"15D6585A2A456E90E89E8774E9D12FE01A6ACFE09936EE41271AA1FBE0551264A9FF9329CB6FEE6AE034238C8A91522A6258361D48C5E70A41C1F1C51F55330D",
				"F6FB0D8448FEC0605CF74CFFCC7B7AE8D31D403BCA26F7BD21CB4AC87B00769E9CC7465A601ED28CDF08920C73C583E69D621BA2E45266B86B5FCF8165CBE309",
				"E88D8C32FE165D34B775F70657B96D8229FFA9C783E61198A6F3CCB92F487982D08F8B16AB9157E2EFC3B78F126088F585E26055741A9F25127AC13E883C9A05"
			};
#endif

			// Sanity:
			EXPECT_EQ(input.InputData.size(), input.PrivateKeys.size());
			EXPECT_EQ(input.InputData.size(), input.ExpectedPublicKeys.size());
			EXPECT_EQ(input.InputData.size(), input.ExpectedSignatures.size());
			return input;
		}
	}

	TEST(TEST_CLASS, SignPassesTestVectors) {
		// Arrange:
		auto input = GetTestVectorsInput();

		// Act / Assert:
		for (auto i = 0u; i < input.InputData.size(); ++i) {
			// Act:
			auto keyPair = KeyPair::FromString(input.PrivateKeys[i]);
			auto signature = SignPayload(keyPair, test::ToVector(input.InputData[i]));

			// Assert:
			auto message = "test vector at " + std::to_string(i);
			EXPECT_EQ(input.ExpectedPublicKeys[i], test::ToString(keyPair.publicKey())) << message;
			EXPECT_EQ(input.ExpectedSignatures[i], test::ToString(signature)) << message;
		}
	}

	TEST(TEST_CLASS, VerifyPassesTestVectors) {
		// Arrange:
		auto input = GetTestVectorsInput();

		// Act / Assert:
		for (auto i = 0u; i < input.InputData.size(); ++i) {
			// Act:
			auto keyPair = KeyPair::FromString(input.PrivateKeys[i]);
			auto payload = test::ToVector(input.InputData[i]);
			auto signature = SignPayload(keyPair, payload);
			auto isVerified = Verify(keyPair.publicKey(), payload, signature);

			// Assert:
			auto message = "test vector at " + std::to_string(i);
			EXPECT_TRUE(isVerified) << message;
		}
	}

	TEST(TEST_CLASS, SignatureForConsecutiveDataMatchesSignatureForChunkedData) {
		// Arrange:
		auto payload = test::GenerateRandomVector(123);
		auto properSignature = SignPayload(GetDefaultKeyPair(), payload);

		// Act:
		{
			Signature result;
			auto partSize = payload.size() / 2;
			ASSERT_NO_THROW(Sign(GetDefaultKeyPair(), {
				{ payload.data(), partSize },
				{ payload.data() + partSize, payload.size() - partSize }
			}, result));
			EXPECT_EQ(properSignature, result);
		}

		{
			Signature result;
			auto partSize = payload.size() / 3;
			ASSERT_NO_THROW(Sign(GetDefaultKeyPair(), {
				{ payload.data(), partSize },
				{ payload.data() + partSize, partSize },
				{ payload.data() + 2 * partSize, payload.size() - 2 * partSize }
			}, result));
			EXPECT_EQ(properSignature, result);
		}

		{
			Signature result;
			auto partSize = payload.size() / 4;
			ASSERT_NO_THROW(Sign(GetDefaultKeyPair(), {
				{ payload.data(), partSize },
				{ payload.data() + partSize, partSize },
				{ payload.data() + 2 * partSize, partSize },
				{ payload.data() + 3 * partSize, payload.size() - 3 * partSize }
			}, result));
			EXPECT_EQ(properSignature, result);
		}
	}
}}
