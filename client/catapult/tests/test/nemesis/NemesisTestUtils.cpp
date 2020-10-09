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

#include "NemesisTestUtils.h"
#include "plugins/txes/mosaic/src/cache/MosaicCache.h"
#include "plugins/txes/namespace/src/cache/NamespaceCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/test/nodeps/TestNetworkConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;
	}

	Key RawPrivateKeyToPublicKey(const char* privateKeyString) {
		auto keyPair = crypto::KeyPair::FromString(privateKeyString);
		return keyPair.publicKey();
	}

	Address RawPublicKeyToAddress(const char* publicKeyString) {
		return model::PublicKeyToAddress(utils::ParseByteArray<Key>(publicKeyString), Network_Identifier);
	}

	Address RawPrivateKeyToAddress(const char* privateKeyString) {
		return model::PublicKeyToAddress(RawPrivateKeyToPublicKey(privateKeyString), Network_Identifier);
	}

	namespace {
		void AssertNemesisAccount(const cache::AccountStateCacheView& view) {
			auto nemesisKeyPair = crypto::KeyPair::FromString(Test_Network_Nemesis_Private_Key);
			auto address = model::PublicKeyToAddress(nemesisKeyPair.publicKey(), Network_Identifier);
			auto accountStateIter = view.find(address);
			const auto& accountState = accountStateIter.get();

			// Assert:
			EXPECT_EQ(Height(1), accountState.AddressHeight);
			EXPECT_EQ(address, accountState.Address);
			EXPECT_EQ(Height(1), accountState.PublicKeyHeight);
			EXPECT_EQ(nemesisKeyPair.publicKey(), accountState.PublicKey);

			EXPECT_EQ(Amount(0), accountState.Balances.get(Default_Currency_Mosaic_Id));
			EXPECT_EQ(Amount(0), accountState.Balances.get(Default_Harvesting_Mosaic_Id));

			EXPECT_EQ(model::ImportanceHeight(0), accountState.ImportanceSnapshots.height());
			EXPECT_EQ(Importance(0), accountState.ImportanceSnapshots.current());
		}

		void AssertRentalFeeAccount(const cache::AccountStateCacheView& view, const Address& address) {
			auto message = model::AddressToString(address);
			auto accountStateIter = view.find(address);
			const auto& accountState = accountStateIter.get();

			// Assert:
			EXPECT_EQ(Height(1), accountState.AddressHeight) << message;
			EXPECT_EQ(address, accountState.Address) << message;
			EXPECT_EQ(Height(0), accountState.PublicKeyHeight) << message;
			EXPECT_EQ(Key(), accountState.PublicKey) << message;

			EXPECT_EQ(Amount(0), accountState.Balances.get(Default_Currency_Mosaic_Id)) << message;
			EXPECT_EQ(Amount(0), accountState.Balances.get(Default_Harvesting_Mosaic_Id)) << message;

			EXPECT_EQ(model::ImportanceHeight(0), accountState.ImportanceSnapshots.height()) << message;
			EXPECT_EQ(Importance(0), accountState.ImportanceSnapshots.current()) << message;
		}

		void AssertRecipientAccount(const cache::AccountStateCacheView& view, const Key& publicKey, size_t index) {
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
			auto message = model::AddressToString(address);
			auto accountStateIter = view.find(address);
			const auto& accountState = accountStateIter.get();

			// Assert:
			EXPECT_EQ(Height(1), accountState.AddressHeight) << message;
			EXPECT_EQ(address, accountState.Address) << message;

			if (index < CountOf(Test_Network_Vrf_Private_Keys)) {
				EXPECT_EQ(Height(1), accountState.PublicKeyHeight) << message;
				EXPECT_EQ(publicKey, accountState.PublicKey) << message;

				auto expectedVrfPublicKey = crypto::KeyPair::FromString(Test_Network_Vrf_Private_Keys[index]).publicKey();
				EXPECT_EQ(state::AccountPublicKeys::KeyType::VRF, accountState.SupplementalPublicKeys.mask());
				EXPECT_EQ(expectedVrfPublicKey, GetVrfPublicKey(accountState));
			} else {
				// recipient public key is unknown (public key height is zero)
				EXPECT_EQ(Height(0), accountState.PublicKeyHeight) << message;
				EXPECT_EQ(Key(), accountState.PublicKey) << message;

				EXPECT_EQ(state::AccountPublicKeys::KeyType::Unset, accountState.SupplementalPublicKeys.mask());
			}

			auto expectedImportance = GetNemesisImportance(publicKey);
			EXPECT_EQ(Amount(409'090'909'000'000), accountState.Balances.get(Default_Currency_Mosaic_Id)) << message;
			EXPECT_EQ(Amount(expectedImportance.unwrap() * 1000), accountState.Balances.get(Default_Harvesting_Mosaic_Id)) << message;

			if (expectedImportance > Importance(0)) {
				EXPECT_EQ(model::ImportanceHeight(1), accountState.ImportanceSnapshots.height()) << message;
				EXPECT_EQ(expectedImportance, accountState.ImportanceSnapshots.current()) << message;
			} else {
				EXPECT_EQ(model::ImportanceHeight(0), accountState.ImportanceSnapshots.height()) << message;
			}
		}

		void AssertNemesisState(const cache::AccountStateCacheView& view) {
			// Assert:
			EXPECT_EQ(3u + CountOf(Test_Network_Private_Keys), view.size());

			// - check nemesis account
			AssertNemesisAccount(view);

			// - check rental fee accounts
			AssertRentalFeeAccount(view, model::StringToAddress(Namespace_Rental_Fee_Sink_Address));
			AssertRentalFeeAccount(view, model::StringToAddress(Mosaic_Rental_Fee_Sink_Address));

			// - check recipient accounts
			auto index = 0u;
			for (const auto* pRecipientPrivateKeyString : Test_Network_Private_Keys)
				AssertRecipientAccount(view, RawPrivateKeyToPublicKey(pRecipientPrivateKeyString), index++);
		}
	}

	void AssertNemesisAccountState(const cache::CatapultCacheView& view) {
		AssertNemesisState(view.sub<cache::AccountStateCache>());
	}

	namespace {
		void AssertNemesisState(const cache::MosaicCacheView& view) {
			// Assert:
			EXPECT_EQ(2u, view.size());

			// - check for known mosaics
			ASSERT_TRUE(view.contains(Default_Currency_Mosaic_Id));
			EXPECT_EQ(Amount(8'999'999'998'000'000), view.find(Default_Currency_Mosaic_Id).get().supply());

			ASSERT_TRUE(view.contains(Default_Harvesting_Mosaic_Id));
			EXPECT_EQ(Amount(17'000'000), view.find(Default_Harvesting_Mosaic_Id).get().supply());
		}
	}

	void AssertNemesisMosaicState(const cache::CatapultCacheView& view) {
		AssertNemesisState(view.sub<cache::MosaicCache>());
	}

	namespace {
		void AssertNemesisState(const cache::NamespaceCacheView& view) {
			// Assert:
			EXPECT_EQ(1u, view.size());

			// - check for known namespaces
			EXPECT_TRUE(view.contains(NamespaceId(Default_Namespace_Id)));
		}
	}

	void AssertNemesisNamespaceState(const cache::CatapultCacheView& view) {
		AssertNemesisState(view.sub<cache::NamespaceCache>());
	}
}}
