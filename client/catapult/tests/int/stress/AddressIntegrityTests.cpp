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
			static constexpr auto Expected_Nemesis_Address = "SARNAMOK7M4ZHNL75ZNNYC6UAM5EGH57UEAQX5I";

			static std::vector<const char*> PrivateKeys() {
				return std::vector<const char*>(&test::Mijin_Test_Private_Keys[0], &test::Mijin_Test_Private_Keys[11]);
			}

			static std::vector<std::string> ExpectedAddresses() {
				return {
					"SAAA26Z5QXYHDIVO2EYSEF7BUMC7FYQPBQSPHQY",
					"SAAA3ZE57CKRZY762QG4OG3RMM2EEAN6SLZ6KDQ",
					"SAAA4RUOJ2RBEKWYOZHSMZWTDGHMCRDUCNAKNFA",
					"SAAA6NI72BJ4FEE24JNVTSQRZHXGZCZCJ6NGNTY",
					"SAAA7O53WQULXNOK2GTP7QURX7RKPRDNF5UMC4I",
					"SAAAC7XL2E6XC2NCSXI4C6MG7HWZGE47WHNIQGY",
					"SAAADPWJ27OZVPSPMWCX5QQWWC47IF6KUFQTWCY",
					"SAAAEBW5QTJAYLWOYUZDJO7BJIZCIFRP3H25Y5A",
					"SAAAFAXT2EHGLAOXOR6I3JXR27ED7Q3LEHIVPMY",
					"SAAAG6WJ6IHREIOJN4U5H6GV5HAQLWU7SMEVWFQ",
					"SAAAHTKTAW4NLGLXU7U2ZWP5LWQ4Y7DGYWNGKXQ"
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
