#include "catapult/crypto/Hashes.h"
#include "catapult/types.h"
#include "tests/TestHarness.h"
#include <sstream>
#include <string>

namespace catapult { namespace crypto {

	namespace {
		// data taken from http://mumble.net/~campbell/hg/sha3/kat/ShortMsgKAT_SHA3-256.txt)
		// same data is used for 512 variant
		static const char* Data_Sets_Long[] = {
			"9F2FCC7C90DE090D6B87CD7E9718C1EA6CB21118FC2D5DE9F97E5DB6AC1E9C10",
			"DE8F1B3FAA4B7040ED4563C3B8E598253178E87E4D0DF75E4FF2F2DEDD5A0BE046",
			"62F154EC394D0BC757D045C798C8B87A00E0655D0481A7D2D9FB58D93AEDC676B5A0",
			"F5961DFD2B1FFFFDA4FFBF30560C165BFEDAB8CE0BE525845DEB8DC61004B7DB38467205F5DCFB34A2ACFE96C0"
		};
		static const char* Data_Set_Shorter[] = {
			"",
			"CC",
			"41FB",
			"1F877C",
			"C1ECFDFC",
			"9F2FCC7C90DE090D6B87CD7E9718C1EA6CB21118FC2D5DE9F97E5DB6AC1E9C10",
		};
		template<typename THashBuilder, typename TCalculateHashSingle>
		void AssertConcatenatedSha3MatchesSingleCallVariant(TCalculateHashSingle calculateHashSingle) {
			using TOutputHash = typename THashBuilder::OutputType;
			// Arrange:
			for (const auto& dataStr : Data_Sets_Long) {
				TOutputHash expected;
				auto data = test::ToVector(dataStr);
				calculateHashSingle(data, expected);

				// Act:
				auto splitInTwo = data.size() / 2;
				TOutputHash result1;
				{
					THashBuilder sha3obj;
					sha3obj.update({
						{ data.data(), splitInTwo },
						{ data.data() + splitInTwo, data.size() - splitInTwo } });
					sha3obj.final(result1);
				}

				auto splitInThree = data.size() / 3;
				TOutputHash result2;
				{
					THashBuilder sha3obj;
					sha3obj.update({
						{ data.data(), splitInThree },
						{ data.data() + splitInThree, splitInThree },
						{ data.data() + 2 * splitInThree, data.size() - 2 * splitInThree } });
					sha3obj.final(result2);
				}

				auto splitInFour = data.size() / 4;
				TOutputHash result3;
				{
					THashBuilder sha3obj;
					sha3obj.update({
						{ data.data(), splitInFour },
						{ data.data() + splitInFour, splitInFour },
						{ data.data() + 2 * splitInFour, splitInFour },
						{ data.data() + 3 * splitInFour, data.size() - 3 * splitInFour } });
					sha3obj.final(result3);
				}

				// Assert:
				EXPECT_EQ(test::ToHexString(expected), test::ToHexString(result1)) << "two splits";
				EXPECT_EQ(test::ToHexString(expected), test::ToHexString(result2)) << "three splits";
				EXPECT_EQ(test::ToHexString(expected), test::ToHexString(result3)) << "four splits";
			}
		}

		template<typename THashBuilder, typename TCalculateHashSingle>
		void AssertObjectBasedShaMatchesSingleCallVariant(TCalculateHashSingle calculateHashSingle) {
			using TOutputHash = typename THashBuilder::OutputType;
			// Arrange:
			for (const auto& dataStr : Data_Sets_Long) {
				TOutputHash expected;
				auto data = test::ToVector(dataStr);
				calculateHashSingle(data, expected);

				// Act:
				TOutputHash results[5];
				for (auto j = 2u; j < 2 + CountOf(results); ++j) {
					auto partSize = data.size() / j;
					THashBuilder sha3obj;
					for (auto k = 0u; k < j - 1; ++k)
						sha3obj.update({ data.data() + partSize * k, partSize });
					sha3obj.update({ data.data() + partSize * (j - 1), data.size() - partSize * (j - 1) });
					sha3obj.final(results[j - 2]);
				}

				// Assert:
				for (const auto& result : results)
					EXPECT_EQ(test::ToHexString(expected), test::ToHexString(result));
			}
		}
	}

