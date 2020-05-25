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

#include "catapult/cache_core/ImportanceView.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/model/NetworkIdentifier.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/cache/ImportanceViewTestUtils.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/TestHarness.h"

using catapult::model::ImportanceHeight;
using catapult::model::ConvertToImportanceHeight;

namespace catapult { namespace cache {

#define TEST_CLASS ImportanceViewTests

	// region utils

	namespace {
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);
		constexpr auto Default_Cache_Options = test::CreateDefaultAccountStateCacheOptions(MosaicId(1111), Harvesting_Mosaic_Id);

		struct AddressTraits {
			static auto AddAccount(AccountStateCacheDelta& delta, const Key& publicKey, Height height) {
				auto address = model::PublicKeyToAddress(publicKey, Default_Cache_Options.NetworkIdentifier);
				delta.addAccount(address, height);
				return delta.find(address);
			}
		};

		struct PublicKeyTraits {
			static auto AddAccount(AccountStateCacheDelta& delta, const Key& publicKey, Height height) {
				delta.addAccount(publicKey, height);
				return delta.find(publicKey);
			}
		};

		struct MainAccountTraits {
			static auto AddAccount(AccountStateCacheDelta& delta, const Key& publicKey, Height height) {
				// explicitly mark the account as a main account (local harvesting when remote harvesting is enabled)
				auto accountStateIter = PublicKeyTraits::AddAccount(delta, publicKey, height);
				accountStateIter.get().AccountType = state::AccountType::Main;
				test::ForceSetLinkedPublicKey(accountStateIter.get(), test::GenerateRandomByteArray<Key>());
				return accountStateIter;
			}
		};

		struct RemoteAccountTraits {
			static auto AddAccount(AccountStateCacheDelta& delta, const Key& publicKey, Height height) {
				// 1. add the main account with a balance
				auto mainAccountPublicKey = test::GenerateRandomByteArray<Key>();
				auto mainAccountStateIter = PublicKeyTraits::AddAccount(delta, mainAccountPublicKey, height);
				mainAccountStateIter.get().AccountType = state::AccountType::Main;
				test::ForceSetLinkedPublicKey(mainAccountStateIter.get(), publicKey);

				// 2. add the remote account with specified key
				auto accountStateIter = PublicKeyTraits::AddAccount(delta, publicKey, height);
				accountStateIter.get().AccountType = state::AccountType::Remote;
				test::ForceSetLinkedPublicKey(accountStateIter.get(), mainAccountPublicKey);
				return mainAccountStateIter;
			}
		};

		template<typename TTraits>
		void AddAccount(
				AccountStateCache& cache,
				const Key& publicKey,
				Importance importance,
				ImportanceHeight importanceHeight,
				Amount balance = Amount(0)) {
			auto delta = cache.createDelta();
			auto accountStateIter = TTraits::AddAccount(*delta, publicKey, Height(100));
			auto& accountState = accountStateIter.get();
			accountState.ImportanceSnapshots.set(importance, importanceHeight);
			accountState.Balances.credit(Harvesting_Mosaic_Id, balance);
			cache.commit();
		}

		auto ConvertToImportanceHeight(Height height) {
			return model::ConvertToImportanceHeight(height, Default_Cache_Options.ImportanceGrouping);
		}

		auto CreateAccountStateCache(Amount minHarvesterBalance, Amount maxHarvesterBalance) {
			auto options = Default_Cache_Options;
			options.MinHarvesterBalance = minHarvesterBalance;
			options.MaxHarvesterBalance = maxHarvesterBalance;
			return std::make_unique<AccountStateCache>(CacheConfiguration(), options);
		}

		auto CreateAccountStateCache() {
			return CreateAccountStateCache(Amount(), Amount(std::numeric_limits<Amount::ValueType>::max()));
		}
	}

	// endregion

	// region tryGetAccountImportance / getAccountImportanceOrDefault

#define KEY_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_PublicKey) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PublicKeyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MainAccount) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MainAccountTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_RemoteAccount) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RemoteAccountTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		void AssertCannotFindImportance(const ImportanceView& view, const Key& key, Height height) {
			// Act:
			Importance importance;
			auto foundImportance = view.tryGetAccountImportance(key, height, importance);
			auto importanceOrDefault = view.getAccountImportanceOrDefault(key, height);

