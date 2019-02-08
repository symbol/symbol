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

#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/utils/Casting.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountStateCacheTests

	// region key traits

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
		constexpr auto Currency_Mosaic_Id = MosaicId(1234);
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);

		constexpr auto Default_Cache_Options = AccountStateCacheTypes::Options{
			Network_Identifier,
			543,
			Amount(std::numeric_limits<Amount::ValueType>::max()),
			Currency_Mosaic_Id,
			Harvesting_Mosaic_Id
		};

		struct AddressTraits {
			using Type = Address;

			static Type GenerateAccountId() {
				return test::GenerateRandomAddress();
			}

			static constexpr auto DefaultHeight() {
				return Height(321);
			}

			template<typename TCache>
			static auto Find(TCache& cache, const Address& address) {
				return cache.find(address);
			}

			static Type ToKey(const state::AccountState& accountState) {
				return accountState.Address;
			}
		};

		struct PublicKeyTraits {
			using Type = Key;

			static Type GenerateAccountId() {
				return test::GenerateRandomData<Key_Size>();
			}

			static constexpr auto DefaultHeight() {
				return Height(432);
			}

			template<typename TCache>
			static auto Find(TCache& cache, const Key& publicKey) {
				return cache.find(publicKey);
			}

			static Type ToKey(const state::AccountState& accountState) {
				return accountState.PublicKey;
			}
		};
	}

	// endregion

	// region mixin traits based tests

	namespace {
		// proxy is required because
		// 1. insert has non-standard name 'addAccount'
		// 2. queueRemove/commitRemovals behavior is non-standard

		template<typename TTraits>
		struct DeltaProxy {
		private:
			using IdType = typename TTraits::Type;

		public:
			explicit DeltaProxy(LockedCacheDelta<AccountStateCacheDelta>&& delta) : m_delta(std::move(delta))
			{}

		public:
			size_t size() const {
				return m_delta->size();
			}

			bool contains(const IdType& accountId) const {
				return m_delta->contains(accountId);
			}

			auto find(const IdType& accountId) {
				return m_delta->find(accountId);
			}

			auto find(const IdType& accountId) const {
				return m_delta->find(accountId);
			}

			void insert(const state::AccountState& accountState) {
				m_delta->addAccount(TTraits::ToKey(accountState), Height(7));
			}

			void remove(const IdType& address) {
				m_delta->queueRemove(address, Height(7));
				m_delta->commitRemovals();
			}

			auto addedElements() const {
				return m_delta->addedElements();
			}

			auto modifiedElements() const {
				return m_delta->modifiedElements();
			}

			auto removedElements() const {
				return m_delta->removedElements();
			}

			auto asReadOnly() const {
				return m_delta->asReadOnly();
			}

		private:
			LockedCacheDelta<AccountStateCacheDelta> m_delta;
		};

		template<typename TTraits>
		struct CacheProxy : public AccountStateCache {
		public:
			CacheProxy() : AccountStateCache(CacheConfiguration(), Default_Cache_Options)
			{}

		public:
			auto createDelta() {
				return std::make_unique<DeltaProxy<TTraits>>(AccountStateCache::createDelta());
			}
		};

		template<typename TTraits>
		struct AccountStateMixinTraits {
			using IdTraits = TTraits;
			using CacheType = CacheProxy<IdTraits>;
			using IdType = typename IdTraits::Type;
			using ValueType = state::AccountState;

			static uint8_t GetRawId(const IdType& id) {
				return id[0];
			}

			static IdType GetId(const std::pair<Key, Address>& pair) {
				return pair.first;
			}

			static IdType GetId(const state::AccountState& accountState) {
				return IdTraits::ToKey(accountState);
			}

			static IdType MakeId(uint8_t id) {
				return { { id } };
			}

			static ValueType CreateWithId(uint8_t id) {
				// store same id in both Address and PublicKey so these traits work for both
				auto accountState = state::AccountState({ { id } }, Height());
				accountState.PublicKey = { { id } };
				return accountState;
			}
		};

		// custom modification policy is needed because double insert can be noop (e.g. double address insert)
		template<typename TTraits>
		struct AccountStateModificationPolicy {
			static void Modify(DeltaProxy<TTraits>& delta, const state::AccountState& accountState) {
				auto& accountStateFromCache = delta.find(TTraits::ToKey(accountState)).get();
				accountStateFromCache.Balances.credit(Harvesting_Mosaic_Id, Amount(1));
			}
		};
	}