	TEST(Sha3_512Test, SampleSha512TestVectors) {
		// Arrange:
#ifdef NIS1_COMPATIBLE_SIGNATURES
		std::string expectedSet[] {
			"0EAB42DE4C3CEB9235FC91ACFFE746B29C29A8C366B7C60E4E67C466F36A4304C00FA9CAF9D87976BA469BCBE06713B435F091EF2769FB160CDAB33D3670680E",
			"8630C13CBD066EA74BBE7FE468FEC1DEE10EDC1254FB4C1B7C5FD69B646E44160B8CE01D05A0908CA790DFB080F4B513BC3B6225ECE7A810371441A5AC666EB9",
			"551DA6236F8B96FCE9F97F1190E901324F0B45E06DBBB5CDB8355D6ED1DC34B3F0EAE7DCB68622FF232FA3CECE0D4616CDEB3931F93803662A28DF1CD535B731",
			"EB7F2A98E00AF37D964F7D8C44C1FB6E114D8EE21A7B976AE736539EFDC1E3FE43BECEF5015171E6DA30168CAE99A82C53FA99042774EF982C01626A540F08C0",
			"952D4C0A6F0EF5CE438C52E3EDD345EA00F91CF5DA8097C1168A16069E958FC05BAD90A0C5FB4DD9EC28E84B226B94A847D6BB89235692EF4C9712F0C7030FAE",
			"1EAFEDCE7292BA73B80AE6151745F43AC95BFC9F31694D422473ABCA2E69D695CB6544DB65506078CB20DBE0762F84AA6AFD14A60AB597955BE73F3F5C50F7A8",
		};
#else
		std::string expectedSet[] {
			"A69F73CCA23A9AC5C8B567DC185A756E97C982164FE25859E0D1DCC1475C80A615B2123AF1F5F94C11E3E9402C3AC558F500199D95B6D3E301758586281DCD26",
			"3939FCC8B57B63612542DA31A834E5DCC36E2EE0F652AC72E02624FA2E5ADEECC7DD6BB3580224B4D6138706FC6E80597B528051230B00621CC2B22999EAA205",
			"AA092865A40694D91754DBC767B5202C546E226877147A95CB8B4C8F8709FE8CD6905256B089DA37896EA5CA19D2CD9AB94C7192FC39F7CD4D598975A3013C69",
			"CB20DCF54955F8091111688BECCEF48C1A2F0D0608C3A575163751F002DB30F40F2F671834B22D208591CFAF1F5ECFE43C49863A53B3225BDFD7C6591BA7658B",
			"D4B4BDFEF56B821D36F4F70AB0D231B8D0C9134638FD54C46309D14FADA92A2840186EED5415AD7CF3969BDFBF2DAF8CCA76ABFE549BE6578C6F4143617A4F1A",
			"B087C90421AEBF87911647DE9D465CBDA166B672EC47CCD4054A7135A1EF885E7903B52C3F2C3FE722B1C169297A91B82428956A02C631A2240F12162C7BC726",
		};
#endif

		ASSERT_EQ(CountOf(Data_Set_Shorter), CountOf(expectedSet));
		for (size_t i = 0; i < CountOf(Data_Set_Shorter); ++i) {
			Hash512 result;
			auto data = test::ToVector(Data_Set_Shorter[i]);

			// Act:
			crypto::Sha3_512(data, result);

			// Assert:
			EXPECT_EQ(expectedSet[i], test::ToHexString(result));
		}
	}

	TEST(Sha3_512Test, ConcatenatedShaMatchesSingleCallVariant) {
		auto fun1 = static_cast<void(*)(const RawBuffer&, Hash512&)>(&crypto::Sha3_512);
		AssertConcatenatedSha3MatchesSingleCallVariant<Sha3_512_Builder>(fun1);
	}

