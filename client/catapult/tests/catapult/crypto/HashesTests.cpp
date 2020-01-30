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

			// data taken from : https://homes.esat.kuleuven.be/~bosselae/ripemd160.html
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

			static std::vector<std::string> CatapultStringTestVectors() {
				return {
					"37F332F68DB77BD9D7EDD4969571AD671CF9DD3B",
					"114C70B78838555E6C3AB418F3052A949F73544A",
					"7D0982BE59EBE828D02AA0D031AA6651644D60DA",
					"5A4535208909435DECD5C7D6D818F67626A177E4",
					"1B3ACB0409F7BA78A0BE07A2DE5454DCB0D48817"
				};
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

			static std::vector<std::string> CatapultStringTestVectors() {
				return {
					"0E3397B4ABC7A382B3EA2365883C3C7CA5F07600",
					"5367F36F9B941E14EBE4B30783B8A32274481A62",
					"17F200E445D54D96986F579662A85D7BF0A1E106",
					"359327D26A3A91458E0F6FDD9BB9F4FF77B9299F",
					"18638F1D82C80B35703A4B89227E3D567FA03BEA"
				};
			}
		};

		struct Sha256Double_Traits {
			using HashType = Hash256;

			static constexpr auto HashFunc = Sha256Double;

			static std::string EmptyStringHash() {
				return "5DF6E0E2761359D30A8275058E299FCC0381534545F55CF43E41983F5D4C9456";
			}

			// data taken from : https://www.dlitz.net/crypto/shad256-test-vectors/
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

			// those vectors were generated
			// non authorative results: https://github.com/DeathAndTaxes/BitCrypto/blob/master/BitCrypto.Test/Sha256Tests.cs
			static std::vector<std::string> CatapultStringTestVectors() {
				return {
					"6D37795021E544D82B41850EDF7AABAB9A0EBE274E54A519840C4666F35B3937",
					"DB6F466A6C6B50BE9AE850C01693BA95BEDC4A8CF8028D2B52B8429F406F6F2F",
					"A1DB794104F5A6532731E7A0F3FD39077932A3B978CC9E250F20259DA900DAD4",
					"7FE281A3E3F5AF049328818B02B8BEC1F9BAE1B5C5D8D3DD3F0AACE75700604F",
					"94A09FEA3A99ED4CFE79819BDEA443AF4B35C83E9E92AD21669B521C5375FA9C"
				};
			}
		};

		struct Keccak_Base {
			// data taken from http://mumble.net/~campbell/hg/sha3/kat/ShortMsgKAT_SHA3-256.txt
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

			// vectors taken from http://mumble.net/~campbell/hg/sha3/kat/ShortMsgKAT_SHA3-256.txt
			static std::vector<std::string> SampleTestVectorsOutput() {
				return {
					"677035391CD3701293D385F037BA32796252BB7CE180B00B582DD9B20AAAD7F0",
					"39F31B6E653DFCD9CAED2602FD87F61B6254F581312FB6EEEC4D7148FA2E72AA",
					"BC22345E4BD3F792A341CF18AC0789F1C9C966712A501B19D1B6632CCD408EC5",
					"C5859BE82560CC8789133F7C834A6EE628E351E504E601E8059A0667FF62C124",
					"2F1A5F7159E34EA19CDDC70EBF9B81F1A66DB40615D7EAD3CC1F1B954D82A3AF"
				};
			}

			// taken from https://www.di-mgt.com.au/sha_testvectors.html
			static std::string MillionTimesATestVector() {
				return "5C8875AE474A3634BA4FD55EC85BFFD661F32ACA75C6D699D0CDCB6C115891C1";
			}
		};

		struct Sha3_512_Traits : public Keccak_Base {
			using HashBuilder = Sha3_512_Builder;
			using HashType = HashBuilder::OutputType;

			static constexpr auto HashFunc = Sha3_512;

			static std::string EmptyStringHash() {
				return "A69F73CCA23A9AC5C8B567DC185A756E97C982164FE25859E0D1DCC1475C80A6"
						"15B2123AF1F5F94C11E3E9402C3AC558F500199D95B6D3E301758586281DCD26";
			}

			// vectors taken from http://mumble.net/~campbell/hg/sha3/kat/ShortMsgKAT_SHA3-512.txt
			static std::vector<std::string> SampleTestVectorsOutput() {
				return {
					"3939FCC8B57B63612542DA31A834E5DCC36E2EE0F652AC72E02624FA2E5ADEEC"
					"C7DD6BB3580224B4D6138706FC6E80597B528051230B00621CC2B22999EAA205",
					"AA092865A40694D91754DBC767B5202C546E226877147A95CB8B4C8F8709FE8C"
					"D6905256B089DA37896EA5CA19D2CD9AB94C7192FC39F7CD4D598975A3013C69",
					"CB20DCF54955F8091111688BECCEF48C1A2F0D0608C3A575163751F002DB30F4"
					"0F2F671834B22D208591CFAF1F5ECFE43C49863A53B3225BDFD7C6591BA7658B",
					"D4B4BDFEF56B821D36F4F70AB0D231B8D0C9134638FD54C46309D14FADA92A28"
					"40186EED5415AD7CF3969BDFBF2DAF8CCA76ABFE549BE6578C6F4143617A4F1A",
					"B087C90421AEBF87911647DE9D465CBDA166B672EC47CCD4054A7135A1EF885E"
					"7903B52C3F2C3FE722B1C169297A91B82428956A02C631A2240F12162C7BC726"
				};
			}

			// taken from https://www.di-mgt.com.au/sha_testvectors.html
			static std::string MillionTimesATestVector() {
				return "3C3A876DA14034AB60627C077BB98F7E120A2A5370212DFFB3385A18D4F38859"
						"ED311D0A9D5141CE9CC5C66EE689B266A8AA18ACE8282A0E0DB596C90B0A7B87";
			}
		};

		struct Keccak_256_Traits : public Keccak_Base {
			using HashBuilder = Keccak_256_Builder;
			using HashType = HashBuilder::OutputType;

			static constexpr auto HashFunc = Keccak_256;

			static std::string EmptyStringHash() {
				return "C5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470";
			}

			static std::vector<std::string> SampleTestVectorsOutput() {
				return {
					"EEAD6DBFC7340A56CAEDC044696A168870549A6A7F6F56961E84A54BD9970B8A",
					"A8EACEDA4D47B3281A795AD9E1EA2122B407BAF9AABCB9E18B5717B7873537D2",
					"627D7BC1491B2AB127282827B8DE2D276B13D7D70FB4C5957FDF20655BC7AC30",
					"B149E766D7612EAF7D55F74E1A4FDD63709A8115B14F61FCD22AA4ABC8B8E122",
					"24DD2EE02482144F539F810D2CAA8A7B75D0FA33657E47932122D273C3F6F6D1"
				};
			}

			// vector was generated
			// non authorative results: https://github.com/weidai11/cryptopp/blob/master/TestVectors/keccak.txt
			static std::string MillionTimesATestVector() {
				return "FADAE6B49F129BBB812BE8407B7B2894F34AECF6DBD1F9B0F0C7E9853098FC96";
			}
		};

		struct Keccak_512_Traits : public Keccak_Base {
			using HashBuilder = Keccak_512_Builder;
			using HashType = HashBuilder::OutputType;

			static constexpr auto HashFunc = Keccak_512;

			static std::string EmptyStringHash() {
				return "0EAB42DE4C3CEB9235FC91ACFFE746B29C29A8C366B7C60E4E67C466F36A4304"
						"C00FA9CAF9D87976BA469BCBE06713B435F091EF2769FB160CDAB33D3670680E";
			}

			static std::vector<std::string> SampleTestVectorsOutput() {
				return {
					"8630C13CBD066EA74BBE7FE468FEC1DEE10EDC1254FB4C1B7C5FD69B646E4416"
					"0B8CE01D05A0908CA790DFB080F4B513BC3B6225ECE7A810371441A5AC666EB9",
					"551DA6236F8B96FCE9F97F1190E901324F0B45E06DBBB5CDB8355D6ED1DC34B3"
					"F0EAE7DCB68622FF232FA3CECE0D4616CDEB3931F93803662A28DF1CD535B731",
					"EB7F2A98E00AF37D964F7D8C44C1FB6E114D8EE21A7B976AE736539EFDC1E3FE"
					"43BECEF5015171E6DA30168CAE99A82C53FA99042774EF982C01626A540F08C0",
					"952D4C0A6F0EF5CE438C52E3EDD345EA00F91CF5DA8097C1168A16069E958FC0"
					"5BAD90A0C5FB4DD9EC28E84B226B94A847D6BB89235692EF4C9712F0C7030FAE",
					"1EAFEDCE7292BA73B80AE6151745F43AC95BFC9F31694D422473ABCA2E69D695"
					"CB6544DB65506078CB20DBE0762F84AA6AFD14A60AB597955BE73F3F5C50F7A8"
				};
			}

			// vector was generated
			// non authorative results: https://github.com/weidai11/cryptopp/blob/master/TestVectors/keccak.txt
			static std::string MillionTimesATestVector() {
				return "5CF53F2E556BE5A624425EDE23D0E8B2C7814B4BA0E4E09CBBF3C2FAC7056F61"
						"E048FC341262875EBC58A5183FEA651447124370C1EBF4D6C89BC9A7731063BB";
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

		template<typename TTraits>
		void AssertCatapultStringHasExpectedHash() {
			// Arrange:
			std::vector<std::string> dataSet{
				"The quick brown fox jumps over the lazy dog",
				"Kitten Kaboodle",
				"Lorem ipsum dolor sit amet",
				"GimreJaguar0625BloodyRookie",
				"The ripe taste of cheese improves with age"
			};
			auto expectedHashes = TTraits::CatapultStringTestVectors();

			// Sanity:
			ASSERT_EQ(dataSet.size(), expectedHashes.size());

			auto i = 0u;
			for (const auto& dataStr : dataSet) {
				// Arrange:
				auto hex = AsciiToHexString(dataStr);
				auto buffer = test::HexStringToVector(hex);

				// Act:
				typename TTraits::HashType hash;
				TTraits::HashFunc(buffer, hash);

				// Assert:
				EXPECT_EQ(utils::ParseByteArray<typename TTraits::HashType>(expectedHashes[i]), hash);
				++i;
			}
		}

		// endregion
	}

#define MAKE_HASH_TEST(TRAITS_PREFIX, TEST_NAME) \
	TEST(TEST_CLASS, TRAITS_PREFIX##_##TEST_NAME) { Assert##TEST_NAME<TRAITS_PREFIX##_Traits>(); }

	// region Ripemd160

	MAKE_HASH_TEST(Ripemd160, EmptyStringHasExpectedHash)
	MAKE_HASH_TEST(Ripemd160, SampleTestVectors)
	MAKE_HASH_TEST(Ripemd160, MillionTimesAHasExpectedHash)
	MAKE_HASH_TEST(Ripemd160, CatapultStringHasExpectedHash)

	// endregion

	// region Bitcoin160

	MAKE_HASH_TEST(Bitcoin160, EmptyStringHasExpectedHash)
	MAKE_HASH_TEST(Bitcoin160, SampleTestVectors)
	// MillionTimesAHasExpectedHash left out, couldn't find any resource on the network, that would confirm the value
	MAKE_HASH_TEST(Bitcoin160, CatapultStringHasExpectedHash)

	// endregion

	// region Sha256Double

	MAKE_HASH_TEST(Sha256Double, EmptyStringHasExpectedHash)
	MAKE_HASH_TEST(Sha256Double, SampleTestVectors)
	MAKE_HASH_TEST(Sha256Double, MillionTimesAHasExpectedHash)
	MAKE_HASH_TEST(Sha256Double, CatapultStringHasExpectedHash)

	// endregion

	// region sha3 / keccak free function tests

#define SHA3_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, Sha3_256_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<Sha3_256_Traits>(); } \
	TEST(TEST_CLASS, Sha3_512_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<Sha3_512_Traits>(); } \
	TEST(TEST_CLASS, Keccak_256_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<Keccak_256_Traits>(); } \
	TEST(TEST_CLASS, Keccak_512_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<Keccak_512_Traits>(); } \
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

	// CatapultStringHasExpectedHash is intentionally left out

	// endregion

	// region Hmac Sha256 tests

	// data from: https://github.com/randombit/botan/blob/master/src/tests/data/mac/hmac.vec

	namespace {
		struct Hmac_Sha256_Traits {
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

					// from NIST CAVP, [L=32], sample 30
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
	}

	TEST(TEST_CLASS, Hmac_Sha256_SampleTestVectors) {
		// Arrange:
		auto keys = Hmac_Sha256_Traits::SampleTestVectorsKey();
		auto inputs = Hmac_Sha256_Traits::SampleTestVectorsInput();
		auto expectedHashes = Hmac_Sha256_Traits::SampleTestVectorsOutput();

		// Sanity:
		ASSERT_EQ(keys.size(), expectedHashes.size());
		ASSERT_EQ(inputs.size(), expectedHashes.size());

		for (auto i = 0u; i < keys.size(); ++i) {
			auto key = test::HexStringToVector(keys[i]);
			auto input = test::HexStringToVector(inputs[i]);
			auto expected = expectedHashes[i];

			// Act:
			Hash256 output;
			Hmac_Sha256(key, input, output);

			// Assert:
			EXPECT_EQ(utils::ParseByteArray<Hash256>(expected), output) << "at test vector " << i;
		}
	}

	// endregion

	// region Sha3 builder - utils

	namespace {
		// data taken from http://mumble.net/~campbell/hg/sha3/kat/ShortMsgKAT_SHA3-256.txt)
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

	// region Sha3 builder - tests

	SHA3_TRAITS_BASED_TEST(ConcatenatedMatchesSingleCallVariant) {
		AssertConcatenatedHashMatchesSingleCallVariant<typename TTraits::HashBuilder>(TTraits::HashFunc);
	}

	SHA3_TRAITS_BASED_TEST(BuilderBasedMatchesSingleCallVariant) {
		AssertBuilderBasedHashMatchesSingleCallVariant<typename TTraits::HashBuilder>(TTraits::HashFunc);
	}

	// endregion
}}