			// Assert:
			EXPECT_FALSE(foundImportance);
			EXPECT_EQ(Importance(0), importanceOrDefault);
		}
	}

	KEY_TRAITS_BASED_TEST(CannotRetrieveImportanceForUnknownAccount) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto height = Height(1000);
		auto pCache = CreateAccountStateCache();
		AddAccount<TTraits>(*pCache, key, Importance(1000), ConvertToImportanceHeight(height));
		auto pView = test::CreateImportanceView(*pCache);

		// Act + Assert: mismatched key
		AssertCannotFindImportance(*pView, test::GenerateRandomByteArray<Key>(), height);
	}

	KEY_TRAITS_BASED_TEST(CannotRetrieveImportanceForAccountAtMismatchedHeight) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto pCache = CreateAccountStateCache();
		AddAccount<TTraits>(*pCache, key, Importance(1000), ConvertToImportanceHeight(Height(10000)));
		auto pView = test::CreateImportanceView(*pCache);

		// Act + Assert: mismatched height
		AssertCannotFindImportance(*pView, key, Height(3333));
	}

	namespace {
		template<typename TTraits>
		void AssertCanFindImportance(Importance accountImportance) {
			// Arrange:
			auto key = test::GenerateRandomByteArray<Key>();
			auto height = Height(1000);
			auto pCache = CreateAccountStateCache();
			AddAccount<TTraits>(*pCache, key, accountImportance, ConvertToImportanceHeight(height));
			auto pView = test::CreateImportanceView(*pCache);

			// Act:
			Importance importance;
			auto foundImportance = pView->tryGetAccountImportance(key, height, importance);
			auto importanceOrDefault = pView->getAccountImportanceOrDefault(key, height);

			// Assert:
			EXPECT_TRUE(foundImportance);
			EXPECT_EQ(accountImportance, importance);
			EXPECT_EQ(accountImportance, importanceOrDefault);
		}
	}

	KEY_TRAITS_BASED_TEST(CanRetrieveZeroImportanceFromAccount) {
		AssertCanFindImportance<TTraits>(Importance(0));
	}

	KEY_TRAITS_BASED_TEST(CanRetrieveNonzeroImportanceFromAccount) {
		AssertCanFindImportance<TTraits>(Importance(1234));
	}

	// endregion

	// region canHarvest

	namespace {
		struct CanHarvestViaMemberTraits {
			static bool CanHarvest(const AccountStateCache& cache, const Key& publicKey, Height height) {
				auto pView = test::CreateImportanceView(cache);
				return pView->canHarvest(model::PublicKeyToAddress(publicKey, Default_Cache_Options.NetworkIdentifier), height);
			}
		};

		struct AddressCanHarvestViaMemberTraits : public AddressTraits, public CanHarvestViaMemberTraits {};
		struct PublicKeyCanHarvestViaMemberTraits : public PublicKeyTraits, public CanHarvestViaMemberTraits {};
		struct MainAccountCanHarvestViaMemberTraits : public MainAccountTraits, public CanHarvestViaMemberTraits {};
		struct RemoteAccountCanHarvestViaMemberTraits : public RemoteAccountTraits, public CanHarvestViaMemberTraits {};
	}