	TEST(Sha3_512Test, ObjectBasedShaMatchesSingleCallVariant) {
		auto fun1 = static_cast<void(*)(const RawBuffer&, Hash512&)>(&crypto::Sha3_512);
		AssertObjectBasedShaMatchesSingleCallVariant<Sha3_512_Builder>(fun1);
	}

	TEST(Sha3_256Test, SampleSha256TestVectors) {
		// Arrange:
#ifdef NIS1_COMPATIBLE_SIGNATURES
		std::string expectedSet[] {
			"C5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470",
			"EEAD6DBFC7340A56CAEDC044696A168870549A6A7F6F56961E84A54BD9970B8A",
			"A8EACEDA4D47B3281A795AD9E1EA2122B407BAF9AABCB9E18B5717B7873537D2",
			"627D7BC1491B2AB127282827B8DE2D276B13D7D70FB4C5957FDF20655BC7AC30",
			"B149E766D7612EAF7D55F74E1A4FDD63709A8115B14F61FCD22AA4ABC8B8E122",
			"24DD2EE02482144F539F810D2CAA8A7B75D0FA33657E47932122D273C3F6F6D1",
		};
#else
		std::string expectedSet[] {
			"A7FFC6F8BF1ED76651C14756A061D662F580FF4DE43B49FA82D80A4B80F8434A",
			"677035391CD3701293D385F037BA32796252BB7CE180B00B582DD9B20AAAD7F0",
			"39F31B6E653DFCD9CAED2602FD87F61B6254F581312FB6EEEC4D7148FA2E72AA",
			"BC22345E4BD3F792A341CF18AC0789F1C9C966712A501B19D1B6632CCD408EC5",
			"C5859BE82560CC8789133F7C834A6EE628E351E504E601E8059A0667FF62C124",
			"2F1A5F7159E34EA19CDDC70EBF9B81F1A66DB40615D7EAD3CC1F1B954D82A3AF",
		};
#endif

		ASSERT_EQ(CountOf(Data_Set_Shorter), CountOf(expectedSet));
		for (size_t i = 0; i < CountOf(Data_Set_Shorter); ++i) {
			Hash256 result;
			auto data = test::ToVector(Data_Set_Shorter[i]);

			// Act:
			crypto::Sha3_256(data, result);

			// Assert:
			EXPECT_EQ(expectedSet[i], test::ToHexString(result));
		}
	}

	TEST(Sha3_256Test, ConcatenatedShaMatchesSingleCallVariant) {
		auto fun1 = static_cast<void(*)(const RawBuffer&, Hash256&)>(&crypto::Sha3_256);
		AssertConcatenatedSha3MatchesSingleCallVariant<Sha3_256_Builder>(fun1);
	}

	TEST(Sha3_256Test, ObjectBasedShaMatchesSingleCallVariant) {
		auto fun1 = static_cast<void(*)(const RawBuffer&, Hash256&)>(&crypto::Sha3_256);
		AssertObjectBasedShaMatchesSingleCallVariant<Sha3_256_Builder>(fun1);
	}

	TEST(Sha3_256Test, NonAlignedShaObjectProducesSameResults) {
		// Arrange:
		auto data = test::GenerateRandomVector(1 * 1024 * 1024);

		Hash256 expected1;
		Hash256 nonAlignedResult;
		{
			Sha3_256_Builder sha3obj;
			sha3obj.update(data);
			sha3obj.final(expected1);
		}

		// Act:
		{
			uint8_t buffer[512];
			auto pSha3Obj = new (buffer + 3) Sha3_256_Builder();
			Sha3_256_Builder& sha3obj = *pSha3Obj;
			sha3obj.update(data);
			sha3obj.final(nonAlignedResult);
		}

		// Assert:
		EXPECT_EQ(expected1, nonAlignedResult);
	}
}}
