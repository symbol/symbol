#include "catapult/local/p2p/UnlockedAccountsUtils.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/chain/UnlockedAccounts.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/ImportanceHeight.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace p2p {

	namespace {
		constexpr Amount Account_Balance(1000);
		constexpr auto Importance_Grouping = 234u;

		auto ConvertToImportanceHeight(Height height) {
			return model::ConvertToImportanceHeight(height, Importance_Grouping);
		}

		auto CreateCacheWithAccount(Height height, const Key& publicKey, model::ImportanceHeight importanceHeight) {
			// Arrange:
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.ImportanceGrouping = Importance_Grouping;
			auto cache = test::CreateEmptyCatapultCache(config);
			auto delta = cache.createDelta();

			// - add an account
			auto pState = delta.sub<cache::AccountStateCache>().addAccount(publicKey, Height(100));
			pState->ImportanceInfo.set(Importance(123), importanceHeight);
			pState->Balances.credit(Xem_Id, Account_Balance);

			// - commit changes
			cache.commit(height);
			return cache;
		}
	}

	TEST(UnlockedAccountsUtilsTests, PruneUnlockedAccounts_DoesNotPruneEligibleAccount) {
		// Arrange: eligible because next height and importance height match
		auto height = Height(2 * Importance_Grouping - 1);
		auto importanceHeight = model::ImportanceHeight(Importance_Grouping);
		auto keyPair = test::GenerateKeyPair();
		auto cache = CreateCacheWithAccount(height, keyPair.publicKey(), importanceHeight);

		chain::UnlockedAccounts unlockedAccounts(3);
		unlockedAccounts.modifier().add(std::move(keyPair));

		// Act:
		PruneUnlockedAccounts(unlockedAccounts, cache, Account_Balance);

		// Assert:
		EXPECT_EQ(1u, unlockedAccounts.view().size());

		// Sanity:
		EXPECT_EQ(importanceHeight, ConvertToImportanceHeight(height + Height(1)));
	}

	TEST(UnlockedAccountsUtilsTests, PruneUnlockedAccounts_DoesPruneAccountIneligibleDueToImportanceHeight) {
		// Arrange: ineligible because next height and importance height do not match
		auto height = Height(2 * Importance_Grouping);
		auto importanceHeight = model::ImportanceHeight(Importance_Grouping);
		auto keyPair = test::GenerateKeyPair();
		auto cache = CreateCacheWithAccount(height, keyPair.publicKey(), importanceHeight);

		chain::UnlockedAccounts unlockedAccounts(3);
		unlockedAccounts.modifier().add(std::move(keyPair));

		// Act:
		PruneUnlockedAccounts(unlockedAccounts, cache, Account_Balance);

		// Assert:
		EXPECT_EQ(0u, unlockedAccounts.view().size());

		// Sanity:
		EXPECT_NE(importanceHeight, ConvertToImportanceHeight(height + Height(1)));
	}

	TEST(UnlockedAccountsUtilsTests, PruneUnlockedAccounts_DoesPruneAccountIneligibleDueToBalance) {
		// Arrange: ineligible because account balance is too low
		auto height = Height(2 * Importance_Grouping - 1);
		auto importanceHeight = model::ImportanceHeight(Importance_Grouping);
		auto keyPair = test::GenerateKeyPair();
		auto cache = CreateCacheWithAccount(height, keyPair.publicKey(), importanceHeight);

		chain::UnlockedAccounts unlockedAccounts(3);
		unlockedAccounts.modifier().add(std::move(keyPair));

		// Act:
		PruneUnlockedAccounts(unlockedAccounts, cache, Account_Balance + Amount(1));

		// Assert:
		EXPECT_EQ(0u, unlockedAccounts.view().size());

		// Sanity:
		EXPECT_EQ(importanceHeight, ConvertToImportanceHeight(height + Height(1)));
	}
}}}