#define DEFINE_ACCOUNT_STATE_CACHE_TESTS(TRAITS, SUFFIX) \
	DEFINE_CACHE_CONTAINS_TESTS(TRAITS, ViewAccessor, _View##SUFFIX) \
	DEFINE_CACHE_CONTAINS_TESTS(TRAITS, DeltaAccessor, _Delta##SUFFIX) \
	\
	DEFINE_CACHE_ACCESSOR_TESTS(TRAITS, ViewAccessor, MutableAccessor, _ViewMutable##SUFFIX) \
	DEFINE_CACHE_ACCESSOR_TESTS(TRAITS, ViewAccessor, ConstAccessor, _ViewConst##SUFFIX) \
	DEFINE_CACHE_ACCESSOR_TESTS(TRAITS, DeltaAccessor, MutableAccessor, _DeltaMutable##SUFFIX) \
	DEFINE_CACHE_ACCESSOR_TESTS(TRAITS, DeltaAccessor, ConstAccessor, _DeltaConst##SUFFIX) \
	\
	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(TRAITS, AccountStateModificationPolicy<TRAITS::IdTraits>, _Delta##SUFFIX)

	DEFINE_ACCOUNT_STATE_CACHE_TESTS(AccountStateMixinTraits<AddressTraits>, _Address)

	// only address-based iteration is supported
	DEFINE_CACHE_ITERATION_TESTS(AccountStateMixinTraits<AddressTraits>, ViewAccessor, _View_Address)

	DEFINE_ACCOUNT_STATE_CACHE_TESTS(AccountStateMixinTraits<PublicKeyTraits>, _PublicKey)

	// only one set of basic tests are needed
	DEFINE_CACHE_BASIC_TESTS(AccountStateMixinTraits<AddressTraits>,)

	// endregion

	// *** custom tests ***

	// region test utils

	namespace {
		Key GenerateRandomPublicKey() {
			return PublicKeyTraits::GenerateAccountId();
		}

		Amount GetBalance(const state::AccountState& accountState) {
			return accountState.Balances.get(Harvesting_Mosaic_Id);
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

		template<typename TKeyTraits>
		void AddAccountToCacheDelta(AccountStateCacheDelta& delta, const typename TKeyTraits::Type& key, Amount balance) {
			delta.addAccount(key, TKeyTraits::DefaultHeight());
			delta.find(key).get().Balances.credit(Harvesting_Mosaic_Id, balance);
		}

		state::AccountState CreateAccountStateWithRandomAddressAndPublicKey() {
			auto publicKey = GenerateRandomPublicKey();
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

			auto accountState = state::AccountState(address, Height(123));
			accountState.PublicKeyHeight = Height(124);
			accountState.PublicKey = publicKey;
			return accountState;
		}

		state::AccountState CreateAccountStateWithRandomAddress() {
			auto accountState = CreateAccountStateWithRandomAddressAndPublicKey();
			accountState.PublicKeyHeight = Height(0);
			return accountState;
		}

		auto CreateAccountStateWithMismatchedAddressAndPublicKey() {
			auto accountState = CreateAccountStateWithRandomAddressAndPublicKey();
			accountState.PublicKey = GenerateRandomPublicKey();
			return accountState;
		}
	}

#define ID_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_PublicKey) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PublicKeyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region network identifier / importance grouping

	TEST(TEST_CLASS, CacheExposesNetworkIdentifier) {
		// Arrange:
		auto networkIdentifier = static_cast<model::NetworkIdentifier>(17);
		auto options = Default_Cache_Options;
		options.NetworkIdentifier = networkIdentifier;
		AccountStateCache cache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, cache.networkIdentifier());
	}

	TEST(TEST_CLASS, CacheWrappersExposeNetworkIdentifier) {
		// Arrange:
		auto networkIdentifier = static_cast<model::NetworkIdentifier>(18);
		auto options = Default_Cache_Options;
		options.NetworkIdentifier = networkIdentifier;
		AccountStateCache cache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, cache.createView()->networkIdentifier());
		EXPECT_EQ(networkIdentifier, cache.createDelta()->networkIdentifier());
		EXPECT_EQ(networkIdentifier, cache.createDetachedDelta().lock()->networkIdentifier());
	}

	TEST(TEST_CLASS, CacheExposesImportanceGrouping) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.ImportanceGrouping = 234;
		AccountStateCache cache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(234u, cache.importanceGrouping());
	}

	TEST(TEST_CLASS, CacheWrappersExposeHarvestingMosaicId) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.HarvestingMosaicId = MosaicId(11229988);
		AccountStateCache cache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(MosaicId(11229988), cache.createView()->harvestingMosaicId());
		EXPECT_EQ(MosaicId(11229988), cache.createDelta()->harvestingMosaicId());
		EXPECT_EQ(MosaicId(11229988), cache.createDetachedDelta().lock()->harvestingMosaicId());
	}

	// endregion

	// region tryGet (custom tests due to multiple views into the cache)

	TEST(TEST_CLASS, FindByKeyReturnsEmptyIteratorForUnknownPublicKeyButKnownAddress_View) {
		// Arrange:
		auto publicKey = GenerateRandomPublicKey();
		auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		delta->addAccount(address, Height());
		cache.commit();

		auto view = cache.createView();

		// Act:
		auto accountStateIter = PublicKeyTraits::Find(*view, publicKey);

		// Assert:
		EXPECT_EQ(1u, view->size());
		EXPECT_FALSE(!!accountStateIter.tryGet());
	}

	TEST(TEST_CLASS, FindByKeyConstReturnsEmptyIteratorForUnknownPublicKeyButKnownAddress_Delta) {
		// Arrange:
		auto publicKey = GenerateRandomPublicKey();
		auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		delta->addAccount(address, Height());
		cache.commit();

		// Act:
		auto accountStateIter = PublicKeyTraits::Find(utils::as_const(*delta), publicKey);

		// Assert:
		EXPECT_EQ(1u, delta->size());
		EXPECT_FALSE(!!accountStateIter.tryGet());
	}

	// endregion

	// region addAccount (basic)

	namespace {
		void AddThreeMosaicBalances(state::AccountState& accountState) {
			accountState.Balances.credit(MosaicId(1), Amount(2));
			accountState.Balances.credit(Harvesting_Mosaic_Id, Amount(3));
			accountState.Balances.credit(Currency_Mosaic_Id, Amount(1));
		}
	}

	ID_BASED_TEST(AddAccountChangesSizeOfCache) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		DefaultFillCache(cache, 10);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();

		// Act:
		delta->addAccount(accountId, TTraits::DefaultHeight());

		// Assert:
		EXPECT_EQ(11u, delta->size());
		EXPECT_TRUE(!!delta->find(accountId).tryGet());
	}

	ID_BASED_TEST(AddAccountAutomaticallyOptimizesCurrencyMosaicAccess) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();

		// Act:
		delta->addAccount(accountId, TTraits::DefaultHeight());
		AddThreeMosaicBalances(delta->find(accountId).get());

		// Assert:
		EXPECT_EQ(Currency_Mosaic_Id, delta->find(accountId).get().Balances.begin()->first);
	}

	ID_BASED_TEST(SubsequentAddAccountHasNoEffect) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		delta->addAccount(accountId, TTraits::DefaultHeight());
		const auto& originalAddedAccountState = delta->find(accountId).get();

		// Act + Assert:
		for (auto i = 0u; i < 10; ++i) {
			delta->addAccount(accountId, Height(1235u + i));
			const auto& addedAccountState = delta->find(accountId).get();

			// - only first added account should change the state (and subsequent adds do not change height)
			EXPECT_EQ(&originalAddedAccountState, &addedAccountState);
			EXPECT_EQ(TTraits::DefaultHeight(), addedAccountState.AddressHeight);
		}
	}

	TEST(TEST_CLASS, SubsequentAddAddressDoesNotMarkAccountAsDirty) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto address = AddressTraits::GenerateAccountId();
		delta->addAccount(address, Height(1230));
		cache.commit();

		// Act:
		delta->addAccount(address, Height(1235));
		const auto& addedAccountState = const_cast<const decltype(delta)&>(delta)->find(address).get();

		// Sanity:
		EXPECT_EQ(Height(1230), addedAccountState.AddressHeight);
		EXPECT_EQ(Height(0), addedAccountState.PublicKeyHeight);

		// Assert:
		EXPECT_TRUE(!!delta->addedElements().empty());
		EXPECT_TRUE(!!delta->modifiedElements().empty());
		EXPECT_TRUE(!!delta->removedElements().empty());
	}

	TEST(TEST_CLASS, SubsequentAddPublicKeyDoesNotMarkAccountWithPublicKeyAsDirty) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto publicKey = GenerateRandomPublicKey();
		delta->addAccount(publicKey, Height(1230));
		cache.commit();

		// Act:
		delta->addAccount(publicKey, Height(1235));
		const auto& addedAccountState = const_cast<const decltype(delta)&>(delta)->find(publicKey).get();

		// Sanity:
		EXPECT_EQ(Height(1230), addedAccountState.AddressHeight);
		EXPECT_EQ(Height(1230), addedAccountState.PublicKeyHeight);

		// Assert:
		EXPECT_TRUE(!!delta->addedElements().empty());
		EXPECT_TRUE(!!delta->modifiedElements().empty());
		EXPECT_TRUE(!!delta->removedElements().empty());
	}

	TEST(TEST_CLASS, SubsequentAddPublicKeyDoesMarkAccountWithoutPublicKeyAsDirty) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto publicKey = GenerateRandomPublicKey();
		auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
		delta->addAccount(address, Height(1230));
		cache.commit();

		// Act:
		delta->addAccount(publicKey, Height(1235));
		const auto& addedAccountState = const_cast<const decltype(delta)&>(delta)->find(address).get();

		// Sanity:
		EXPECT_EQ(Height(1230), addedAccountState.AddressHeight);
		EXPECT_EQ(Height(1235), addedAccountState.PublicKeyHeight);

		// Assert:
		EXPECT_TRUE(!!delta->addedElements().empty());
		EXPECT_FALSE(!!delta->modifiedElements().empty());
		EXPECT_TRUE(!!delta->removedElements().empty());
	}

	TEST(TEST_CLASS, SubsequentAddAccountStateDoesNotMarkAccountAsDirty) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto address = AddressTraits::GenerateAccountId();
		delta->addAccount(address, Height(1230));
		cache.commit();

		// Act:
		auto accountState = CreateAccountStateWithMismatchedAddressAndPublicKey();
		accountState.Address = address;
		delta->addAccount(accountState);
		const auto& addedAccountState = const_cast<const decltype(delta)&>(delta)->find(address).get();

		// Sanity:
		EXPECT_EQ(Height(1230), addedAccountState.AddressHeight);
		EXPECT_EQ(Height(0), addedAccountState.PublicKeyHeight);

		// Assert:
		EXPECT_TRUE(!!delta->addedElements().empty());
		EXPECT_TRUE(!!delta->modifiedElements().empty());
		EXPECT_TRUE(!!delta->removedElements().empty());
	}

	// endregion

	// region addAccount (AccountState)

	TEST(TEST_CLASS, AddAccountAutomaticallyOptimizesCurrencyMosaicAccess) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountState = CreateAccountStateWithMismatchedAddressAndPublicKey();

		// Act:
		delta->addAccount(accountState);
		AddThreeMosaicBalances(delta->find(accountState.Address).get());

		// Assert:
		EXPECT_EQ(Currency_Mosaic_Id, delta->find(accountState.Address).get().Balances.begin()->first);
	}

	TEST(TEST_CLASS, CanAddAccountViaAccountStateWithoutPublicKey) {
		// Arrange: note that public key height is 0
		auto accountState = CreateAccountStateWithMismatchedAddressAndPublicKey();
		accountState.PublicKeyHeight = Height(0);

		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();

		// Act:
		delta->addAccount(accountState);
		const auto* pAccountStateFromAddress = AddressTraits::Find(*delta, accountState.Address).tryGet();
		const auto* pAccountStateFromKey = PublicKeyTraits::Find(*delta, accountState.PublicKey).tryGet();

		// Assert:
		// - accountState properties should have been copied into the cache
		// - state is only accessible by address because public key height is 0
		ASSERT_TRUE(!!pAccountStateFromAddress);
		EXPECT_FALSE(!!pAccountStateFromKey);

		// - cache automatically optimizes added account state, so update to match expected
		accountState.Balances.optimize(Currency_Mosaic_Id);
		test::AssertEqual(accountState, *pAccountStateFromAddress, "pAccountStateFromAddress");
	}

	TEST(TEST_CLASS, CanAddAccountViaAccountStateWithPublicKey) {
		// Arrange: note that public key height is not 0
		auto accountState = CreateAccountStateWithMismatchedAddressAndPublicKey();
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();

		// Act:
		delta->addAccount(accountState);
		const auto* pAccountStateFromAddress = AddressTraits::Find(*delta, accountState.Address).tryGet();
		const auto* pAccountStateFromKey = PublicKeyTraits::Find(*delta, accountState.PublicKey).tryGet();

		// Assert:
		// - accountState properties should have been copied into the cache
		// - state is accessible by address and public key (note that state mapping, if present, is trusted)
		ASSERT_TRUE(!!pAccountStateFromAddress);
		ASSERT_TRUE(!!pAccountStateFromKey);

		// - cache automatically optimizes added account state, so update to match expected
		accountState.Balances.optimize(Currency_Mosaic_Id);
		test::AssertEqual(accountState, *pAccountStateFromAddress, "pAccountStateFromAddress");
		test::AssertEqual(accountState, *pAccountStateFromKey, "pAccountStateFromKey");
	}

	namespace {
		template<typename TAdd>
		void AssertAddAccountViaStateDoesNotOverrideKnownAccounts(TAdd add) {
			// Arrange:
			auto accountState = CreateAccountStateWithRandomAddressAndPublicKey();
			accountState.ImportanceInfo.set(Importance(777), model::ImportanceHeight(1));

			AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
			auto delta = cache.createDelta();

			// Act: add the state using add and set importance to 123
			add(*delta, accountState);
			auto& addedAccountState = delta->find(accountState.Address).get();
			addedAccountState.ImportanceInfo.set(Importance(123), model::ImportanceHeight(2));

			// - add the state again (with importance 777)
			delta->addAccount(accountState);

			// - get the state from the cache
			const auto* pAccountState = delta->find(accountState.Address).tryGet();

			// Assert: the second add had no effect (the importance is still 123)
			ASSERT_TRUE(!!pAccountState);
			EXPECT_EQ(&addedAccountState, pAccountState);
			EXPECT_EQ(Importance(123), pAccountState->ImportanceInfo.current());
		}
	}

	TEST(TEST_CLASS, AddAccountViaStateDoesNotOverrideKnownAccounts_Address) {
		// Assert:
		AssertAddAccountViaStateDoesNotOverrideKnownAccounts([](auto& delta, const auto& accountState) {
			delta.addAccount(accountState.Address, accountState.AddressHeight);
		});
	}

	TEST(TEST_CLASS, AddAccountViaStateDoesNotOverrideKnownAccounts_PublicKey) {
		// Assert:
		AssertAddAccountViaStateDoesNotOverrideKnownAccounts([](auto& delta, const auto& accountState) {
			delta.addAccount(accountState.PublicKey, accountState.PublicKeyHeight);
		});
	}

	TEST(TEST_CLASS, AddAccountViaStateDoesNotOverrideKnownAccounts_AccountState) {
		// Assert:
		AssertAddAccountViaStateDoesNotOverrideKnownAccounts([](auto& delta, const auto& accountState) {
			delta.addAccount(accountState);
		});
	}

	// endregion

	// region queueRemove / commitRemovals

	ID_BASED_TEST(Remove_RemovesExistingAccountIfHeightMatches) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto accountId = AddRandomAccount<TTraits>(cache);
		auto delta = cache.createDelta();

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountId, TTraits::DefaultHeight());
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(0u, delta->size());
		EXPECT_FALSE(!!utils::as_const(delta)->find(accountId).tryGet());
	}

	ID_BASED_TEST(Remove_DoesNotRemovesExistingAccountIfHeightDoesNotMatch) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		delta->addAccount(accountId, TTraits::DefaultHeight());
		const auto& expectedAccount = delta->find(accountId).get();

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountId, TTraits::DefaultHeight() + Height(1));
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(1u, delta->size());
		EXPECT_EQ(&expectedAccount, TTraits::Find(*delta, accountId).tryGet());
	}

	ID_BASED_TEST(Remove_CanBeCalledOnNonExistingAccount) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
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
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		delta->addAccount(accountId, TTraits::DefaultHeight());
		const auto& expectedAccount = delta->find(accountId).get();

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountId, TTraits::DefaultHeight());

		// Assert: the account was not removed yet
		EXPECT_EQ(1u, delta->size());
		EXPECT_EQ(&expectedAccount, TTraits::Find(*delta, accountId).tryGet());
	}

	ID_BASED_TEST(CommitRemovals_DoesNothingIfNoRemovalsHaveBeenQueued) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		delta->addAccount(TTraits::GenerateAccountId(), TTraits::DefaultHeight());

		// Act:
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(1u, delta->size());
	}

	ID_BASED_TEST(CommitRemovals_CanCommitQueuedRemovalsOfMultipleAccounts) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();

		// Act: add 5 (0..4) accounts and queue removals of two (1, 3)
		for (auto i = 0u; i < 5; ++i) {
			auto accountId = TTraits::GenerateAccountId();
			auto height = TTraits::DefaultHeight() + Height(i);
			delta->addAccount(accountId, height);

			if (1u == i % 2)
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
			EXPECT_EQ(delta.find(address).get().PublicKey, accountState.PublicKey);
		}

		void AssertRemoveByKeyAtHeight(Height removalHeight) {
			// Arrange:
			auto publicKey = GenerateRandomPublicKey();
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

			AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
			auto delta = cache.createDelta();
			AddAccountToCacheDelta<AddressTraits>(*delta, address, Amount(1234));
			AddAccountToCacheDelta<PublicKeyTraits>(*delta, publicKey, Amount());

			// Act:
			delta->queueRemove(publicKey, removalHeight);
			delta->commitRemovals();
			const auto* pAccountState = PublicKeyTraits::Find(utils::as_const(*delta), publicKey).tryGet();

			// Assert:
			if (PublicKeyTraits::DefaultHeight() != removalHeight)
				AssertState(PublicKeyTraits::DefaultHeight(), *delta, address, *pAccountState);
			else
				EXPECT_FALSE(!!pAccountState);
		}

		void AssertRemoveByAddressAtHeight(Height removalHeight) {
			// Arrange:
			auto publicKey = GenerateRandomPublicKey();
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

			AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
			auto delta = cache.createDelta();
			AddAccountToCacheDelta<AddressTraits>(*delta, address, Amount(1234));
			AddAccountToCacheDelta<PublicKeyTraits>(*delta, publicKey, Amount());

			// Act:
			delta->queueRemove(address, removalHeight);
			delta->commitRemovals();
			const auto* pAccountState = AddressTraits::Find(utils::as_const(*delta), address).tryGet();

			if (AddressTraits::DefaultHeight() == removalHeight) {
				EXPECT_EQ(0u, delta->size());
				EXPECT_FALSE(!!pAccountState);
				return;
			}

			AssertState(PublicKeyTraits::DefaultHeight(), *delta, address, *pAccountState);
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

		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
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
		EXPECT_FALSE(!!utils::as_const(delta)->find(address).tryGet());
	}

	namespace {
		template<typename TCache, typename TCacheQualifier>
		void AssertCanAccessAllAccountsThroughFindByAddress(TCache& cache, TCacheQualifier qualifier) {
			// Arrange:
			auto address1 = test::GenerateRandomAddress();
			auto address2 = test::GenerateRandomAddress();
			auto address3 = test::GenerateRandomAddress();
			auto address4 = test::GenerateRandomAddress();
			auto key5 = GenerateRandomPublicKey();
			auto address5 = model::PublicKeyToAddress(key5, Network_Identifier);

			auto accountState6 = CreateAccountStateWithRandomAddressAndPublicKey();
			auto accountState7 = CreateAccountStateWithRandomAddress();
			auto accountState8 = CreateAccountStateWithRandomAddressAndPublicKey();
			auto accountState9 = CreateAccountStateWithRandomAddress();

			cache.addAccount(address1, Height(100));
			cache.addAccount(address3, Height(100));
			cache.addAccount(key5, Height(100));

			cache.addAccount(accountState6);
			cache.addAccount(accountState7);

			// Act + Assert:
			auto& qualifiedCache = *qualifier(cache);
			EXPECT_TRUE(!!qualifiedCache.find(address1).tryGet());
			EXPECT_FALSE(qualifiedCache.contains(address2));
			EXPECT_TRUE(!!qualifiedCache.find(address3).tryGet());
			EXPECT_FALSE(qualifiedCache.contains(address4));
			EXPECT_TRUE(!!qualifiedCache.find(address5).tryGet()); // added by key but accessible via address

			EXPECT_TRUE(!!qualifiedCache.find(accountState6.Address).tryGet());
			EXPECT_TRUE(!!qualifiedCache.find(accountState7.Address).tryGet());
			EXPECT_FALSE(qualifiedCache.contains(accountState8.Address));
			EXPECT_FALSE(qualifiedCache.contains(accountState9.Address));
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

			auto accountState6 = CreateAccountStateWithRandomAddressAndPublicKey();
			auto accountState7 = CreateAccountStateWithRandomAddress();
			auto accountState8 = CreateAccountStateWithRandomAddressAndPublicKey();
			auto accountState9 = CreateAccountStateWithRandomAddress();

			cache.addAccount(key1, Height(100));
			cache.addAccount(key3, Height(100));
			cache.addAccount(address5, Height(100));

			cache.addAccount(accountState6);
			cache.addAccount(accountState7);

			// Act + Assert:
			auto& qualifiedCache = *qualifier(cache);
			EXPECT_TRUE(!!qualifiedCache.find(key1).tryGet());
			EXPECT_FALSE(qualifiedCache.contains(key2));
			EXPECT_TRUE(!!qualifiedCache.find(key3).tryGet());
			EXPECT_FALSE(qualifiedCache.contains(key4));
			EXPECT_FALSE(qualifiedCache.contains(key5)); // only added via address

			EXPECT_TRUE(!!qualifiedCache.find(accountState6.PublicKey).tryGet());
			EXPECT_FALSE(qualifiedCache.contains(accountState7.PublicKey)); // only added via address
			EXPECT_FALSE(qualifiedCache.contains(accountState8.PublicKey));
			EXPECT_FALSE(qualifiedCache.contains(accountState9.PublicKey));
		}
	}

	TEST(TEST_CLASS, CanAccessAllAccountsThroughFindByAddress) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();

		// Assert:
		AssertCanAccessAllAccountsThroughFindByAddress(*delta, [](auto& c) {
			return &const_cast<AccountStateCacheDelta&>(c);
		});
	}

	TEST(TEST_CLASS, CanAccessAllAccountsThroughFindConstByAddress) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();

		// Assert:
		AssertCanAccessAllAccountsThroughFindByAddress(*delta, [](auto& c) {
			return &const_cast<const AccountStateCacheDelta&>(c);
		});
	}

	TEST(TEST_CLASS, CanAccessAllAccountsThroughFindByPublicKey) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();

		// Assert:
		AssertCanAccessAllAccountsThroughFindByPublicKey(*delta, [](auto& c) {
			return &const_cast<AccountStateCacheDelta&>(c);
		});
	}

	TEST(TEST_CLASS, CanAccessAllAccountsThroughFindConstByPublicKey) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();

		// Assert:
		AssertCanAccessAllAccountsThroughFindByPublicKey(*delta, [](auto& c) {
			return &const_cast<const AccountStateCacheDelta&>(c);
		});
	}

	// endregion

	// region highValueAddresses

	namespace {
		std::vector<Address> AddAccountsWithBalances(AccountStateCacheDelta& delta, const std::vector<Amount>& balances) {
			auto addresses = test::GenerateRandomDataVector<Address>(balances.size());
			for (auto i = 0u; i < balances.size(); ++i) {
				delta.addAccount(addresses[i], Height(1));
				auto& accountState = delta.find(addresses[i]).get();
				accountState.Balances.credit(Harvesting_Mosaic_Id, balances[i]);
			}

			return addresses;
		}

		template<typename TAction>
		void RunHighValueAddressesTest(const std::vector<Amount>& balances, TAction action) {
			// Arrange: set min balance to 1M
			auto options = Default_Cache_Options;
			options.MinHighValueAccountBalance = Amount(1'000'000);
			AccountStateCache cache(CacheConfiguration(), options);

			// - prepare delta with requested accounts
			auto delta = cache.createDelta();
			auto addresses = AddAccountsWithBalances(*delta, balances);
			cache.commit();

			// Act: run the test
			action(addresses, delta, cache.createView());
		}
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsEmptySetWhenNoAccountsMeetCriteria) {
		// Arrange: add 0/3 with sufficient balance
		auto balances = std::vector<Amount>{ Amount(999'999), Amount(1'000), Amount(1) };
		RunHighValueAddressesTest(balances, [](const auto&, const auto& delta, const auto& view) {
			// Act + Assert:
			EXPECT_TRUE(delta->highValueAddresses().empty());
			EXPECT_TRUE(view->highValueAddresses().empty());
		});
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsOriginalAccountsMeetingCriteria) {
		// Arrange: add 2/3 accounts with sufficient balance
		auto balances = std::vector<Amount>{ Amount(1'100'000), Amount(900'000), Amount(1'000'000) };
		RunHighValueAddressesTest(balances, [](const auto& addresses, const auto& delta, const auto& view) {
			// Act + Assert:
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), delta->highValueAddresses());
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), view->highValueAddresses());
		});
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsAddedAccountsMeetingCriteria) {
		// Arrange:
		RunHighValueAddressesTest({}, [](const auto&, auto& delta, const auto& view) {
			// - add 2/3 accounts with sufficient balance (uncommitted)
			auto balances = std::vector<Amount>{ Amount(1'100'000), Amount(900'000), Amount(1'000'000) };
			auto addresses = AddAccountsWithBalances(*delta, balances);

			// Act + Assert:
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), delta->highValueAddresses());
			EXPECT_TRUE(view->highValueAddresses().empty());
		});
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsModifiedAccountsMeetingCriteria) {
		// Arrange: add 1/3 accounts with sufficient balance
		auto balances = std::vector<Amount>{ Amount(1'100'000 - 1), Amount(900'000 - 1), Amount(1'000'000 - 1) };
		RunHighValueAddressesTest(balances, [](const auto& addresses, auto& delta, const auto& view) {
			// - increment balances of all accounts (this will make 2/3 have sufficient balance)
			for (const auto& address : addresses)
				delta->find(address).get().Balances.credit(Harvesting_Mosaic_Id, Amount(1));

			// Act + Assert:
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), delta->highValueAddresses());
			EXPECT_EQ(model::AddressSet({ addresses[0] }), view->highValueAddresses());
		});
	}

	TEST(TEST_CLASS, HighValueAddressesDoesNotReturnRemovedAccountsMeetingCriteria) {
		// Arrange: add 2/3 accounts with sufficient balance
		auto balances = std::vector<Amount>{ Amount(1'100'000), Amount(900'000), Amount(1'000'000) };
		RunHighValueAddressesTest(balances, [](const auto& addresses, auto& delta, const auto& view) {
			// - remove high value accounts
			delta->queueRemove(addresses[0], Height(1));
			delta->queueRemove(addresses[2], Height(1));
			delta->commitRemovals();

			// Act + Assert:
			EXPECT_TRUE(delta->highValueAddresses().empty());
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), view->highValueAddresses());
		});
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsAllAccountsMeetingCriteria) {
		// Arrange: add 3/5 accounts with sufficient balance [3 match]
		auto balances = std::vector<Amount>{ Amount(1'100'000), Amount(900'000), Amount(1'000'000), Amount(800'000), Amount(1'200'000) };
		RunHighValueAddressesTest(balances, [](const auto& addresses, auto& delta, const auto& view) {
			// - add 2/3 accounts with sufficient balance (uncommitted) [5 match]
			auto uncommittedAddresses = AddAccountsWithBalances(*delta, { Amount(1'100'000), Amount(900'000), Amount(1'000'000) });

			// - modify two [5 match]
			delta->find(addresses[1]).get().Balances.credit(Harvesting_Mosaic_Id, Amount(100'000));
			delta->find(addresses[4]).get().Balances.debit(Harvesting_Mosaic_Id, Amount(200'001));

			// - delete two [3 match]
			delta->queueRemove(addresses[2], Height(1));
			delta->queueRemove(uncommittedAddresses[0], Height(1));
			delta->commitRemovals();

			// Act + Assert:
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[1], uncommittedAddresses[2] }), delta->highValueAddresses());
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2], addresses[4] }), view->highValueAddresses());
		});
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsAllAccountsMeetingCriteriaAfterDeltaChangesAreThrownAway) {
		// Arrange: set min balance to 1M
		auto options = Default_Cache_Options;
		options.MinHighValueAccountBalance = Amount(1'000'000);
		AccountStateCache cache(CacheConfiguration(), options);

		// - prepare delta with requested accounts
		std::vector<Address> addresses;
		{
			// - add 3/5 accounts with sufficient balance [3 match]
			auto delta = cache.createDelta();
			addresses = AddAccountsWithBalances(*delta, {
				Amount(1'100'000), Amount(900'000), Amount(1'000'000), Amount(800'000), Amount(1'200'000)
			});
			cache.commit();

			// - make changes to delta but do not commit
			// - add 2/3 accounts with sufficient balance (uncommitted) [5 match]
			auto uncommittedAddresses = AddAccountsWithBalances(*delta, { Amount(1'100'000), Amount(900'000), Amount(1'000'000) });

			// - modify two [5 match]
			delta->find(addresses[1]).get().Balances.credit(Harvesting_Mosaic_Id, Amount(100'000));
			delta->find(addresses[4]).get().Balances.debit(Harvesting_Mosaic_Id, Amount(200'001));

			// - delete two [3 match]
			delta->queueRemove(addresses[2], Height(1));
			delta->queueRemove(uncommittedAddresses[0], Height(1));
			delta->commitRemovals();
		}

		// - create a new delta and get addresses from it
		{
			auto delta = cache.createDelta();
			auto view = cache.createView();

			// Assert: only original accounts with highValue addresses are returned
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2], addresses[4] }), delta->highValueAddresses());
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2], addresses[4] }), view->highValueAddresses());
		}
	}

	// endregion

	// region cache init

	TEST(TEST_CLASS, CanSpecifyInitialValuesViaInit) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto addresses = test::GenerateRandomDataVector<Address>(3);
		auto addressSet = model::AddressSet(addresses.cbegin(), addresses.cend());

		// Act:
		cache.init(model::AddressSet(addressSet));

		// Assert:
		auto view = cache.createView();
		EXPECT_EQ(addressSet, view->highValueAddresses());
	}

	// endregion
}}