#define CAN_HARVEST_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressCanHarvestViaMemberTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_PublicKey) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PublicKeyCanHarvestViaMemberTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MainAccount) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MainAccountCanHarvestViaMemberTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_RemoteAccount) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RemoteAccountCanHarvestViaMemberTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CAN_HARVEST_TRAITS_BASED_TEST(CannotHarvestWhenAccountIsUnknown) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto height = Height(1000);
		auto pCache = CreateAccountStateCache(Amount(1234), Amount(9876));
		AddAccount<TTraits>(*pCache, key, Importance(1000), ConvertToImportanceHeight(height));

		// Act + Assert:
		EXPECT_FALSE(TTraits::CanHarvest(*pCache, test::GenerateRandomByteArray<Key>(), height));
	}

	namespace {
		template<typename TTraits>
		bool CanHarvest(int64_t minBalanceDelta, Importance importance, ImportanceHeight importanceHeight, Height testHeight) {
			// Arrange:
			auto key = test::GenerateRandomByteArray<Key>();
			auto pCache = CreateAccountStateCache(Amount(1234), Amount(9876));
			auto initialBalance = Amount(static_cast<Amount::ValueType>(1234 + minBalanceDelta));
			AddAccount<TTraits>(*pCache, key, importance, importanceHeight, initialBalance);

			// Act:
			return TTraits::CanHarvest(*pCache, key, testHeight);
		}

		template<typename TTraits>
		bool CanHarvest(int64_t minBalanceDelta, Importance importance) {
			auto height = Height(10000);
			return CanHarvest<TTraits>(minBalanceDelta, importance, ConvertToImportanceHeight(height), height);
		}
	}

	CAN_HARVEST_TRAITS_BASED_TEST(CannotHarvestWhenBalanceIsBelowMinBalance) {
		EXPECT_FALSE(CanHarvest<TTraits>(-1, Importance(123)));
		EXPECT_FALSE(CanHarvest<TTraits>(-100, Importance(123)));
	}

	CAN_HARVEST_TRAITS_BASED_TEST(CannotHarvestWhenBalanceIsAboveMaxBalance) {
		EXPECT_FALSE(CanHarvest<TTraits>(9876 - 1234 + 1, Importance(123)));
		EXPECT_FALSE(CanHarvest<TTraits>(12345, Importance(123)));
	}

	CAN_HARVEST_TRAITS_BASED_TEST(CannotHarvestWhenImportanceIsZero) {
		EXPECT_FALSE(CanHarvest<TTraits>(2345, Importance(0)));
	}

	CAN_HARVEST_TRAITS_BASED_TEST(CannotHarvestWhenImportanceIsNotSetAtCorrectHeight) {
		EXPECT_FALSE(CanHarvest<TTraits>(2345, Importance(0), ImportanceHeight(123), Height(1234)));
	}

	CAN_HARVEST_TRAITS_BASED_TEST(CanHarvestWhenAllCriteriaAreMet) {
		EXPECT_TRUE(CanHarvest<TTraits>(0, Importance(123)));
		EXPECT_TRUE(CanHarvest<TTraits>(1, Importance(123)));
		EXPECT_TRUE(CanHarvest<TTraits>(2345, Importance(123)));
		EXPECT_TRUE(CanHarvest<TTraits>(9876 - 1234, Importance(123)));
	}

	// endregion

	// region improper links

	namespace {
		struct TryGetTraits {
			static void Act(const ImportanceView& view, const Key& publicKey) {
				Importance importance;
				view.tryGetAccountImportance(publicKey, Height(111), importance);
			}
		};

		struct GetTraits {
			static void Act(const ImportanceView& view, const Key& publicKey) {
				view.getAccountImportanceOrDefault(publicKey, Height(111));
			}
		};

		struct CanHarvestTraits {
			static void Act(const ImportanceView& view, const Key& publicKey) {
				view.canHarvest(model::PublicKeyToAddress(publicKey, Default_Cache_Options.NetworkIdentifier), Height(111));
			}
		};
	}

#define IMPROPER_LINKS_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_TryGet) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TryGetTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Get) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<GetTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_CanHarvest) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CanHarvestTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		template<typename TTraits, typename TMutator>
		void AssertImproperLink(TMutator mutator) {
			// Arrange:
			auto publicKey = test::GenerateRandomByteArray<Key>();
			auto pCache = CreateAccountStateCache();

			{
				auto delta = pCache->createDelta();
				auto accountStateIter = RemoteAccountTraits::AddAccount(*delta, publicKey, Height(100));
				mutator(accountStateIter.get());
				pCache->commit();
			}

			auto pView = test::CreateImportanceView(*pCache);

			// Act + Assert:
			EXPECT_THROW(TTraits::Act(*pView, publicKey), catapult_runtime_error);
		}
	}

	IMPROPER_LINKS_TRAITS_BASED_TEST(FailureWhenLinkedAccountHasWrongType) {
		AssertImproperLink<TTraits>([](auto& accountState) {
			// Arrange: change the main account to have the wrong type
			accountState.AccountType = state::AccountType::Remote;
		});
	}

	IMPROPER_LINKS_TRAITS_BASED_TEST(FailureWhenLinkedAccountDoesNotReferenceRemoteAccount) {
		AssertImproperLink<TTraits>([](auto& accountState) {
			// Arrange: change the main account to point to a different account
			test::ForceSetLinkedPublicKey(accountState, test::GenerateRandomByteArray<Key>());
		});
	}

	// endregion
}}
