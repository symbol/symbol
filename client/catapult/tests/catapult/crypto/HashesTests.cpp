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

#include "catapult/crypto/Hashes.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS HashesTests

	namespace {
		std::string AsciiToHexString(const std::string& str) {
			const auto* pAsciiData = reinterpret_cast<const uint8_t*>(str.data());

			std::stringstream out;
			out << utils::HexFormat(pAsciiData, pAsciiData + str.size());
			return out.str();
		}

		// region traits

		struct Ripemd160_Traits {
			using HashType = Hash160;

			static constexpr auto HashFunc = Ripemd160;

			static std::string EmptyStringHash() {
				return "9C1185A5C5E9FC54612808977EE8F548B2258D31";
			}

			// data from: https://homes.esat.kuleuven.be/~bosselae/ripemd160.html
			static std::vector<std::string> SampleTestVectorsInput() {
				return {
					AsciiToHexString("a"),
					AsciiToHexString("abc"),
					AsciiToHexString("message digest"),
					AsciiToHexString("abcdefghijklmnopqrstuvwxyz"),
					AsciiToHexString("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
					AsciiToHexString("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"),
					AsciiToHexString("12345678901234567890123456789012345678901234567890123456789012345678901234567890")
				};
			}

			static std::vector<std::string> SampleTestVectorsOutput() {
				return {
					"0BDC9D2D256B3EE9DAAE347BE6F4DC835A467FFE",
					"8EB208F7E05D987A9B044A8E98C6B087F15A0BFC",
					"5D0689EF49D2FAE572B881B123A85FFA21595F36",
					"F71C27109C692C1B56BBDCEB5B9D2865B3708DBC",
					"12A053384A9C0C88E405A06C27DCF49ADA62EB2B",
					"B0E20B6E3116640286ED3A87A5713079B21F5189",
					"9B752E45573D4B39F4DBD3323CAB82BF63326BFB"
				};
			}

			static std::string MillionTimesATestVector() {
				return "52783243C1697BDBE16D37F97F68F08325DC1528";
			}
		};

		struct Bitcoin160_Traits {
			using HashType = Hash160;

			static constexpr auto HashFunc = Bitcoin160;

			static std::string EmptyStringHash() {
				return "B472A266D0BD89C13706A4132CCFB16F7C3B9FCB";
			}

			// https://github.com/libbitcoin/libbitcoin-system/blob/master/test/chain/script.hpp
			static std::vector<std::string> SampleTestVectorsInput() {
				return {
					AsciiToHexString("a"),
					AsciiToHexString("abcdefghijklmnopqrstuvwxyz"),
					// https://en.bitcoin.it/wiki/Transaction
					"04D4FB35C2CDB822644F1057E9BD07E3D3B0A36702662327EF4EB799EB219856"
						"D0FD884FCE43082B73424A3293837C5F94A478F7BC4EC4DA82BFB7E0B43FB218CC",
					// http://learnmeabitcoin.com/glossary/public-key-hash160
					"02B4632D08485FF1DF2DB55B9DAFD23347D1C47A457072A1E87BE26896549A8737",
					// first bitcoin transaction address https://www.BLOCKCHAIN.com/btc/address/12cbQLTFMXRnSzktFkuoG3eHoMeFtpTu3S
					"0411DB93E1DCDB8A016B49840F8C53BC1EB68A382E97B1482ECAD7B148A6909A"
						"5CB2E0EADDFB84CCF9744464F82E160BFA9B8B64F9D4C03F999B8643F656B412A3"
				};
			}

			static std::vector<std::string> SampleTestVectorsOutput() {
				return {
					"994355199E516FF76C4FA4AAB39337B9D84CF12B",
					"C286A1AF0947F58D1AD787385B1C2C4A976F9E71",
					"404371705FA9BD789A2FCD52D2C580B65D35549D",
					"93CE48570B55C42C2AF816AEABA06CFEE1224FAE",
					"11B366EDFC0A8B66FEEBAE5C2E25A7B6A5D1CF31"
				};
			}
		};

		struct Sha256_Traits {
			using HashType = Hash256;

			static constexpr auto HashFunc = Sha256;

			static std::string EmptyStringHash() {
				return "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855";
			}

			// data from: https://opensource.apple.com/source/sudo/sudo-83/sudo/plugins/sudoers/regress/parser/check_digest.out.ok
			static std::vector<std::string> SampleTestVectorsInput() {
				return {
					AsciiToHexString("a"),
					AsciiToHexString("abc"),
					AsciiToHexString("message digest"),
					AsciiToHexString("abcdefghijklmnopqrstuvwxyz"),
					AsciiToHexString("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
					AsciiToHexString("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"),
					AsciiToHexString("12345678901234567890123456789012345678901234567890123456789012345678901234567890")
				};
			}

			static std::vector<std::string> SampleTestVectorsOutput() {
				return {
					"CA978112CA1BBDCAFAC231B39A23DC4DA786EFF8147C4E72B9807785AFEE48BB",
					"BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD",
					"F7846F55CF23E14EEBEAB5B4E1550CAD5B509E3348FBC4EFA3A1413D393CB650",
					"71C480DF93D6AE2F1EFAD1447C66C9525E316218CF51FC8D9ED832F2DAF18B73",
					"248D6A61D20638B8E5C026930C3E6039A33CE45964FF2167F6ECEDD419DB06C1",
					"DB4BFCBD4DA0CD85A60C3C37D3FBD8805C77F15FC6B1FDFE614EE0A7C8FDB4C0",
					"F371BC4A311F2B009EEF952DD83CA80E2B60026C8E935592D0F9C308453C813E"
				};
			}

			static std::string MillionTimesATestVector() {
				return "CDC76E5C9914FB9281A1C7E284D73E67F1809A48A497200E046D39CCC7112CD0";
			}
		};

		struct Sha256Double_Traits {
			using HashType = Hash256;

			static constexpr auto HashFunc = Sha256Double;

			static std::string EmptyStringHash() {
				return "5DF6E0E2761359D30A8275058E299FCC0381534545F55CF43E41983F5D4C9456";
			}

			// data from: https://www.dlitz.net/crypto/shad256-test-vectors/
			static std::vector<std::string> SampleTestVectorsInput() {
				return {
					"616263",
					"DE188941A3375D3A8A061E67576E926D",
					"DE188941A3375D3A8A061E67576E926DC71A7FA3F0CCEB97452B4D3227965F9E",
					"DE188941A3375D3A8A061E67576E926DC71A7FA3F0CCEB97452B4D3227965F9EA8CC75076D9FB9C5417AA5CB30FC22198B34982DBB629E"
				};
			}

			static std::vector<std::string> SampleTestVectorsOutput() {
				return {
					"4F8B42C22DD3729B519BA6F68D2DA7CC5B2D606D05DAED5AD5128CC03E6C6358",
					"2182D3FE9882FD597D25DAF6A85E3A574E5A9861DBC75C13CE3F47FE98572246",
					"14D881FE278E33F7165CB52B714140C96306A93ED02BCF4D1B6F650DE67A9E5F",
					"3B4666A5643DE038930566A5930713E65D72888D3F51E20F9545329620485B03"
				};
			}

			static std::string MillionTimesATestVector() {
				return "80D1189477563E1B5206B2749F1AFE4807E5705E8BD77887A60187A712156688";
			}
		};

		struct Sha512_Traits {
			using HashBuilder = Sha512_Builder;
			using HashType = HashBuilder::OutputType;

			static constexpr auto HashFunc = Sha512;

			static std::string EmptyStringHash() {
				return
						"CF83E1357EEFB8BDF1542850D66D8007D620E4050B5715DC83F4A921D36CE9CE"
						"47D0D13C5D85F2B0FF8318D2877EEC2F63B931BD47417A81A538327AF927DA3E";
			}

			// data from: https://opensource.apple.com/source/sudo/sudo-83/sudo/plugins/sudoers/regress/parser/check_digest.out.ok
			static std::vector<std::string> SampleTestVectorsInput() {
				return {
					AsciiToHexString("a"),
					AsciiToHexString("abc"),
					AsciiToHexString("message digest"),
					AsciiToHexString("abcdefghijklmnopqrstuvwxyz"),
					AsciiToHexString("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
					AsciiToHexString("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"),
					AsciiToHexString("12345678901234567890123456789012345678901234567890123456789012345678901234567890")
				};
			}

			static std::vector<std::string> SampleTestVectorsOutput() {
				return {
					"1F40FC92DA241694750979EE6CF582F2D5D7D28E18335DE05ABC54D0560E0F53"
					"02860C652BF08D560252AA5E74210546F369FBBBCE8C12CFC7957B2652FE9A75",
					"DDAF35A193617ABACC417349AE20413112E6FA4E89A97EA20A9EEEE64B55D39A"
					"2192992A274FC1A836BA3C23A3FEEBBD454D4423643CE80E2A9AC94FA54CA49F",
					"107DBF389D9E9F71A3A95F6C055B9251BC5268C2BE16D6C13492EA45B0199F33"
					"09E16455AB1E96118E8A905D5597B72038DDB372A89826046DE66687BB420E7C",
					"4DBFF86CC2CA1BAE1E16468A05CB9881C97F1753BCE3619034898FAA1AABE429"
					"955A1BF8EC483D7421FE3C1646613A59ED5441FB0F321389F77F48A879C7B1F1",
					"204A8FC6DDA82F0A0CED7BEB8E08A41657C16EF468B228A8279BE331A703C335"
					"96FD15C13B1B07F9AA1D3BEA57789CA031AD85C7A71DD70354EC631238CA3445",
					"1E07BE23C26A86EA37EA810C8EC7809352515A970E9253C26F536CFC7A9996C4"
					"5C8370583E0A78FA4A90041D71A4CEAB7423F19C71B9D5A3E01249F0BEBD5894",
					"72EC1EF1124A45B047E8B7C75A932195135BB61DE24EC0D1914042246E0AEC3A"
					"2354E093D76F3048B456764346900CB130D2A4FD5DD16ABB5E30BCB850DEE843"
				};
			}

			static std::string MillionTimesATestVector() {
				return
						"E718483D0CE769644E2E42C7BC15B4638E1F98B13B2044285632A803AFA973EB"
						"DE0FF244877EA60A4CB0432CE577C31BEB009C5C2C49AA2E4EADB217AD8CC09B";
			}
		};

		struct Keccak_Base {
			// data from: http://mumble.net/~campbell/hg/sha3/kat/ShortMsgKAT_SHA3-256.txt
			static std::vector<std::string> SampleTestVectorsInput() {
				return {
					"CC",
					"41FB",
					"1F877C",
					"C1ECFDFC",
					"9F2FCC7C90DE090D6B87CD7E9718C1EA6CB21118FC2D5DE9F97E5DB6AC1E9C10"
				};
			}
		};

		struct Sha3_256_Traits : public Keccak_Base {
			using HashBuilder = Sha3_256_Builder;
			using HashType = HashBuilder::OutputType;

			static constexpr auto HashFunc = Sha3_256;

			static std::string EmptyStringHash() {
				return "A7FFC6F8BF1ED76651C14756A061D662F580FF4DE43B49FA82D80A4B80F8434A";
			}

			// data from: http://mumble.net/~campbell/hg/sha3/kat/ShortMsgKAT_SHA3-256.txt
			static std::vector<std::string> SampleTestVectorsOutput() {
				return {
					"677035391CD3701293D385F037BA32796252BB7CE180B00B582DD9B20AAAD7F0",
					"39F31B6E653DFCD9CAED2602FD87F61B6254F581312FB6EEEC4D7148FA2E72AA",
					"BC22345E4BD3F792A341CF18AC0789F1C9C966712A501B19D1B6632CCD408EC5",
					"C5859BE82560CC8789133F7C834A6EE628E351E504E601E8059A0667FF62C124",
					"2F1A5F7159E34EA19CDDC70EBF9B81F1A66DB40615D7EAD3CC1F1B954D82A3AF"
				};
			}

			// data from: https://www.di-mgt.com.au/sha_testvectors.html
			static std::string MillionTimesATestVector() {
				return "5C8875AE474A3634BA4FD55EC85BFFD661F32ACA75C6D699D0CDCB6C115891C1";
			}
		};

		struct GenerationHash_Traits : public Sha3_256_Traits {
		public:
			using HashBuilder = GenerationHash_Builder;
			using HashType = HashBuilder::OutputType;

			static void HashFunc(const RawBuffer& dataBuffer, GenerationHash& hash) noexcept {
				// workaround because there is no source GenerationHash function
				Hash256 tempHash;
				Sha3_256(dataBuffer, tempHash);
				std::copy(tempHash.cbegin(), tempHash.cend(), hash.begin());
			}
		};

		// endregion

		// region single call hash function tests

		template<typename TTraits>
		void AssertEmptyStringHasExpectedHash() {
			// Arrange:
			auto buffer = test::HexStringToVector("");
			auto expectedHash = utils::ParseByteArray<typename TTraits::HashType>(TTraits::EmptyStringHash());

			// Act:
			typename TTraits::HashType hash;
			TTraits::HashFunc(buffer, hash);

			// Assert:
			EXPECT_EQ(expectedHash, hash);
		}

		template<typename TTraits>
		void AssertSampleTestVectors() {
			// Arrange:
			auto dataSet = TTraits::SampleTestVectorsInput();
			auto expectedHashes = TTraits::SampleTestVectorsOutput();

			// Sanity:
			ASSERT_EQ(dataSet.size(), expectedHashes.size());

			auto i = 0u;
			for (const auto& dataHexStr : dataSet) {
				auto buffer = test::HexStringToVector(dataHexStr);

				// Act:
				typename TTraits::HashType hash;
				TTraits::HashFunc(buffer, hash);

				// Assert:
				EXPECT_EQ(utils::ParseByteArray<typename TTraits::HashType>(expectedHashes[i]), hash);
				++i;
			}
		}

		template<typename TTraits>
		void AssertMillionTimesAHasExpectedHash() {
			// Arrange:
			std::vector<uint8_t> buffer(1'000'000, 'a');
			auto expectedHash = utils::ParseByteArray<typename TTraits::HashType>(TTraits::MillionTimesATestVector());

			// Act:
			typename TTraits::HashType hash;
			TTraits::HashFunc(buffer, hash);

			// Assert:
			EXPECT_EQ(expectedHash, hash);
		}

		// endregion
	}

#define MAKE_HASH_TEST(TRAITS_PREFIX, TEST_NAME) \
	TEST(TEST_CLASS, TRAITS_PREFIX##_##TEST_NAME) { Assert##TEST_NAME<TRAITS_PREFIX##_Traits>(); }

	// region Ripemd160

	MAKE_HASH_TEST(Ripemd160, EmptyStringHasExpectedHash)
	MAKE_HASH_TEST(Ripemd160, SampleTestVectors)
	MAKE_HASH_TEST(Ripemd160, MillionTimesAHasExpectedHash)

	// endregion

	// region Bitcoin160

	MAKE_HASH_TEST(Bitcoin160, EmptyStringHasExpectedHash)
	MAKE_HASH_TEST(Bitcoin160, SampleTestVectors)
	// MillionTimesAHasExpectedHash left out, couldn't find any resource on the network, that would confirm the value

	// endregion

	// region Sha256

	MAKE_HASH_TEST(Sha256, EmptyStringHasExpectedHash)
	MAKE_HASH_TEST(Sha256, SampleTestVectors)
	MAKE_HASH_TEST(Sha256, MillionTimesAHasExpectedHash)

	// endregion

	// region Sha256Double

	MAKE_HASH_TEST(Sha256Double, EmptyStringHasExpectedHash)
	MAKE_HASH_TEST(Sha256Double, SampleTestVectors)
	MAKE_HASH_TEST(Sha256Double, MillionTimesAHasExpectedHash)

	// endregion

	// region Sha512

	MAKE_HASH_TEST(Sha512, EmptyStringHasExpectedHash)
	MAKE_HASH_TEST(Sha512, SampleTestVectors)
	MAKE_HASH_TEST(Sha512, MillionTimesAHasExpectedHash)

	// endregion

	// region Sha3

#define SHA3_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, Sha3_256_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<Sha3_256_Traits>(); } \
	TEST(TEST_CLASS, GenerationHash_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<GenerationHash_Traits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	SHA3_TRAITS_BASED_TEST(EmptyStringHasExpectedHash) {
		AssertEmptyStringHasExpectedHash<TTraits>();
	}

	SHA3_TRAITS_BASED_TEST(SampleTestVectors) {
		AssertSampleTestVectors<TTraits>();
	}

	SHA3_TRAITS_BASED_TEST(MillionTimesAHasExpectedHash) {
		AssertMillionTimesAHasExpectedHash<TTraits>();
	}

	// endregion

	// region Hmac_Sha256 / Hmac_Sha512

	// data from: https://github.com/randombit/botan/blob/master/src/tests/data/mac/hmac.vec

	namespace {
		struct Hmac_Sha256_Traits {
		public:
			using HashType = Hash256;

			static constexpr auto Hmac = Hmac_Sha256;

		public:
			static std::vector<std::string> SampleTestVectorsKey() {
				return {
					"0102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F20",
					"4A656665",
					"0102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F20",
					"0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B",
					"0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C",
					"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAA",
					"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAA",

					// data from: NIST CAVP, [L=32], sample 30
					"9779D9120642797F1747025D5B22B7AC607CAB08E1758F2F3A46C8BE1E25C53B8C6A"
						"8F58FFEFA176"
				};
			}

			static std::vector<std::string> SampleTestVectorsInput() {
				return {
					"616263",
					"7768617420646F2079612077616E7420666F72206E6F7468696E673F",
					"6162636462636465636465666465666765666768666768696768696A68696A6B"
						"696A6B6C6A6B6C6D6B6C6D6E6C6D6E6F6D6E6F706E6F70716162636462636465"
						"636465666465666765666768666768696768696A68696A6B696A6B6C6A6B6C6D"
						"6B6C6D6E6C6D6E6F6D6E6F706E6F7071",
					"4869205468657265",
					"546573742057697468205472756E636174696F6E",
					"54657374205573696E67204C6172676572205468616E20426C6F636B2D53697A"
						"65204B6579202D2048617368204B6579204669727374",
					"5468697320697320612074657374207573696E672061206C6172676572207468"
						"616E20626C6F636B2D73697A65206B657920616E642061206C61726765722074"
						"68616E20626C6F636B2D73697A6520646174612E20546865206B6579206E6565"
						"647320746F20626520686173686564206265666F7265206265696E6720757365"
						"642062792074686520484D414320616C676F726974686D2E",
					"B1689C2591EAF3C9E66070F8A77954FFB81749F1B00346F9DFE0B2EE905DCC288BAF"
						"4A92DE3F4001DD9F44C468C3D07D6C6EE82FACEAFC97C2FC0FC0601719D2DCD0AA2A"
						"EC92D1B0AE933C65EB06A03C9C935C2BAD0459810241347AB87E9F11ADB30415424C"
						"6C7F5F22A003B8AB8DE54F6DED0E3AB9245FA79568451DFA258E"
				};
			}

			static std::vector<std::string> SampleTestVectorsOutput() {
				return {
					"A21B1F5D4CF4F73A4DD939750F7A066A7F98CC131CB16A6692759021CFAB8181",
					"5BDCC146BF60754E6A042426089575C75A003F089D2739839DEC58B964EC3843",
					"470305FC7E40FE34D3EEB3E773D95AAB73ACF0FD060447A5EB4595BF33A9D1A3",
					"198A607EB44BFBC69903A0F1CF2BBDC5BA0AA3F3D9AE3C1C7A3B1696A0B68CF7",
					"7546AF01841FC09B1AB9C3749A5F1C17D4F589668A587B2700A9C97C1193CF42",
					"60E431591EE0B67F0D8A26AACBF5B77F8E0BC6213728C5140546040F0EE37F54",
					"9B09FFA71B942FCB27635FBCD5B0E944BFDC63644F0713938A7F51535C3A35E2",
					"769F00D3E6A6CC1FB426A14A4F76C6462E6149726E0DEE0EC0CF97A16605AC8B"
				};
			}
		};

		struct Hmac_Sha512_Traits {
		public:
			using HashType = Hash512;

			static constexpr auto Hmac = Hmac_Sha512;

		public:
			static std::vector<std::string> SampleTestVectorsKey() {
				return {
					"0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B",
					"4A656665",
					"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
					"0102030405060708090A0B0C0D0E0F10111213141516171819",
					"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAA",
					"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
						"AAAAAA"
				};
			}

			static std::vector<std::string> SampleTestVectorsInput() {
				return {
					"4869205468657265",
					"7768617420646F2079612077616E7420666F72206E6F7468696E673F",
					"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD",
					"CDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCD",
					"54657374205573696E67204C6172676572205468616E20426C6F636B2D53697A"
						"65204B6579202D2048617368204B6579204669727374",
					"5468697320697320612074657374207573696E672061206C6172676572207468"
						"616E20626C6F636B2D73697A65206B657920616E642061206C61726765722074"
						"68616E20626C6F636B2D73697A6520646174612E20546865206B6579206E6565"
						"647320746F20626520686173686564206265666F7265206265696E6720757365"
						"642062792074686520484D414320616C676F726974686D2E"
				};
			}

			static std::vector<std::string> SampleTestVectorsOutput() {
				return {
					"87AA7CDEA5EF619D4FF0B4241A1D6CB02379F4E2CE4EC2787AD0B30545E17CDE"
					"DAA833B7D6B8A702038B274EAEA3F4E4BE9D914EEB61F1702E696C203A126854",
					"164B7A7BFCF819E2E395FBE73B56E0A387BD64222E831FD610270CD7EA250554"
					"9758BF75C05A994A6D034F65F8F0E6FDCAEAB1A34D4A6B4B636E070A38BCE737",
					"FA73B0089D56A284EFB0F0756C890BE9B1B5DBDD8EE81A3655F83E33B2279D39"
					"BF3E848279A722C806B485A47E67C807B946A337BEE8942674278859E13292FB",
					"B0BA465637458C6990E5A8C5F61D4AF7E576D97FF94B872DE76F8050361EE3DB"
					"A91CA5C11AA25EB4D679275CC5788063A5F19741120C4F2DE2ADEBEB10A298DD",
					"80B24263C7C1A3EBB71493C1DD7BE8B49B46D1F41B4AEEC1121B013783F8F352"
					"6B56D037E05F2598BD0FD2215D6A1E5295E64F73F63F0AEC8B915A985D786598",
					"E37B6A775DC87DBAA4DFA9F96E5E3FFDDEBD71F8867289865DF5A32D20CDC944"
					"B6022CAC3C4982B10D5EEB55C3E4DE15134676FB6DE0446065C97440FA8C6A58"
				};
			}
		};

		template<typename TTraits>
		void RunHmacTestVectors() {
			// Arrange:
			auto keys = TTraits::SampleTestVectorsKey();
			auto inputs = TTraits::SampleTestVectorsInput();
			auto expectedHashes = TTraits::SampleTestVectorsOutput();

			// Sanity:
			ASSERT_EQ(keys.size(), expectedHashes.size());
			ASSERT_EQ(inputs.size(), expectedHashes.size());

			for (auto i = 0u; i < keys.size(); ++i) {
				auto key = test::HexStringToVector(keys[i]);
				auto input = test::HexStringToVector(inputs[i]);
				auto expected = expectedHashes[i];

				// Act:
				typename TTraits::HashType output;
				TTraits::Hmac(key, input, output);

				// Assert:
				EXPECT_EQ(utils::ParseByteArray<typename TTraits::HashType>(expected), output) << "test vector at " << i;
			}
		}
	}

	TEST(TEST_CLASS, Hmac_Sha256_SampleTestVectors) {
		RunHmacTestVectors<Hmac_Sha256_Traits>();
	}

	TEST(TEST_CLASS, Hmac_Sha512_SampleTestVectors) {
		RunHmacTestVectors<Hmac_Sha512_Traits>();
	}

	// endregion

	// region Pbkdf2_Sha512

	// data from: https://fossies.org/linux/nettle/testsuite/pbkdf2-test.c

	namespace {
		struct TestVectorPbkdf2 {
			RawString Salt;
			uint32_t IterationCount;
			RawString Password;
			std::string ExpectedResult;
		};

		std::vector<TestVectorPbkdf2> GetTestVectorsPbkdf2() {
			return {
				{
					{ "NaCL", 4 },
					1,
					{ "password", 8 },
					"73DECFA58AA2E84F94771A75736BB88BD3C7B38270CFB50CB390ED78B305656A"
					"F8148E52452B2216B2B8098B761FC6336060A09F76415E9F71EA47F9E9064306"
				},
				{
					{ "sa\0lt", 5 },
					1,
					{ "pass\0word", 9 },
					"71A0EC842ABD5C678BCFD145F09D83522F93361560563C4D0D63B88329871090"
					"E76604A49AF08FE7C9F57156C8790996B20F06BC535E5AB5440DF7E878296FA7"
				},
				{
					{ "salt\0\0\0", 7 },
					50,
					{ "passwordPASSWORDpassword", 24 },
					"016871A4C4B75F96857FD2B9F8CA28023B30EE2A39F5ADCAC8C9375F9BDA1CCD"
					"1B6F0B2FC3ADDA505412E79D890056C62E524C7D51154B1A8534575BD02DEE39"
				}
			};
		}
	}

	TEST(TEST_CLASS, Pbkdf2_Sha512_SampleTestVectors) {
		// Arrange:
		auto i = 0u;
		for (const auto& testVector : GetTestVectorsPbkdf2()) {
			// Act:
			Hash512 output;
			Pbkdf2_Sha512(
					{ reinterpret_cast<const uint8_t*>(testVector.Password.pData), testVector.Password.Size },
					{ reinterpret_cast<const uint8_t*>(testVector.Salt.pData), testVector.Salt.Size },
					testVector.IterationCount,
					output);

			// Assert:
			EXPECT_EQ(utils::ParseByteArray<Hash512>(testVector.ExpectedResult), output) << "test vector at " << i;
			++i;
		}
	}

	// endregion

	// region builder test utils

	namespace {
		// data from: http://mumble.net/~campbell/hg/sha3/kat/ShortMsgKAT_SHA3-256.txt)
		// same data is used for 512 variant
		const char* Data_Sets_Long[] = {
			"9F2FCC7C90DE090D6B87CD7E9718C1EA6CB21118FC2D5DE9F97E5DB6AC1E9C10",
			"DE8F1B3FAA4B7040ED4563C3B8E598253178E87E4D0DF75E4FF2F2DEDD5A0BE046",
			"62F154EC394D0BC757D045C798C8B87A00E0655D0481A7D2D9FB58D93AEDC676B5A0",
			"F5961DFD2B1FFFFDA4FFBF30560C165BFEDAB8CE0BE525845DEB8DC61004B7DB38467205F5DCFB34A2ACFE96C0"
		};

		template<typename THashBuilder, typename TCalculateHashSingle>
		void AssertConcatenatedHashMatchesSingleCallVariant(TCalculateHashSingle calculateHashSingle) {
			using OutputHashType = typename THashBuilder::OutputType;
			// Arrange:
			for (const auto& dataStr : Data_Sets_Long) {
				OutputHashType expected;
				auto buffer = test::HexStringToVector(dataStr);
				calculateHashSingle(buffer, expected);

				// Act:
				auto splitInTwo = buffer.size() / 2;
				OutputHashType result1;
				{
					THashBuilder hashBuilder;
					hashBuilder.update({
						{ buffer.data(), splitInTwo },
						{ buffer.data() + splitInTwo, buffer.size() - splitInTwo } });
					hashBuilder.final(result1);
				}

				auto splitInThree = buffer.size() / 3;
				OutputHashType result2;
				{
					THashBuilder hashBuilder;
					hashBuilder.update({
						{ buffer.data(), splitInThree },
						{ buffer.data() + splitInThree, splitInThree },
						{ buffer.data() + 2 * splitInThree, buffer.size() - 2 * splitInThree } });
					hashBuilder.final(result2);
				}

				auto splitInFour = buffer.size() / 4;
				OutputHashType result3;
				{
					THashBuilder hashBuilder;
					hashBuilder.update({
						{ buffer.data(), splitInFour },
						{ buffer.data() + splitInFour, splitInFour },
						{ buffer.data() + 2 * splitInFour, splitInFour },
						{ buffer.data() + 3 * splitInFour, buffer.size() - 3 * splitInFour } });
					hashBuilder.final(result3);
				}

				// Assert:
				EXPECT_EQ(expected, result1) << "two splits";
				EXPECT_EQ(expected, result2) << "three splits";
				EXPECT_EQ(expected, result3) << "four splits";
			}
		}

		template<typename THashBuilder, typename TCalculateHashSingle>
		void AssertBuilderBasedHashMatchesSingleCallVariant(TCalculateHashSingle calculateHashSingle) {
			using OutputHashType = typename THashBuilder::OutputType;
			// Arrange:
			for (const auto& dataStr : Data_Sets_Long) {
				OutputHashType expected;
				auto buffer = test::HexStringToVector(dataStr);
				calculateHashSingle(buffer, expected);

				// Act:
				OutputHashType results[5];
				for (auto i = 2u; i < 2 + CountOf(results); ++i) {
					auto partSize = buffer.size() / i;
					THashBuilder hashBuilder;
					for (auto j = 0u; j < i - 1; ++j)
						hashBuilder.update({ buffer.data() + partSize * j, partSize });

					hashBuilder.update({ buffer.data() + partSize * (i - 1), buffer.size() - partSize * (i - 1) });
					hashBuilder.final(results[i - 2]);
				}

				// Assert:
				for (const auto& result : results)
					EXPECT_EQ(expected, result);
			}
		}
	}

	// endregion

	// region Sha512 builder - tests

	TEST(TEST_CLASS, Sha512_ConcatenatedMatchesSingleCallVariant) {
		AssertConcatenatedHashMatchesSingleCallVariant<Sha512_Builder>(Sha512_Traits::HashFunc);
	}

	TEST(TEST_CLASS, Sha512_BuilderBasedMatchesSingleCallVariant) {
		AssertBuilderBasedHashMatchesSingleCallVariant<Sha512_Builder>(Sha512_Traits::HashFunc);
	}

	// endregion

	// region Sha3 builder - tests

	SHA3_TRAITS_BASED_TEST(ConcatenatedMatchesSingleCallVariant) {
		AssertConcatenatedHashMatchesSingleCallVariant<typename TTraits::HashBuilder>(TTraits::HashFunc);
	}

	SHA3_TRAITS_BASED_TEST(BuilderBasedMatchesSingleCallVariant) {
		AssertBuilderBasedHashMatchesSingleCallVariant<typename TTraits::HashBuilder>(TTraits::HashFunc);
	}

	// endregion
}}
