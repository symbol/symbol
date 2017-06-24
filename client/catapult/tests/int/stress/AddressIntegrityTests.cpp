#include "catapult/crypto/KeyGenerator.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/model/Address.h"
#include "catapult/model/NetworkInfo.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"

using namespace catapult::crypto;

namespace catapult {

#define TEST_CLASS AddressIntegrityTests

	TEST(TEST_CLASS, CanFindAddressStartingWithMA) {
		// Arrange:
		for (auto i = 0u; i < 1000; ++i) {
			auto sk = PrivateKey::Generate(test::RandomByte);
			auto kp = KeyPair::FromPrivate(std::move(sk));
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
			static auto NemesisPrivateKey() { return test::Mijin_Test_Nemesis_Private_Key; }
			static std::string ExpectedNemesisAddress() { return "SARNASAS2BIAB6LMFA3FPMGBPGIJGK6IJETM3ZSP"; }

			static std::vector<const char*> PrivateKeys() {
				return std::vector<const char*>(&test::Mijin_Test_Private_Keys[0], &test::Mijin_Test_Private_Keys[11]);
			}

			static std::vector<std::string> ExpectedAddresses() {
				return {
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
				};
			}
		};

		auto PrivateKeyStringToAddressString(const std::string& pkString, model::NetworkIdentifier networkIdentifier) {
			auto kp = KeyPair::FromString(pkString);
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
