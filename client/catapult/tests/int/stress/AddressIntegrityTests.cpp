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

#include "catapult/crypto/KeyGenerator.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/model/Address.h"
#include "catapult/model/NetworkInfo.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"

namespace catapult {

#define TEST_CLASS AddressIntegrityTests

	TEST(TEST_CLASS, CanFindAddressStartingWithMA) {
		// Arrange:
		for (auto i = 0u; i < 1000; ++i) {
			auto sk = crypto::PrivateKey::Generate(test::RandomByte);
			auto kp = crypto::KeyPair::FromPrivate(std::move(sk));
			auto rawAddress = model::PublicKeyToAddress(kp.publicKey(), model::NetworkIdentifier::Mijin);
			auto address = model::AddressToString(rawAddress);
			if (address[0] == 'M' && address[1] == 'A')
				return;
		}

		FAIL() << "could not generate MA* address";
	}

	namespace {
		struct MijinTestNetworkTraits {
			static const auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
			static constexpr auto NemesisPrivateKey() { return test::Mijin_Test_Nemesis_Private_Key; }
			static constexpr auto ExpectedNemesisAddress() {
#ifdef SIGNATURE_SCHEME_NIS1
				return "SDXZPM4GUCEYAMMJH6QRPTRMEZ4JIXUU5W276BYK";
#else
				return "SARNASAS2BIAB6LMFA3FPMGBPGIJGK6IJETM3ZSP";
#endif
			}

			static std::vector<const char*> PrivateKeys() {
				return std::vector<const char*>(&test::Mijin_Test_Private_Keys[0], &test::Mijin_Test_Private_Keys[11]);
			}

			static std::vector<std::string> ExpectedAddresses() {
				return {
#ifdef SIGNATURE_SCHEME_NIS1
					"SCA6I5AP4X4B3U4GTO2FU7SJD4UDX37I6SEVR3GP",
					"SCHC72JZZSO3OKXIB6TSOVA4BTT6A2CRXMVKOJAV",
					"SBJMEHK43TA4GYFDK6WZDYZEIDNTI7UVFLA4ISCQ",
					"SCTMMUHWPBQSVQGHWOZBOHWM3QRNU6KZRAPS3TDP",
					"SAZGBECYA55CDZV5S2Q32CKABW7ZCVNIXCJWQVQC",
					"SDMZAQJZJ7WCFUCOJIKXAAXVEMDBLXFLSW2HVX2F",
					"SBTZ7EHSKCXDIO2TUEMD4JGSZ3SBAPLNV5ZMO25O",
					"SCPLDDFMD4KKNHRBTA774MYW63XCP5QZIKIY2LKJ",
					"SAGCA236SGPMW2HWU2EBJSHQ74SDZ6OB7W35SMVN",
					"SDLCP3E4V6S3DXV3C4AKNQOZITH5NLXK7QGNAFTT",
					"SAOFFTLCERCRXWMQ6B3253D7QNYVK33PGPYQBAI4"
#else
					"SAAA244WMCB2JXGNQTQHQOS45TGBFF4V2MJBVOUI",
					"SAAA34PEDKJHKIHGVXV3BSKBSQPPQDDMO2ATWMY3",
					"SAAA467G4ZDNOEGLNXLGWUAXZKC6VAES74J7N34D",
					"SAAA57DREOPYKUFX4OG7IQXKITMBWKD6KXTVBBQP",
					"SAAA66EEZKK3HGBRV57E6TOK335NK22BF2KGOEDS",
					"SAAAIBC7AM65HOFDLYGFUT46H44TROZ7MUWCW6MZ",
					"SAAAJ5BYWZI5J3ASQEKCUV6JSPKIVYBCIAKS4ECB",
					"SAAAK7HILTOL6YHC3HXQTGACIWIJKD65CMGK7B5W",
					"SAAAL4JPUQLKCWRRWMXQT3T2F3GMHIG4RUKQB24N",
					"SAAAMZYSPE5TRAVH7I3VSF7ZD542EVDLB7JT7Z4K",
					"SAAAZY5C3L6ONXRAPH2WYAPC3FKYFIPBBPFMLAS4"
#endif
				};
			}
		};

		auto PrivateKeyStringToAddressString(const std::string& pkString, model::NetworkIdentifier networkIdentifier) {
			auto kp = crypto::KeyPair::FromString(pkString);
			auto rawAddress = model::PublicKeyToAddress(kp.publicKey(), networkIdentifier);
			return model::AddressToString(rawAddress);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Mijin_Test) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MijinTestNetworkTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(NemesisKeyProducesExpectedAddress) {
		// Act:
		auto address = PrivateKeyStringToAddressString(TTraits::NemesisPrivateKey(), TTraits::Network_Identifier);

		// Assert:
		EXPECT_EQ(TTraits::ExpectedNemesisAddress(), address);
	}

	TRAITS_BASED_TEST(PrivateKeysProduceExpectedAddresses) {
		// Arrange:
		auto expectedAddresses = TTraits::ExpectedAddresses();

		// Act + Assert:
		auto i = 0u;
		for (const auto& pkString : TTraits::PrivateKeys()) {
			const auto& expectedAddress = expectedAddresses[i++];
			EXPECT_EQ(expectedAddress, PrivateKeyStringToAddressString(pkString, TTraits::Network_Identifier));
		}
	}
}
