#include "src/cache/AccountStateCache.h"
#include "tests/test/cache/CacheContentsTests.h"
#include "tests/test/cache/CacheIterationTests.h"
#include "tests/test/cache/CacheSynchronizationTests.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountStateCacheTests

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

		using AccountStatePointer = std::shared_ptr<state::AccountState>;
		using ConstAccountStatePointer = std::shared_ptr<const state::AccountState>;
		using AccountStates = std::vector<state::AccountState>;

		Key GenerateRandomPublicKey() {
			return test::GenerateRandomData<Key_Size>();
		}

		Amount GetBalance(const state::AccountState& accountState) {
			return accountState.Balances.get(Xem_Id);
		}

		void DefaultFillCache(AccountStateCache& cache, uint8_t count) {
			auto delta = cache.createDelta();
			for (uint8_t i = 0u; i < count; ++i) {
				if (i % 2)
					delta->addAccount(Address{ { i } }, Height(1235 + i));
				else
					delta->addAccount(Key{ { i } }, Height(1235 + i));
			}

			cache.commit();
		}

		template<typename TTraits>
		auto AddRandomAccount(AccountStateCache& cache) {
			auto delta = cache.createDelta();
			auto accountId = TTraits::GenerateAccountId();
			delta->addAccount(accountId, TTraits::DefaultHeight());
			cache.commit();
			return accountId;
		}

		struct AddressTraits {
			using Type = Address;

			static Type GenerateAccountId() {
				return test::GenerateRandomAddress();
			}

			static constexpr auto DefaultHeight() {
				return Height(321);
			}

			static void AssertFoundAccount(const Type& address, const ConstAccountStatePointer& pFoundAccount) {
				EXPECT_EQ(AddressTraits::DefaultHeight(), pFoundAccount->AddressHeight);
				EXPECT_EQ(address, pFoundAccount->Address);
				EXPECT_EQ(Height(0), pFoundAccount->PublicKeyHeight);
			}
		};

		struct PublicKeyTraits {
			using Type = Key;

			static Type GenerateAccountId() {
				return GenerateRandomPublicKey();
			}

			static constexpr auto DefaultHeight() {
				return Height(432);
			}

			static void AssertFoundAccount(const Type& publicKey, const ConstAccountStatePointer& pFoundAccount) {
				auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
				EXPECT_EQ(PublicKeyTraits::DefaultHeight(), pFoundAccount->AddressHeight);
				EXPECT_EQ(address, pFoundAccount->Address);
				EXPECT_EQ(PublicKeyTraits::DefaultHeight(), pFoundAccount->PublicKeyHeight);
				EXPECT_EQ(publicKey, pFoundAccount->PublicKey);
			}
		};

		template<typename TKeyTraits>
		AccountStatePointer AddAccountToCacheDelta(AccountStateCacheDelta& delta, const typename TKeyTraits::Type& key, Amount balance) {
			auto pAccountState = delta.addAccount(key, TKeyTraits::DefaultHeight());
			pAccountState->Balances.credit(Xem_Id, balance);
			return pAccountState;
		}
	}

#define ID_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_PublicKey) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PublicKeyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
}}

namespace catapult { namespace cache {

	// region network identifier / importance grouping

	TEST(TEST_CLASS, CacheExposesNetworkIdentifier) {
		// Arrange:
		auto networkIdentifier = static_cast<model::NetworkIdentifier>(17);
		AccountStateCache cache(networkIdentifier, 234);

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, cache.networkIdentifier());
	}

