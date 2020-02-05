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
#include "catapult/model/Address.h"
#include "catapult/model/NetworkIdentifier.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"

namespace catapult {

#define TEST_CLASS AddressIntegrityTests

	TEST(TEST_CLASS, CanFindAddressStartingWithMA) {
		// Arrange:
		for (auto i = 0u; i < 1000; ++i) {
			auto kp = test::GenerateKeyPair();
			auto rawAddress = model::PublicKeyToAddress(kp.publicKey(), model::NetworkIdentifier::Mijin);
			auto address = model::AddressToString(rawAddress);
			if (address[0] == 'M' && address[1] == 'A')
				return;
		}

		FAIL() << "could not generate MA* address";
	}

	namespace {
		struct MijinTestNetworkTraits {
			static constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
			static constexpr auto Nemesis_Private_Key = test::Mijin_Test_Nemesis_Private_Key;
#ifdef SIGNATURE_SCHEME_KECCAK
			static constexpr auto Expected_Nemesis_Address = "SAPO6C4QJBWWFZAZAKH55IDCIDEQX6AVRFV5M7Q3";
#else
			static constexpr auto Expected_Nemesis_Address = "SARNASAS2BIAB6LMFA3FPMGBPGIJGK6IJETM3ZSP";
#endif

			static std::vector<const char*> PrivateKeys() {
				return std::vector<const char*>(&test::Mijin_Test_Private_Keys[0], &test::Mijin_Test_Private_Keys[11]);
			}

			static std::vector<std::string> ExpectedAddresses() {
				return {
#ifdef SIGNATURE_SCHEME_KECCAK
					"SCKPJHW4DQEQE6ALBRPUXL7CCOEYC6KRSTM27AXP",
					"SDOFLGBMIJDJSIGSPDIXDMHUGL32RYCRSBQOHPP4",
					"SBFLOD5YUZPAU7TVWKCHHOBGNX7C7IXJY37I3R6M",
					"SBTSWV7IMBFZPR4ZHIKVVE7NTIJGGVZNX36J2VD2",
					"SBDN4JD3NKFYNMJ6TLZYHSZNMQMOJI6556CIQ3QO",
					"SD75TQ4OEKUVSJYSMH4JO3PJFIZNOZAVJHY3FFD5",
					"SBGPI2WK76YU5IUXH6BIYGNRVQR25FYQRZ42QRUD",
					"SDOGWQZAZH6QTPNKOU562HJRAOPJ3PBYQ65NY2BM",
					"SDPRTL47A7Z55KIBJMLV7UK2BPA66UJTCYRCKW3J",
					"SBKII3BJITHFOSZWZA6EKSSINLPW7SI6OF5FFZKE",
					"SCR3NI7J3FNPY3KFUTXMDFL23DNF23BGTMTZLWCI"
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
		auto address = PrivateKeyStringToAddressString(TTraits::Nemesis_Private_Key, TTraits::Network_Identifier);

		// Assert:
		EXPECT_EQ(TTraits::Expected_Nemesis_Address, address);
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
