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

#include "src/extensions/Bip39.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS Bip39Tests

	namespace {
		// region test vectors

		// data from: https://github.com/trezor/python-mnemonic/blob/master/vectors.json
		struct TestVectorBip39 {
			std::string Entroy;
			std::string Mnemonic;
			std::string Seed;
		};

		std::vector<TestVectorBip39> GetTestVectorsBip39() {
			return {
				{
					"00000000000000000000000000000000",
					"abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
					"C55257C360C07C72029AEBC1B53C05ED0362ADA38EAD3E3E9EFA3708E5349553"
					"1F09A6987599D18264C1E1C92F2CF141630C7A3C4AB7C81B2F001698E7463B04"
				},
				{
					"7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f",
					"legal winner thank year wave sausage worth useful legal winner thank yellow",
					"2E8905819B8723FE2C1D161860E5EE1830318DBF49A83BD451CFB8440C28BD6F"
					"A457FE1296106559A3C80937A1C1069BE3A3A5BD381EE6260E8D9739FCE1F607"
				},
				{
					"80808080808080808080808080808080",
					"letter advice cage absurd amount doctor acoustic avoid letter advice cage above",
					"D71DE856F81A8ACC65E6FC851A38D4D7EC216FD0796D0A6827A3AD6ED5511A30"
					"FA280F12EB2E47ED2AC03B5C462A0358D18D69FE4F985EC81778C1B370B652A8"
				},
				{
					"ffffffffffffffffffffffffffffffff",
					"zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo wrong",
					"AC27495480225222079D7BE181583751E86F571027B0497B5B5D11218E0A8A13"
					"332572917F0F8E5A589620C6F15B11C61DEE327651A14C34E18231052E48C069"
				},
				{
					"000000000000000000000000000000000000000000000000",
					"abandon abandon abandon abandon abandon abandon abandon abandon abandon"
					" abandon abandon abandon abandon abandon abandon abandon abandon agent",
					"035895F2F481B1B0F01FCF8C289C794660B289981A78F8106447707FDD9666CA"
					"06DA5A9A565181599B79F53B844D8A71DD9F439C52A3D7B3E8A79C906AC845FA"
				},
				{
					"7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f",
					"legal winner thank year wave sausage worth useful legal winner thank year wave sausage worth useful legal will",
					"F2B94508732BCBACBCC020FAEFECFC89FEAFA6649A5491B8C952CEDE496C214A"
					"0C7B3C392D168748F2D4A612BADA0753B52A1C7AC53C1E93ABD5C6320B9E95DD"
				},
				{
					"808080808080808080808080808080808080808080808080",
					"letter advice cage absurd amount doctor acoustic avoid letter"
					" advice cage absurd amount doctor acoustic avoid letter always",
					"107D7C02A5AA6F38C58083FF74F04C607C2D2C0ECC55501DADD72D025B751BC2"
					"7FE913FFB796F841C49B1D33B610CF0E91D3AA239027F5E99FE4CE9E5088CD65"
				},
				{
					"ffffffffffffffffffffffffffffffffffffffffffffffff",
					"zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo when",
					"0CD6E5D827BB62EB8FC1E262254223817FD068A74B5B449CC2F667C3F1F985A7"
					"6379B43348D952E2265B4CD129090758B3E3C2C49103B5051AAC2EAEB890A528"
				},
				{
					"0000000000000000000000000000000000000000000000000000000000000000",
					"abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon"
					" abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon art",
					"BDA85446C68413707090A52022EDD26A1C9462295029F2E60CD7C4F2BBD30971"
					"70AF7A4D73245CAFA9C3CCA8D561A7C3DE6F5D4A10BE8ED2A5E608D68F92FCC8"
				},
				{
					"7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f",
					"legal winner thank year wave sausage worth useful legal winner thank year"
					" wave sausage worth useful legal winner thank year wave sausage worth title",
					"BC09FCA1804F7E69DA93C2F2028EB238C227F2E9DDA30CD63699232578480A40"
					"21B146AD717FBB7E451CE9EB835F43620BF5C514DB0F8ADD49F5D121449D3E87"
				},
				{
					"8080808080808080808080808080808080808080808080808080808080808080",
					"letter advice cage absurd amount doctor acoustic avoid letter advice cage absurd"
					" amount doctor acoustic avoid letter advice cage absurd amount doctor acoustic bless",
					"C0C519BD0E91A2ED54357D9D1EBEF6F5AF218A153624CF4F2DA911A0ED8F7A09"
					"E2EF61AF0ACA007096DF430022F7A2B6FB91661A9589097069720D015E4E982F"
				},
				{
					"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
					"zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo vote",
					"DD48C104698C30CFE2B6142103248622FB7BB0FF692EEBB00089B32D22484E16"
					"13912F0A5B694407BE899FFD31ED3992C456CDF60F5D4564B8BA3F05A69890AD"
				},
				{
					"9e885d952ad362caeb4efe34a8e91bd2",
					"ozone drill grab fiber curtain grace pudding thank cruise elder eight picnic",
					"274DDC525802F7C828D8EF7DDBCDC5304E87AC3535913611FBBFA986D0C9E547"
					"6C91689F9C8A54FD55BD38606AA6A8595AD213D4C9C9F9ACA3FB217069A41028"
				},
				{
					"6610b25967cdcca9d59875f5cb50b0ea75433311869e930b",
					"gravity machine north sort system female filter attitude volume"
					" fold club stay feature office ecology stable narrow fog",
					"628C3827A8823298EE685DB84F55CAA34B5CC195A778E52D45F59BCF75ABA68E"
					"4D7590E101DC414BC1BBD5737666FBBEF35D1F1903953B66624F910FEEF245AC"
				},
				{
					"68a79eaca2324873eacc50cb9c6eca8cc68ea5d936f98787c60c7ebc74e6ce7c",
					"hamster diagram private dutch cause delay private meat slide toddler razor book"
					" happy fancy gospel tennis maple dilemma loan word shrug inflict delay length",
					"64C87CDE7E12ECF6704AB95BB1408BEF047C22DB4CC7491C4271D170A1B213D2"
					"0B385BC1588D9C7B38F1B39D415665B8A9030C9EC653D75E65F847D8FC1FC440"
				},
				{
					"c0ba5a8e914111210f2bd131f3d5e08d",
					"scheme spot photo card baby mountain device kick cradle pact join borrow",
					"EA725895AAAE8D4C1CF682C1BFD2D358D52ED9F0F0591131B559E2724BB234FC"
					"A05AA9C02C57407E04EE9DC3B454AA63FBFF483A8B11DE949624B9F1831A9612"
				},
				{
					"6d9be1ee6ebd27a258115aad99b7317b9c8d28b6d76431c3",
					"horn tenant knee talent sponsor spell gate clip pulse soap slush warm silver nephew swap uncle crack brave",
					"FD579828AF3DA1D32544CE4DB5C73D53FC8ACC4DDB1E3B251A31179CDB71E853"
					"C56D2FCB11AED39898CE6C34B10B5382772DB8796E52837B54468AEB312CFC3D"
				},
				{
					"9f6a2878b2520799a44ef18bc7df394e7061a224d2c33cd015b157d746869863",
					"panda eyebrow bullet gorilla call smoke muffin taste mesh discover soft ostrich"
					" alcohol speed nation flash devote level hobby quick inner drive ghost inside",
					"72BE8E052FC4919D2ADF28D5306B5474B0069DF35B02303DE8C1729C9538DBB6"
					"FC2D731D5F832193CD9FB6AEECBC469594A70E3DD50811B5067F3B88B28C3E8D"
				},
				{
					"23db8160a31d3e0dca3688ed941adbf3",
					"cat swing flag economy stadium alone churn speed unique patch report train",
					"DEB5F45449E615FEFF5640F2E49F933FF51895DE3B4381832B3139941C57B592"
					"05A42480C52175B6EFCFFAA58A2503887C1E8B363A707256BDD2B587B46541F5"
				},
				{
					"8197a4a47f0425faeaa69deebc05ca29c0a5b5cc76ceacc0",
					"light rule cinnamon wrap drastic word pride squirrel upgrade"
					" then income fatal apart sustain crack supply proud access",
					"4CBDFF1CA2DB800FD61CAE72A57475FDC6BAB03E441FD63F96DABD1F183EF5B7"
					"82925F00105F318309A7E9C3EA6967C7801E46C8A58082674C860A37B93EDA02"
				},
				{
					"066dca1a2bb7e8a1db2832148ce9933eea0f3ac9548d793112d9a95c9407efad",
					"all hour make first leader extend hole alien behind guard gospel lava"
					" path output census museum junior mass reopen famous sing advance salt reform",
					"26E975EC644423F4A4C4F4215EF09B4BD7EF924E85D1D17C4CF3F136C2863CF6"
					"DF0A475045652C57EB5FB41513CA2A2D67722B77E954B4B3FC11F7590449191D"
				},
				{
					"f30f8c1da665478f49b001d94c5fc452",
					"vessel ladder alter error federal sibling chat ability sun glass valve picture",
					"2AAA9242DAAFCEE6AA9D7269F17D4EFE271E1B9A529178D7DC139CD18747090B"
					"F9D60295D0CE74309A78852A9CAADF0AF48AAE1C6253839624076224374BC63F"
				},
				{
					"c10ec20dc3cd9f652c7fac2f1230f7a3c828389a14392f05",
					"scissors invite lock maple supreme raw rapid void congress"
					" muscle digital elegant little brisk hair mango congress clump",
					"7B4A10BE9D98E6CBA265566DB7F136718E1398C71CB581E1B2F464CAC1CEEDF4"
					"F3E274DC270003C670AD8D02C4558B2F8E39EDEA2775C9E232C7CB798B069E88"
				},
				{
					"f585c11aec520db57dd353c69554b21a89b20fb0650966fa0a9d6f74fd989d8f",
					"void come effort suffer camp survey warrior heavy shoot primary clutch crush"
					" open amazing screen patrol group space point ten exist slush involve unfold",
					"01F5BCED59DEC48E362F2C45B5DE68B9FD6C92C6634F44D6D40AAB69056506F0"
					"E35524A518034DDC1192E1DACD32C1ED3EAA3C3B131C88ED8E7E54C49A5D0998"
				}
			};
		}

		// endregion
	}

	// region tests

	TEST(TEST_CLASS, CannotConvertEntropyToMnemonicWhenEntropySizeIsInvalid) {
		// Arrange:
		for (auto size : std::initializer_list<size_t>{ 12, 15, 17, 31, 33, 36 }) {
			auto entropy = test::GenerateRandomVector(size);

			// Act + Assert:
			EXPECT_THROW(Bip39EntropyToMnemonic(entropy), catapult_invalid_argument);
		}
	}

	TEST(TEST_CLASS, CanConvertEntropyToMnemonic) {
		// Arrange:
		auto i = 0u;
		for (const auto& testVector : GetTestVectorsBip39()) {
			// Act:
			auto mnemonic = Bip39EntropyToMnemonic(test::HexStringToVector(testVector.Entroy));

			// Assert:
			EXPECT_EQ(testVector.Mnemonic, mnemonic) << "test vector at " << i;
			++i;
		}
	}

	TEST(TEST_CLASS, CanConvertMnemonicToSeed) {
		// Arrange:
		auto i = 0u;
		for (const auto& testVector : GetTestVectorsBip39()) {
			// Act:
			auto seed = Bip39MnemonicToSeed(testVector.Mnemonic, "TREZOR");

			// Assert:
			EXPECT_EQ(utils::ParseByteArray<Hash512>(testVector.Seed), seed) << "test vector at " << i;
			++i;
		}
	}

	// endregion
}}