	TEST(TEST_CLASS, CacheWrappersExposeNetworkIdentifier) {
		// Arrange:
		auto networkIdentifier = static_cast<model::NetworkIdentifier>(18);
		AccountStateCache cache(networkIdentifier, 543);

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, cache.createView()->networkIdentifier());
		EXPECT_EQ(networkIdentifier, cache.createDelta()->networkIdentifier());
		EXPECT_EQ(networkIdentifier, cache.createDetachedDelta().lock()->networkIdentifier());
	}

	TEST(TEST_CLASS, CacheExposesImportanceGrouping) {
		// Arrange:
		AccountStateCache cache(static_cast<model::NetworkIdentifier>(17), 234);

		// Act + Assert:
		EXPECT_EQ(234u, cache.importanceGrouping());
	}

	TEST(TEST_CLASS, CacheWrappersExposeImportanceGrouping) {
		// Arrange:
		AccountStateCache cache(static_cast<model::NetworkIdentifier>(18), 543);

		// Act + Assert:
		EXPECT_EQ(543u, cache.createView()->importanceGrouping());
		EXPECT_EQ(543u, cache.createDelta()->importanceGrouping());
		EXPECT_EQ(543u, cache.createDetachedDelta().lock()->importanceGrouping());
	}

	// endregion

	// region addAccount (basic)

	ID_BASED_TEST(AddAccountChangesSizeOfCache) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		DefaultFillCache(cache, 10);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();

		// Act:
		delta->addAccount(accountId, TTraits::DefaultHeight());

		// Assert:
		EXPECT_EQ(11u, delta->size());
		EXPECT_TRUE(!!delta->findAccount(accountId));
	}

	ID_BASED_TEST(SubsequentAddAccountsReturnExistingAccount) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		auto pAddedAccount = delta->addAccount(accountId, TTraits::DefaultHeight());

		// Act + Assert:
		for (auto i = 0u; i < 10; ++i) {
			EXPECT_EQ(pAddedAccount, delta->addAccount(accountId, Height(1235u + i)));

			// The height is not supposed to change after multiple adds
			EXPECT_EQ(TTraits::DefaultHeight(), pAddedAccount->AddressHeight);
		}
	}

	// endregion

	// region findAccount

	ID_BASED_TEST(FindAccountReturnsNullptrForAnUnknownAccount_View) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		DefaultFillCache(cache, 10);
		auto view = cache.createView();
		auto accountId = TTraits::GenerateAccountId();

		// Act:
		auto pAccount = view->findAccount(accountId);

		// Assert:
		EXPECT_FALSE(!!pAccount);
		EXPECT_EQ(10u, view->size());
	}

	ID_BASED_TEST(FindAccountConstReturnsNullptrForAnUnknownAccount_Delta) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		DefaultFillCache(cache, 10);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();

		// Act:
		auto pAccount = utils::as_const(delta)->findAccount(accountId);

		// Assert:
		EXPECT_FALSE(!!pAccount);
		EXPECT_EQ(10u, delta->size());
	}

	ID_BASED_TEST(FindAccountThrowsForAnUnknownAccount_Delta) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		DefaultFillCache(cache, 10);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();

		// Act:
		EXPECT_THROW(delta->findAccount(accountId), catapult_invalid_argument);

		// Assert:
		EXPECT_EQ(10u, delta->size());
	}

	ID_BASED_TEST(FindAccountReturnsSameStateAsAdd) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		auto pAddedAccount = delta->addAccount(accountId, TTraits::DefaultHeight());
		cache.commit();

		auto view = cache.createView();

		// Act:
		auto pFoundAccount = view->findAccount(accountId);

		// Assert:
		EXPECT_EQ(1u, view->size());
		EXPECT_EQ(pAddedAccount, pFoundAccount);
		EXPECT_EQ(Amount(0), GetBalance(*pFoundAccount));

		TTraits::AssertFoundAccount(accountId, pFoundAccount);
	}

	ID_BASED_TEST(FindAccountIsIdempotent) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto accountId1 = AddRandomAccount<TTraits>(cache);
		auto accountId2 = accountId1;

		auto view = cache.createView();
		auto delta = cache.createDelta();

		// Act:
		auto pAccount1 = view->findAccount(accountId1);
		auto pAccount2 = const_cast<const AccountStateCacheDelta&>(*delta).findAccount(accountId2);

		// Assert:
		EXPECT_EQ(1u, view->size());
		EXPECT_EQ(1u, delta->size());
		EXPECT_EQ(pAccount1, pAccount2);
	}

	ID_BASED_TEST(FindAccountDifferentAddressesReturnDifferentStates_View) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto accountId1 = AddRandomAccount<TTraits>(cache);
		auto accountId2 = AddRandomAccount<TTraits>(cache);
		auto view = cache.createView();

		// Act:
		auto pAccount1 = view->findAccount(accountId1);
		auto pAccount2 = view->findAccount(accountId2);

		// Assert:
		EXPECT_EQ(2u, view->size());
		EXPECT_NE(pAccount1, pAccount2);
		EXPECT_NE(pAccount1->Address, pAccount2->Address);

		TTraits::AssertFoundAccount(accountId1, pAccount1);
		TTraits::AssertFoundAccount(accountId2, pAccount2);
	}

	ID_BASED_TEST(FindAccountDifferentAddressesReturnDifferentStates_Delta) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto accountId1 = AddRandomAccount<TTraits>(cache);
		auto accountId2 = AddRandomAccount<TTraits>(cache);
		auto delta = cache.createDelta();

		// Act:
		auto pAccount1 = delta->findAccount(accountId1);
		auto pAccount2 = delta->findAccount(accountId2);

		// Assert:
		EXPECT_EQ(2u, delta->size());
		EXPECT_NE(pAccount1, pAccount2);
		EXPECT_NE(pAccount1->Address, pAccount2->Address);

		TTraits::AssertFoundAccount(accountId1, pAccount1);
		TTraits::AssertFoundAccount(accountId2, pAccount2);
	}

	TEST(TEST_CLASS, FindAccountByKeyReturnsNullptrForUnknownPublicKeyButKnownAddress_View) {
		// Arrange:
		auto publicKey = GenerateRandomPublicKey();
		auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		delta->addAccount(address, Height());
		cache.commit();

		auto view = cache.createView();

		// Act:
		auto pAccount = view->findAccount(publicKey);

		// Assert:
		EXPECT_EQ(1u, view->size());
		EXPECT_FALSE(!!pAccount);
	}

	TEST(TEST_CLASS, FindAccountByKeyConstReturnsNullptrForUnknownPublicKeyButKnownAddress_Delta) {
		// Arrange:
		auto publicKey = GenerateRandomPublicKey();
		auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		delta->addAccount(address, Height());
		cache.commit();

		// Act:
		auto pAccount = utils::as_const(delta)->findAccount(publicKey);

		// Assert:
		EXPECT_EQ(1u, delta->size());
		EXPECT_FALSE(!!pAccount);
	}

	TEST(TEST_CLASS, FindAccountByKeyThrowsForUnknownPublicKeyButKnownAddress_Delta) {
		// Arrange:
		auto publicKey = GenerateRandomPublicKey();
		auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		delta->addAccount(address, Height());
		cache.commit();

		// Act:
		EXPECT_THROW(delta->findAccount(publicKey), catapult_invalid_argument);

		// Assert:
		EXPECT_EQ(1u, delta->size());
	}

	ID_BASED_TEST(FindNonConstCreatesCopyOfAccount) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		auto pAddedAccount = delta->addAccount(accountId, TTraits::DefaultHeight());
		cache.commit();

		// Act:
		auto pFoundAccount = delta->findAccount(accountId);

		// Assert:
		EXPECT_NE(pAddedAccount, pFoundAccount);
		EXPECT_FALSE(std::is_const<typename decltype(pFoundAccount)::element_type>());
	}

	ID_BASED_TEST(FindConstDoesNotCreateCopyOfAccount) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		auto pAddedAccount = delta->addAccount(accountId, TTraits::DefaultHeight());
		cache.commit();

		// Act:
		auto pFoundAccount = const_cast<const AccountStateCacheDelta&>(*delta).findAccount(accountId);

		// Assert:
		EXPECT_EQ(pAddedAccount, pFoundAccount);
		EXPECT_TRUE(std::is_const<typename decltype(pFoundAccount)::element_type>());
	}

	// endregion

	// region queueRemove / commitRemovals

	ID_BASED_TEST(Remove_RemovesExistingAccountIfHeightMatches) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto accountId = AddRandomAccount<TTraits>(cache);
		auto delta = cache.createDelta();

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountId, TTraits::DefaultHeight());
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(0u, delta->size());
		EXPECT_FALSE(!!utils::as_const(delta)->findAccount(accountId));
	}

	ID_BASED_TEST(Remove_DoesNotRemovesExistingAccountIfHeightDoesNotMatch) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		auto pExpectedState = delta->addAccount(accountId, TTraits::DefaultHeight());

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountId, TTraits::DefaultHeight() + Height(1));
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(1u, delta->size());
		EXPECT_EQ(pExpectedState, delta->findAccount(accountId));
	}

	ID_BASED_TEST(Remove_CanBeCalledOnNonExistingAccount) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		DefaultFillCache(cache, 10);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();

		// Sanity:
		EXPECT_EQ(10u, delta->size());

		// Act:
		delta->queueRemove(accountId, Height(1236));
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(10u, delta->size());
	}

	ID_BASED_TEST(Remove_QueuesRemovalButDoesNotRemoveImmediately) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		auto pExpectedState = delta->addAccount(accountId, TTraits::DefaultHeight());

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountId, TTraits::DefaultHeight());

		// Assert: the account was not removed yet
		EXPECT_EQ(1u, delta->size());
		EXPECT_EQ(pExpectedState, delta->findAccount(accountId));
	}

	ID_BASED_TEST(CommitRemovals_DoesNothingIfNoRemovalsHaveBeenQueued) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		delta->addAccount(TTraits::GenerateAccountId(), TTraits::DefaultHeight());

		// Act:
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(1u, delta->size());
	}

	ID_BASED_TEST(CommitRemovals_CanCommitQueuedRemovalsOfMultipleAccounts) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();

		// Act: add 5 (0..4) accounts and queue removals of two (1, 3)
		for (auto i = 0u; i < 5u; ++i) {
			auto accountId = TTraits::GenerateAccountId();
			auto height = TTraits::DefaultHeight() + Height(i);
			delta->addAccount(accountId, height);

			if (1u == i % 2u)
				delta->queueRemove(accountId, height);
		}

		// Sanity:
		EXPECT_EQ(5u, delta->size());

		// Act:
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(3u, delta->size());
	}

	// endregion

	// region iteration

	// notice that CacheIterationTests are not sufficient for AccountStateCache because it also supports *delta* iteration

	namespace {
		void AddToCache(AccountStateCacheDelta& delta, const AccountStates& accountStates) {
			for (const auto& state : accountStates)
				delta.addAccount(state.Address, state.AddressHeight);
		}

		struct BaseIterationTraits {
			template<typename TAction>
			static void RunIterationTest(const AccountStates& accountStates, TAction action) {
				// Arrange:
				AccountStateCache cache(Network_Identifier, 543);
				auto delta = cache.createDelta();
				AddToCache(*delta, accountStates);
				cache.commit();

				// Act:
				action(*cache.createView());
			}
		};

		struct DeltaIterationTraits {
			template<typename TAction>
			static void RunIterationTest(const AccountStates& accountStates, TAction action) {
				// Arrange:
				AccountStateCache cache(Network_Identifier, 543);
				auto delta = cache.createDelta();
				AddToCache(*delta, accountStates);

				// Act:
				action(*delta);
			}
		};
	}

