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
#include "plugins/txes/namespace/src/cache/MosaicCache.h"
#include "plugins/txes/namespace/src/cache/NamespaceCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/model/Address.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
		constexpr Amount Nemesis_Recipient_Amount(409'090'909'000'000);
		constexpr Importance Nemesis_Recipient_Importance(409'090'909);
	}

	Address RawPrivateKeyToAddress(const char* privateKeyString) {
		auto keyPair = crypto::KeyPair::FromString(privateKeyString);
		return model::PublicKeyToAddress(keyPair.publicKey(), Network_Identifier);
	}

	Address RawPublicKeyToAddress(const char* publicKeyString) {
		return model::PublicKeyToAddress(crypto::ParseKey(publicKeyString), Network_Identifier);
	}

	namespace {
		void AssertNemesisAccount(const cache::AccountStateCacheView& view) {
			auto nemesisKeyPair = crypto::KeyPair::FromString(test::Mijin_Test_Nemesis_Private_Key);
			auto address = model::PublicKeyToAddress(nemesisKeyPair.publicKey(), Network_Identifier);
			const auto& accountState = view.get(address);

			// Assert:
			EXPECT_EQ(Height(1), accountState.AddressHeight);
			EXPECT_EQ(address, accountState.Address);
			EXPECT_EQ(Height(1), accountState.PublicKeyHeight);
			EXPECT_EQ(nemesisKeyPair.publicKey(), accountState.PublicKey);

			EXPECT_EQ(Amount(0), accountState.Balances.get(Xem_Id));

			EXPECT_EQ(model::ImportanceHeight(0), accountState.ImportanceInfo.height());
			EXPECT_EQ(Importance(0), accountState.ImportanceInfo.current());
		}

		void AssertRentalFeeAccount(const cache::AccountStateCacheView& view, const Key& publicKey) {
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
			auto message = model::AddressToString(address);
			const auto& accountState = view.get(address);

			// Assert:
			EXPECT_EQ(Height(1), accountState.AddressHeight) << message;
			EXPECT_EQ(address, accountState.Address) << message;
			EXPECT_EQ(Height(1), accountState.PublicKeyHeight) << message;
			EXPECT_EQ(publicKey, accountState.PublicKey) << message;

			EXPECT_EQ(Amount(0), accountState.Balances.get(Xem_Id)) << message;

			EXPECT_EQ(model::ImportanceHeight(0), accountState.ImportanceInfo.height()) << message;
			EXPECT_EQ(Importance(0), accountState.ImportanceInfo.current()) << message;
		}

		void AssertRecipientAccount(const cache::AccountStateCacheView& view, const Address& address) {
			auto message = model::AddressToString(address);
			const auto& accountState = view.get(address);

			// Assert:
			EXPECT_EQ(Height(1), accountState.AddressHeight) << message;
			EXPECT_EQ(address, accountState.Address) << message;
			EXPECT_EQ(Height(0), accountState.PublicKeyHeight) << message;
			// recipient public key is unknown (public key height is zero)

			EXPECT_EQ(Nemesis_Recipient_Amount, accountState.Balances.get(Xem_Id)) << message;

			EXPECT_EQ(model::ImportanceHeight(1), accountState.ImportanceInfo.height()) << message;
			EXPECT_EQ(Nemesis_Recipient_Importance, accountState.ImportanceInfo.current()) << message;
		}

		void AssertNemesisState(const cache::AccountStateCacheView& view) {
			// Assert:
			EXPECT_EQ(3u + CountOf(test::Mijin_Test_Private_Keys), view.size());

			// - check nemesis account
			AssertNemesisAccount(view);

			// - check rental fee accounts
			AssertRentalFeeAccount(view, crypto::ParseKey(Namespace_Rental_Fee_Sink_Public_Key));
			AssertRentalFeeAccount(view, crypto::ParseKey(Mosaic_Rental_Fee_Sink_Public_Key));

			// - check recipient accounts
			for (const auto* pRecipientPrivateKeyString : test::Mijin_Test_Private_Keys)
				AssertRecipientAccount(view, RawPrivateKeyToAddress(pRecipientPrivateKeyString));
		}
	}

	void AssertNemesisAccountState(const cache::CatapultCacheView& view) {
		AssertNemesisState(view.sub<cache::AccountStateCache>());
	}

	namespace {
		void AssertNemesisState(const cache::MosaicCacheView& view) {
			// Assert:
			EXPECT_EQ(1u, view.size());

			// - check for known mosaics
			ASSERT_TRUE(view.contains(Xem_Id));
			EXPECT_EQ(Amount(8'999'999'998'000'000), view.get(Xem_Id).supply());
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
			EXPECT_TRUE(view.contains(Nem_Id));
		}
	}

	void AssertNemesisNamespaceState(const cache::CatapultCacheView& view) {
		AssertNemesisState(view.sub<cache::NamespaceCache>());
	}
}}
