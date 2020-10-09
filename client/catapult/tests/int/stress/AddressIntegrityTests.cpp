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

#include "catapult/model/Address.h"
#include "catapult/model/NetworkIdentifier.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/TestNetworkConstants.h"
#include "tests/TestHarness.h"

namespace catapult {

#define TEST_CLASS AddressIntegrityTests

	TEST(TEST_CLASS, CanFindAddressStartingWithPA) {
		// Arrange:
		for (auto i = 0u; i < 1000; ++i) {
			auto kp = test::GenerateKeyPair();
			auto rawAddress = model::PublicKeyToAddress(kp.publicKey(), model::NetworkIdentifier::Private);
			auto address = model::AddressToString(rawAddress);
			if (address[0] == 'P' && address[1] == 'A')
				return;
		}

		FAIL() << "could not generate PA* address";
	}

	namespace {
		struct PrivateTestNetworkTraits {
			static constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;
			static constexpr auto Nemesis_Private_Key = test::Test_Network_Nemesis_Private_Key;
			static constexpr auto Expected_Nemesis_Address = "QARNAMOK7M4ZHNL75ZNNYC6UAM5EGH57UELLDZA";

			static std::vector<const char*> PrivateKeys() {
				return std::vector<const char*>(&test::Test_Network_Private_Keys[0], &test::Test_Network_Private_Keys[11]);
			}

			static std::vector<std::string> ExpectedAddresses() {
				return {
					"QAAA26Z5QXYHDIVO2EYSEF7BUMC7FYQPBTENLAQ",
					"QAAA3ZE57CKRZY762QG4OG3RMM2EEAN6SKDKLEY",
					"QAAA4RUOJ2RBEKWYOZHSMZWTDGHMCRDUCORP2ZY",
					"QAAA6NI72BJ4FEE24JNVTSQRZHXGZCZCJ5D455I",
					"QAAA7O53WQULXNOK2GTP7QURX7RKPRDNF4GGGEY",
					"QAAAC7XL2E6XC2NCSXI4C6MG7HWZGE47WG2YLNI",
					"QAAADPWJ27OZVPSPMWCX5QQWWC47IF6KUEYAYVY",
					"QAAAEBW5QTJAYLWOYUZDJO7BJIZCIFRP3HMHZQQ",
					"QAAAFAXT2EHGLAOXOR6I3JXR27ED7Q3LEFOERXA",
					"QAAAG6WJ6IHREIOJN4U5H6GV5HAQLWU7SMNPSBI",
					"QAAAHTKTAW4NLGLXU7U2ZWP5LWQ4Y7DGYV2AT7Y"
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
	TEST(TEST_CLASS, TEST_NAME##_Private_Test) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PrivateTestNetworkTraits>(); } \
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