#define ITERATION_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Base) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BaseIterationTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Delta) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeltaIterationTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		template<typename TTraits>
		void AssertIteration(const AccountStates& expectedEntities) {
			// Arrange:
			TTraits::RunIterationTest(expectedEntities, [&expectedEntities](const auto& view) {
				// Assert:
				EXPECT_EQ(expectedEntities.size(), view.size());
				for (auto iter = view.cbegin(); view.cend() != iter; ++iter) {
					auto pState = iter->second;
					auto index = pState->AddressHeight.unwrap() - 1;
					auto expectedEntity = expectedEntities[index];

					auto message = "account state at " + std::to_string(index);
					EXPECT_EQ(expectedEntity.Address, pState->Address) << message;
					EXPECT_EQ(expectedEntity.AddressHeight, pState->AddressHeight) << message;
					EXPECT_EQ(Height(0), pState->PublicKeyHeight) << message;
				}
			});
		}
	}

	ITERATION_BASED_TEST(CanIterateThroughEmptyCache) {
		// Assert:
		AssertIteration<TTraits>({});
	}

	ITERATION_BASED_TEST(CanIterateThroughSingleValueCache) {
		// Assert:
		AssertIteration<TTraits>({
			state::AccountState(Address{ { 2 } }, Height(1))
		});
	}

	ITERATION_BASED_TEST(CanIterateThroughMultiValueCache) {
		// Assert:
		AssertIteration<TTraits>({
			state::AccountState(Address{ { 2 } }, Height(1)),
			state::AccountState(Address{ { 4 } }, Height(2)),
			state::AccountState(Address{ { 9 } }, Height(3)),
		});
	}

	// endregion

	// region MixedState tests (public key added at a different height)

	namespace {
		void AssertState(
				Height expectedHeight,
				AccountStateCacheDelta& delta,
				const Address& address,
				const state::AccountState& accountState) {
			EXPECT_EQ(1u, delta.size());
			EXPECT_EQ(address, accountState.Address);
			EXPECT_EQ(Height(321), accountState.AddressHeight);
			EXPECT_EQ(expectedHeight, accountState.PublicKeyHeight);
			EXPECT_EQ(Amount(1234), GetBalance(accountState));
			EXPECT_EQ(delta.findAccount(address)->PublicKey, accountState.PublicKey);
		}

		void AssertRemoveByKeyAtHeight(Height removalHeight) {
			// Arrange:
			auto publicKey = GenerateRandomPublicKey();
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

			AccountStateCache cache(Network_Identifier, 543);
			auto delta = cache.createDelta();
			AddAccountToCacheDelta<AddressTraits>(*delta, address, Amount(1234));
			AddAccountToCacheDelta<PublicKeyTraits>(*delta, publicKey, Amount());

			// Act:
			delta->queueRemove(publicKey, removalHeight);
			delta->commitRemovals();
			auto pAccount = utils::as_const(delta)->findAccount(publicKey);

			// Assert:
			if (PublicKeyTraits::DefaultHeight() != removalHeight)
				AssertState(PublicKeyTraits::DefaultHeight(), *delta, address, *pAccount);
			else
				EXPECT_FALSE(!!pAccount);
		}

		void AssertRemoveByAddressAtHeight(Height removalHeight) {
			// Arrange:
			auto publicKey = GenerateRandomPublicKey();
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

			AccountStateCache cache(Network_Identifier, 543);
			auto delta = cache.createDelta();
			AddAccountToCacheDelta<AddressTraits>(*delta, address, Amount(1234));
			AddAccountToCacheDelta<PublicKeyTraits>(*delta, publicKey, Amount());

			// Act:
			delta->queueRemove(address, removalHeight);
			delta->commitRemovals();
			auto pAccount = utils::as_const(delta)->findAccount(address);

			if (AddressTraits::DefaultHeight() == removalHeight) {
				EXPECT_EQ(0u, delta->size());
				EXPECT_FALSE(!!pAccount);
				return;
			}

			AssertState(PublicKeyTraits::DefaultHeight(), *delta, address, *pAccount);
		}
	}

	TEST(TEST_CLASS, MixedState_Remove_ByKeyRemovesOnlyPublicKeyIfHeightMatches) {
		AssertRemoveByKeyAtHeight(PublicKeyTraits::DefaultHeight());
	}

	TEST(TEST_CLASS, MixedState_Remove_ByKeyDoesNotRemoveIfHeightDoesNotMatch) {
		AssertRemoveByKeyAtHeight(PublicKeyTraits::DefaultHeight() + Height(1));
	}

	TEST(TEST_CLASS, MixedState_Remove_ByAddressRemovesAnAccountIfHeightMatches) {
		AssertRemoveByAddressAtHeight(AddressTraits::DefaultHeight());
	}

	TEST(TEST_CLASS, MixedState_Remove_ByAddressDoesNotRemoveIfHeightDoesNotMatch) {
		AssertRemoveByAddressAtHeight(AddressTraits::DefaultHeight() + Height(1));
	}

	TEST(TEST_CLASS, CanQueueMultipleRemovalsOfSameAccount) {
		// Arrange:
		auto publicKey = GenerateRandomPublicKey();
		auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		AddAccountToCacheDelta<AddressTraits>(*delta, address, Amount(1234));
		AddAccountToCacheDelta<PublicKeyTraits>(*delta, publicKey, Amount());

		// Act:
		delta->queueRemove(publicKey, PublicKeyTraits::DefaultHeight());
		delta->queueRemove(address, AddressTraits::DefaultHeight());
		delta->queueRemove(publicKey, PublicKeyTraits::DefaultHeight());
		delta->queueRemove(address, AddressTraits::DefaultHeight());
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(0u, delta->size());
		EXPECT_FALSE(!!utils::as_const(delta)->findAccount(address));
	}

	namespace {
		model::AccountInfo CreateRandomAccountInfoWithKey() {
			model::AccountInfo info;
			info.Size = sizeof(model::AccountInfo);
			info.NumMosaics = 0;

			info.PublicKeyHeight = Height(124);
			info.PublicKey = GenerateRandomPublicKey();

			info.AddressHeight = Height(123);
			info.Address = model::PublicKeyToAddress(info.PublicKey, Network_Identifier);
			return info;
		}

		model::AccountInfo CreateRandomAccountInfoWithoutKey() {
			auto info = CreateRandomAccountInfoWithKey();
			info.PublicKeyHeight = Height(0);
			return info;
		}

		template<typename TCache, typename TCacheQualifier>
		void AssertCanAccessAllAccountsThroughFindByAddress(TCache& cache, TCacheQualifier qualifier) {
			// Arrange:
			auto address1 = test::GenerateRandomAddress();
			auto address2 = test::GenerateRandomAddress();
			auto address3 = test::GenerateRandomAddress();
			auto address4 = test::GenerateRandomAddress();
			auto key5 = GenerateRandomPublicKey();
			auto address5 = model::PublicKeyToAddress(key5, Network_Identifier);

			auto info6 = CreateRandomAccountInfoWithKey();
			auto info7 = CreateRandomAccountInfoWithoutKey();
			auto info8 = CreateRandomAccountInfoWithKey();
			auto info9 = CreateRandomAccountInfoWithoutKey();

			cache.addAccount(address1, Height(100));
			cache.addAccount(address3, Height(100));
			cache.addAccount(key5, Height(100));

			cache.addAccount(info6);
			cache.addAccount(info7);

			// Act + Assert:
			auto& qualifiedCache = *qualifier(cache);
			EXPECT_TRUE(!!qualifiedCache.findAccount(address1));
			EXPECT_FALSE(qualifiedCache.contains(address2));
			EXPECT_TRUE(!!qualifiedCache.findAccount(address3));
			EXPECT_FALSE(qualifiedCache.contains(address4));
			EXPECT_TRUE(!!qualifiedCache.findAccount(address5)); // added by key but accessible via address

			EXPECT_TRUE(!!qualifiedCache.findAccount(info6.Address));
			EXPECT_TRUE(!!qualifiedCache.findAccount(info7.Address));
			EXPECT_FALSE(qualifiedCache.contains(info8.Address));
			EXPECT_FALSE(qualifiedCache.contains(info9.Address));
		}

		template<typename TCache, typename TCacheQualifier>
		void AssertCanAccessAllAccountsThroughFindByPublicKey(TCache& cache, TCacheQualifier qualifier) {
			// Arrange:
			auto key1 = GenerateRandomPublicKey();
			auto key2 = GenerateRandomPublicKey();
			auto key3 = GenerateRandomPublicKey();
			auto key4 = GenerateRandomPublicKey();
			auto key5 = GenerateRandomPublicKey();
			auto address5 = model::PublicKeyToAddress(key5, Network_Identifier);

			auto info6 = CreateRandomAccountInfoWithKey();
			auto info7 = CreateRandomAccountInfoWithoutKey();
			auto info8 = CreateRandomAccountInfoWithKey();
			auto info9 = CreateRandomAccountInfoWithoutKey();

			cache.addAccount(key1, Height(100));
			cache.addAccount(key3, Height(100));
			cache.addAccount(address5, Height(100));

			cache.addAccount(info6);
			cache.addAccount(info7);

			// Act + Assert:
			auto& qualifiedCache = *qualifier(cache);
			EXPECT_TRUE(!!qualifiedCache.findAccount(key1));
			EXPECT_FALSE(qualifiedCache.contains(key2));
			EXPECT_TRUE(!!qualifiedCache.findAccount(key3));
			EXPECT_FALSE(qualifiedCache.contains(key4));
			EXPECT_FALSE(qualifiedCache.contains(key5)); // only added via address

			EXPECT_TRUE(!!qualifiedCache.findAccount(info6.PublicKey));
			EXPECT_FALSE(qualifiedCache.contains(info7.PublicKey)); // only added via address
			EXPECT_FALSE(qualifiedCache.contains(info8.PublicKey));
			EXPECT_FALSE(qualifiedCache.contains(info9.PublicKey));
		}
	}

	TEST(TEST_CLASS, CanAccessAllAccountsThroughFindByAddress) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();

		// Assert:
		AssertCanAccessAllAccountsThroughFindByAddress(*delta, [](auto& c) {
			return &const_cast<AccountStateCacheDelta&>(c);
		});
	}

	TEST(TEST_CLASS, CanAccessAllAccountsThroughFindConstByAddress) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();

		// Assert:
		AssertCanAccessAllAccountsThroughFindByAddress(*delta, [](auto& c) {
			return &const_cast<const AccountStateCacheDelta&>(c);
		});
	}

	TEST(TEST_CLASS, CanAccessAllAccountsThroughFindByPublicKey) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();

		// Assert:
		AssertCanAccessAllAccountsThroughFindByPublicKey(*delta, [](auto& c) {
			return &const_cast<AccountStateCacheDelta&>(c);
		});
	}

	TEST(TEST_CLASS, CanAccessAllAccountsThroughFindConstByPublicKey) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();

		// Assert:
		AssertCanAccessAllAccountsThroughFindByPublicKey(*delta, [](auto& c) {
			return &const_cast<const AccountStateCacheDelta&>(c);
		});
	}

	// endregion

	// region addAccount (AccountInfo)

	namespace {
		// inconsistent because the address and public key are not related
		auto CreateInconsistentAccountInfo() {
			model::AccountInfo info;
			info.Size = sizeof(model::AccountInfo);
			info.Address = test::GenerateRandomAddress();
			info.PublicKey = GenerateRandomPublicKey();
			info.PublicKeyHeight = Height(123);
			info.Importances[0] = Importance(421);
			info.ImportanceHeights[0] = model::ImportanceHeight(1);
			info.NumMosaics = 0;
			return info;
		}

		void AssertEqual(const model::AccountInfo& expected, const state::AccountState& actual, const char* message) {
			// Assert:
			EXPECT_EQ(expected.Address, actual.Address) << message;
			EXPECT_EQ(expected.PublicKey, actual.PublicKey) << message;
			EXPECT_EQ(expected.PublicKeyHeight, actual.PublicKeyHeight) << message;
			EXPECT_EQ(expected.Importances[0], actual.ImportanceInfo.current()) << message;
			EXPECT_EQ(0u, actual.Balances.size()) << message;
		}
	}

	TEST(TEST_CLASS, CanAddAccountViaAccountInfoWithoutPublicKey) {
		// Arrange: note that public key height is 0
		auto info = CreateInconsistentAccountInfo();
		info.PublicKeyHeight = Height(0);

		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();

		// Act:
		auto pState = delta->addAccount(info);
		auto pStateFromAddress = delta->findAccount(info.Address);
		auto pStateFromKey = utils::as_const(delta)->findAccount(info.PublicKey);

		// Assert:
		// - info properties should have been copied into the state
		// - state is only accessible by address because public key height is 0
		ASSERT_TRUE(!!pState);
		ASSERT_TRUE(!!pStateFromAddress);
		EXPECT_FALSE(!!pStateFromKey);

		AssertEqual(info, *pState, "pState");
		AssertEqual(info, *pStateFromAddress, "pStateFromAddress");
	}

	TEST(TEST_CLASS, CanAddAccountViaAccountInfoWithPublicKey) {
		// Arrange: note that public key height is not 0
		auto info = CreateInconsistentAccountInfo();
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();

		// Act:
		auto pState = delta->addAccount(info);
		auto pStateFromAddress = delta->findAccount(info.Address);
		auto pStateFromKey = delta->findAccount(info.PublicKey);

		// Assert:
		// - info properties should have been copied into the state
		// - state is accessible by address and public key (note that info mapping, if present, is trusted)
		ASSERT_TRUE(!!pState);
		ASSERT_TRUE(!!pStateFromAddress);
		ASSERT_TRUE(!!pStateFromKey);

		AssertEqual(info, *pState, "pState");
		AssertEqual(info, *pStateFromAddress, "pStateFromAddress");
		AssertEqual(info, *pStateFromKey, "pStateFromKey");
	}

	namespace {
		template<typename TAdd>
		void AssertAddAccountViaInfoDoesNotOverrideKnownAccounts(TAdd add) {
			// Arrange: create an info
			auto info = CreateRandomAccountInfoWithKey();
			info.Importances[0] = Importance(777);

			AccountStateCache cache(Network_Identifier, 543);
			auto delta = cache.createDelta();

			// Act: add the info using add and set importance to 123
			auto pAddState1 = add(*delta, info);
			pAddState1->ImportanceInfo.set(Importance(123), model::ImportanceHeight(1));

			// - add the info again (with importance 777)
			auto pAddState2 = delta->addAccount(info);

			// - get the info from the cache
			auto pState = delta->findAccount(info.Address);

			// Assert: the second add had no effect (the importance is still 123)
			ASSERT_TRUE(!!pState);
			EXPECT_EQ(pAddState1, pState);
			EXPECT_EQ(pAddState2, pState);
			EXPECT_EQ(Importance(123), pState->ImportanceInfo.current());
		}
	}

	TEST(TEST_CLASS, AddAccountViaInfoDoesNotOverrideKnownAccounts_Address) {
		// Assert:
		AssertAddAccountViaInfoDoesNotOverrideKnownAccounts([](auto& delta, const auto& info) {
			return delta.addAccount(info.Address, info.AddressHeight);
		});
	}

	TEST(TEST_CLASS, AddAccountViaInfoDoesNotOverrideKnownAccounts_PublicKey) {
		// Assert:
		AssertAddAccountViaInfoDoesNotOverrideKnownAccounts([](auto& delta, const auto& info) {
			return delta.addAccount(info.PublicKey, info.PublicKeyHeight);
		});
	}

	TEST(TEST_CLASS, AddAccountViaInfoDoesNotOverrideKnownAccounts_Info) {
		// Assert:
		AssertAddAccountViaInfoDoesNotOverrideKnownAccounts([](auto& delta, const auto& info) {
			return delta.addAccount(info);
		});
	}

	// endregion

	// region modifiedAccountStates / removedAccountStates

	namespace {
		std::set<Address> GetAddresses(const std::unordered_set<account_state_cache_types::ConstPointerType>& accountStates) {
			std::set<Address> addresses;
			for (const auto& pAccountState : accountStates)
				addresses.insert(pAccountState->Address);

			return addresses;
		}

		void AssertMarkedAddresses(
				const AccountStateCacheDelta& delta,
				const std::set<Address>& expectedModifiedAddresses,
				const std::set<Address>& expectedRemovedAddresses) {
			// note that std::set is used because order of returned addresses is not important
			EXPECT_EQ(expectedModifiedAddresses, GetAddresses(delta.modifiedAccountStates()));
			EXPECT_EQ(expectedRemovedAddresses, GetAddresses(delta.removedAccountStates()));
		}
	}

	TEST(TEST_CLASS, InitiallyNoAccountsAreMarkedAsModifiedOrRemoved) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();

		// Assert:
		AssertMarkedAddresses(*delta, {}, {});
	}

	TEST(TEST_CLASS, AddedAccountIsMarkedAsModified) {
		// Arrange:
		auto address = test::GenerateRandomAddress();
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();

		// Act:
		delta->addAccount(address, Height(100));

		// Assert:
		AssertMarkedAddresses(*delta, { address }, {});
	}

	TEST(TEST_CLASS, ModifiedAccountIsMarkedAsModified) {
		// Arrange:
		auto address = test::GenerateRandomAddress();
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		delta->addAccount(address, Height(100));
		cache.commit();

		// Act:
		// - modify the account
		delta->findAccount(address)->PublicKey = GenerateRandomPublicKey();

		// Assert:
		AssertMarkedAddresses(*delta, { address }, {});
	}

	TEST(TEST_CLASS, RemovedAccountIsMarkedAsRemoved) {
		// Arrange:
		auto address = test::GenerateRandomAddress();
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();
		delta->addAccount(address, Height(100));
		cache.commit();

		// Act:
		// - remove the account
		delta->queueRemove(address, Height(100));
		delta->commitRemovals();

		// Assert:
		AssertMarkedAddresses(*delta, {}, { address });
	}

	TEST(TEST_CLASS, MultipleMarkedAccountsCanBeTracked) {
		// Arrange: add ten accounts
		std::vector<Address> addresses;
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();

		for (auto i = 0u; i < 10u; ++i) {
			addresses.push_back(test::GenerateRandomAddress());
			delta->addAccount(addresses.back(), Height(100));
		}

		cache.commit();

		// Act:
		// - delete three
		delta->queueRemove(addresses[2], Height(100));
		delta->queueRemove(addresses[5], Height(100));
		delta->queueRemove(addresses[8], Height(100));
		delta->commitRemovals();

		// - modify two
		delta->findAccount(addresses[3])->PublicKey = GenerateRandomPublicKey();
		delta->findAccount(addresses[7])->PublicKey = GenerateRandomPublicKey();

		// - add two
		auto address1 = test::GenerateRandomAddress();
		auto address2 = test::GenerateRandomAddress();
		delta->addAccount(address1, Height(100));
		delta->addAccount(address2, Height(100));

		// Assert:
		AssertMarkedAddresses(
				*delta,
				{ addresses[3], addresses[7], address1, address2 },
				{ addresses[2], addresses[5], addresses[8] });
	}

	// endregion

	// region general cache tests

	namespace {
		struct AccountStateCacheAddressEntityTraits {
		public:
			using KeyType = Address;

			static AccountStatePointer CreateEntity(size_t id) {
				auto pState = std::make_shared<state::AccountState>(Address{ { static_cast<uint8_t>(id) } }, DefaultHeight());
				pState->PublicKeyHeight = Height();
				return pState;
			}

			static const KeyType& ToKey(const ConstAccountStatePointer& pState) {
				return pState->Address;
			}

			static const KeyType& ToKey(const std::pair<Address, AccountStatePointer>& pair) {
				return ToKey(pair.second);
			}

			static Height DefaultHeight() {
				return AddressTraits::DefaultHeight();
			}
		};

		struct AccountStateCachePublicKeyEntityTraits {
		public:
			using KeyType = Key;

			static AccountStatePointer CreateEntity(size_t id) {
				auto publicKey = Key{ { static_cast<uint8_t>(id) } };
				auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
				auto pState = std::make_shared<state::AccountState>(address, DefaultHeight());
				pState->PublicKey = publicKey;
				pState->PublicKeyHeight = DefaultHeight();
				return pState;
			}

			static const KeyType& ToKey(const ConstAccountStatePointer& pState) {
				return pState->PublicKey;
			}

			static const KeyType& ToKey(const std::pair<Address, AccountStatePointer>& pair) {
				return ToKey(pair.second);
			}

			static Height DefaultHeight() {
				return PublicKeyTraits::DefaultHeight();
			}
		};

		template<typename TEntityTraits>
		struct AccountStateCacheTraits {
		public:
			using EntityTraits = TEntityTraits;
			using EntityVector = std::vector<AccountStatePointer>;

		public:
			template<typename TAction>
			static void RunEmptyCacheTest(TAction action) {
				// Arrange:
				AccountStateCache cache(Network_Identifier, 543);

				// Act:
				action(cache);
			}

			template<typename TAction>
			static void RunCacheTest(TAction action) {
				// Arrange:
				AccountStateCache cache(Network_Identifier, 543);
				auto entities = InsertMultiple(cache, { 1, 4, 9 });

				// Act:
				action(cache, entities);
			}

			static auto InsertMultiple(AccountStateCache& cache, std::initializer_list<size_t> ids) {
				auto delta = cache.createDelta();
				std::vector<typename EntityTraits::KeyType> entities;
				for (auto id : ids) {
					auto pState = EntityTraits::CreateEntity(id);
					delta->addAccount(*pState->toAccountInfo());
					entities.push_back(EntityTraits::ToKey(pState));
				}

				cache.commit();
				return entities;
			}

		public:
			static void Insert(AccountStateCacheDelta& delta, const ConstAccountStatePointer& pState) {
				delta.addAccount(EntityTraits::ToKey(pState), EntityTraits::DefaultHeight());
			}

			static void Remove(AccountStateCacheDelta& delta, const ConstAccountStatePointer& pState) {
				delta.queueRemove(EntityTraits::ToKey(pState), EntityTraits::DefaultHeight());
				delta.commitRemovals();
			}
		};

		using AccountStateCacheAddressTraits = AccountStateCacheTraits<AccountStateCacheAddressEntityTraits>;
		using AccountStateCachePublicKeyTraits = AccountStateCacheTraits<AccountStateCachePublicKeyEntityTraits>;
	}

	DEFINE_CACHE_CONTENTS_TESTS_SUFFIX(TEST_CLASS, AccountStateCacheAddressTraits, _Address)
	DEFINE_CACHE_ITERATION_TESTS_SUFFIX(TEST_CLASS, AccountStateCacheAddressTraits, Unordered, _Address)

	DEFINE_CACHE_CONTENTS_TESTS_SUFFIX(TEST_CLASS, AccountStateCachePublicKeyTraits, _PublicKey)
	DEFINE_CACHE_ITERATION_TESTS_SUFFIX(TEST_CLASS, AccountStateCachePublicKeyTraits, Unordered, _PublicKey)

	DEFINE_CACHE_SYNC_TESTS(TEST_CLASS, AccountStateCacheAddressTraits)

	// endregion
}}
