#include "NemesisTestUtils.h"
#include "plugins/txes/namespace/src/cache/MosaicCache.h"
#include "plugins/txes/namespace/src/cache/NamespaceCache.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/model/Address.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
		constexpr Amount Nemesis_Recipient_Amount(818'181'818'000'000);
		constexpr Importance Nemesis_Recipient_Importance(818'181'818);
	}

	Address RawPrivateKeyToAddress(const char* pPrivateKeyString) {
		auto keyPair = crypto::KeyPair::FromString(pPrivateKeyString);
		return model::PublicKeyToAddress(keyPair.publicKey(), Network_Identifier);
	}

	Address RawPublicKeyToAddress(const char* pPublicKeyString) {
		return model::PublicKeyToAddress(crypto::ParseKey(pPublicKeyString), Network_Identifier);
	}

	namespace {
		void AssertNemesisAccount(const cache::AccountStateCacheView& view) {
			auto nemesisKeyPair = crypto::KeyPair::FromString(test::Mijin_Test_Nemesis_Private_Key);
			auto address = model::PublicKeyToAddress(nemesisKeyPair.publicKey(), Network_Identifier);
			auto pAccountState = view.findAccount(address);

			// Assert:
			ASSERT_TRUE(!!pAccountState);
			EXPECT_EQ(Height(1), pAccountState->AddressHeight);
			EXPECT_EQ(address, pAccountState->Address);
			EXPECT_EQ(Height(1), pAccountState->PublicKeyHeight);
			EXPECT_EQ(nemesisKeyPair.publicKey(), pAccountState->PublicKey);

			EXPECT_EQ(Amount(0), pAccountState->Balances.get(Xem_Id));

			EXPECT_EQ(model::ImportanceHeight(0), pAccountState->ImportanceInfo.height());
			EXPECT_EQ(Importance(0), pAccountState->ImportanceInfo.current());
		}

		void AssertRentalFeeAccount(const cache::AccountStateCacheView& view, const Key& publicKey) {
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
			auto message = model::AddressToString(address);
			auto pAccountState = view.findAccount(address);

			// Assert:
			ASSERT_TRUE(!!pAccountState) << message;
			EXPECT_EQ(Height(1), pAccountState->AddressHeight) << message;
			EXPECT_EQ(address, pAccountState->Address) << message;
			EXPECT_EQ(Height(1), pAccountState->PublicKeyHeight) << message;
			EXPECT_EQ(publicKey, pAccountState->PublicKey) << message;

			EXPECT_EQ(Amount(0), pAccountState->Balances.get(Xem_Id)) << message;

			EXPECT_EQ(model::ImportanceHeight(0), pAccountState->ImportanceInfo.height()) << message;
			EXPECT_EQ(Importance(0), pAccountState->ImportanceInfo.current()) << message;
		}

		void AssertRecipientAccount(const cache::AccountStateCacheView& view, const Address& address) {
			auto message = model::AddressToString(address);
			auto pAccountState = view.findAccount(address);

			// Assert:
			ASSERT_TRUE(!!pAccountState) << message;
			EXPECT_EQ(Height(1), pAccountState->AddressHeight) << message;
			EXPECT_EQ(address, pAccountState->Address) << message;
			EXPECT_EQ(Height(0), pAccountState->PublicKeyHeight) << message;
			// recipient public key is unknown (public key height is zero)

			EXPECT_EQ(Nemesis_Recipient_Amount, pAccountState->Balances.get(Xem_Id)) << message;

			EXPECT_EQ(model::ImportanceHeight(1), pAccountState->ImportanceInfo.height()) << message;
			EXPECT_EQ(Nemesis_Recipient_Importance, pAccountState->ImportanceInfo.current()) << message;
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
