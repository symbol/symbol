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
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountStateCacheTests

	// region key traits

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;
		constexpr auto Currency_Mosaic_Id = MosaicId(1234);
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);
		constexpr auto Default_Cache_Options = test::CreateDefaultAccountStateCacheOptions(Currency_Mosaic_Id, Harvesting_Mosaic_Id);

		struct AddressTraits {
			using Type = Address;

			static constexpr auto Default_Height = Height(321);

			static Type GenerateAccountIdentifier() {
				return test::GenerateRandomAddress();
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

			static constexpr auto Default_Height = Height(432);

			static Type GenerateAccountIdentifier() {
				return test::GenerateRandomByteArray<Key>();
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

			bool contains(const IdType& accountIdentifier) const {
				return m_delta->contains(accountIdentifier);
			}

			auto find(const IdType& accountIdentifier) {
				return m_delta->find(accountIdentifier);
			}

			auto find(const IdType& accountIdentifier) const {
				return m_delta->find(accountIdentifier);
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
		struct AccountStateCacheDeltaModificationPolicy : public test:: DeltaInsertModificationPolicy {
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
	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(TRAITS, AccountStateCacheDeltaModificationPolicy<TRAITS::IdTraits>, _Delta##SUFFIX)

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
			return PublicKeyTraits::GenerateAccountIdentifier();
		}

		Address ToAddress(const Key& publicKey) {
			return model::PublicKeyToAddress(publicKey, Network_Identifier);
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

		template<typename TKeyTraits>
		void AddAccountToCacheDelta(AccountStateCacheDelta& delta, const typename TKeyTraits::Type& accountIdentifier, Amount balance) {
			delta.addAccount(accountIdentifier, TKeyTraits::Default_Height);
			delta.find(accountIdentifier).get().Balances.credit(Harvesting_Mosaic_Id, balance);
		}

		state::AccountState CreateAccountStateWithRandomAddressAndPublicKey() {
			auto publicKey = GenerateRandomPublicKey();
			auto address = ToAddress(publicKey);

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

	// region cache properties

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
		EXPECT_EQ(networkIdentifier, cache.createDetachedDelta().tryLock()->networkIdentifier());
	}

	TEST(TEST_CLASS, CacheExposesImportanceGrouping) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.ImportanceGrouping = 234;
		AccountStateCache cache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(234u, cache.importanceGrouping());
	}

	TEST(TEST_CLASS, CacheWrappersExposeMinHarvesterBalance) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.MinHarvesterBalance = Amount(336644);
		AccountStateCache cache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(Amount(336644), cache.createView()->minHarvesterBalance());
		EXPECT_EQ(Amount(336644), cache.createDelta()->minHarvesterBalance());
		EXPECT_EQ(Amount(336644), cache.createDetachedDelta().tryLock()->minHarvesterBalance());
	}

	TEST(TEST_CLASS, CacheWrappersExposeMaxHarvesterBalance) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.MaxHarvesterBalance = Amount(446633);
		AccountStateCache cache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(Amount(446633), cache.createView()->maxHarvesterBalance());
		EXPECT_EQ(Amount(446633), cache.createDelta()->maxHarvesterBalance());
		EXPECT_EQ(Amount(446633), cache.createDetachedDelta().tryLock()->maxHarvesterBalance());
	}

	TEST(TEST_CLASS, CacheWrappersExposeHarvestingMosaicId) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.HarvestingMosaicId = MosaicId(11229988);
		AccountStateCache cache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(MosaicId(11229988), cache.createView()->harvestingMosaicId());
		EXPECT_EQ(MosaicId(11229988), cache.createDelta()->harvestingMosaicId());
		EXPECT_EQ(MosaicId(11229988), cache.createDetachedDelta().tryLock()->harvestingMosaicId());
	}

	// endregion

	// region tryGet (custom tests due to multiple views into the cache)

	TEST(TEST_CLASS, FindByKeyReturnsEmptyIteratorForUnknownPublicKeyButKnownAddress_View) {
		// Arrange:
		auto publicKey = GenerateRandomPublicKey();
		auto address = ToAddress(publicKey);

		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		{
			auto delta = cache.createDelta();
			delta->addAccount(address, Height());
			cache.commit();
		}

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
		auto address = ToAddress(publicKey);

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

	// region find (optimizedMosaicId bug regression)

	namespace {
		void AddThreeMosaicBalances(state::AccountState& accountState) {
			accountState.Balances.credit(MosaicId(1), Amount(10));
			accountState.Balances.credit(Harvesting_Mosaic_Id, Amount(20));
			accountState.Balances.credit(Currency_Mosaic_Id, Amount(30));
		}

		std::vector<MosaicId> GetMosaicIdsWithBalance(const state::AccountState& accountState) {
			std::vector<MosaicId> mosaicIds;
			for (const auto& pair : accountState.Balances)
				mosaicIds.push_back(pair.first);

			return mosaicIds;
		}
	}

	ID_BASED_TEST(FindByMutableAutomaticallyOptimizesCurrencyMosaicAccessWhenPresent) {
		// Arrange:
		test::TempDirectoryGuard dbDirGuard;
		auto cacheConfig = cache::CacheConfiguration(dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);

		AccountStateCache cache(cacheConfig, Default_Cache_Options);
		auto accountIdentifier = TTraits::GenerateAccountIdentifier();

		std::vector<MosaicId> mosaicIdsFromDelta;
		{
			// - add an account with three mosaics
			auto delta = cache.createDelta();
			delta->addAccount(accountIdentifier, TTraits::Default_Height);
			AddThreeMosaicBalances(delta->find(accountIdentifier).get());
			mosaicIdsFromDelta = GetMosaicIdsWithBalance(delta->find(accountIdentifier).get());

			cache.commit();
		}

		// Act:
		auto mosaicIdsFromView = GetMosaicIdsWithBalance(cache.createView()->find(accountIdentifier).get());

		// Assert: mosaics are ordered with currency mosaic id first
		EXPECT_EQ(3u, mosaicIdsFromDelta.size());
		EXPECT_EQ(std::vector<MosaicId>({ Currency_Mosaic_Id, MosaicId(1), Harvesting_Mosaic_Id }), mosaicIdsFromDelta);

		EXPECT_EQ(3u, mosaicIdsFromView.size());
		EXPECT_EQ(std::vector<MosaicId>({ Currency_Mosaic_Id, MosaicId(1), Harvesting_Mosaic_Id }), mosaicIdsFromView);
	}

	ID_BASED_TEST(FindByMutableAutomaticallyOptimizesCurrencyMosaicAccessWhenNotPresent) {
		// Arrange: this test needs to use rocksdb because it serializes / deserializes account states for each delta
		test::TempDirectoryGuard dbDirGuard;
		auto cacheConfig = cache::CacheConfiguration(dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);

		AccountStateCache cache(cacheConfig, Default_Cache_Options);
		auto accountIdentifier = TTraits::GenerateAccountIdentifier();

		std::vector<MosaicId> mosaicIdsFromDelta;
		{
			// - add an account without any mosaics
			auto delta = cache.createDelta();
			delta->addAccount(accountIdentifier, TTraits::Default_Height);
			cache.commit();
		}

		{
			// - add three mosaics to same account
			auto delta = cache.createDelta();
			AddThreeMosaicBalances(delta->find(accountIdentifier).get());
			mosaicIdsFromDelta = GetMosaicIdsWithBalance(delta->find(accountIdentifier).get());

			cache.commit();
		}

		// Act:
		auto mosaicIdsFromView = GetMosaicIdsWithBalance(cache.createView()->find(accountIdentifier).get());

		// Assert: mosaics are ordered with currency mosaic id first
		EXPECT_EQ(3u, mosaicIdsFromDelta.size());
		EXPECT_EQ(std::vector<MosaicId>({ Currency_Mosaic_Id, MosaicId(1), Harvesting_Mosaic_Id }), mosaicIdsFromDelta);

		EXPECT_EQ(3u, mosaicIdsFromView.size());
		EXPECT_EQ(std::vector<MosaicId>({ Currency_Mosaic_Id, MosaicId(1), Harvesting_Mosaic_Id }), mosaicIdsFromView);
	}

	// endregion

	// region addAccount (basic)

	ID_BASED_TEST(AddAccountChangesSizeOfCache) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		DefaultFillCache(cache, 10);
		auto delta = cache.createDelta();
		auto accountIdentifier = TTraits::GenerateAccountIdentifier();

		// Act:
		delta->addAccount(accountIdentifier, TTraits::Default_Height);

		// Assert:
		EXPECT_EQ(11u, delta->size());
		EXPECT_TRUE(!!delta->find(accountIdentifier).tryGet());
	}

	ID_BASED_TEST(AddAccountAutomaticallyOptimizesCurrencyMosaicAccess) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountIdentifier = TTraits::GenerateAccountIdentifier();

		// Act:
		delta->addAccount(accountIdentifier, TTraits::Default_Height);
		AddThreeMosaicBalances(delta->find(accountIdentifier).get());

		// Assert:
		EXPECT_EQ(Currency_Mosaic_Id, delta->find(accountIdentifier).get().Balances.begin()->first);
	}

	ID_BASED_TEST(SubsequentAddAccountHasNoEffect) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountIdentifier = TTraits::GenerateAccountIdentifier();
		delta->addAccount(accountIdentifier, TTraits::Default_Height);
		const auto& originalAddedAccountState = delta->find(accountIdentifier).get();

		// Act + Assert:
		for (auto i = 0u; i < 10; ++i) {
			delta->addAccount(accountIdentifier, Height(1235u + i));
			const auto& addedAccountState = delta->find(accountIdentifier).get();

			// - only first added account should change the state (and subsequent adds do not change height)
			EXPECT_EQ(&originalAddedAccountState, &addedAccountState);
			EXPECT_EQ(TTraits::Default_Height, addedAccountState.AddressHeight);
		}
	}

	TEST(TEST_CLASS, SubsequentAddAddressDoesNotMarkAccountAsDirty) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto address = AddressTraits::GenerateAccountIdentifier();
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
		auto address = ToAddress(publicKey);
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
		auto address = AddressTraits::GenerateAccountIdentifier();
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
			accountState.ImportanceSnapshots.set(Importance(777), model::ImportanceHeight(1));

			AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
			auto delta = cache.createDelta();

			// Act: add the state using add and set importance to 123
			add(*delta, accountState);
			auto& addedAccountState = delta->find(accountState.Address).get();
			addedAccountState.ImportanceSnapshots.set(Importance(123), model::ImportanceHeight(2));

			// - add the state again (with importance 777)
			delta->addAccount(accountState);

			// - get the state from the cache
			const auto* pAccountState = delta->find(accountState.Address).tryGet();

			// Assert: the second add had no effect (the importance is still 123)
			ASSERT_TRUE(!!pAccountState);
			EXPECT_EQ(&addedAccountState, pAccountState);
			EXPECT_EQ(Importance(123), pAccountState->ImportanceSnapshots.current());
		}
	}

	TEST(TEST_CLASS, AddAccountViaStateDoesNotOverrideKnownAccounts_Address) {
		AssertAddAccountViaStateDoesNotOverrideKnownAccounts([](auto& delta, const auto& accountState) {
			delta.addAccount(accountState.Address, accountState.AddressHeight);
		});
	}

	TEST(TEST_CLASS, AddAccountViaStateDoesNotOverrideKnownAccounts_PublicKey) {
		AssertAddAccountViaStateDoesNotOverrideKnownAccounts([](auto& delta, const auto& accountState) {
			delta.addAccount(accountState.PublicKey, accountState.PublicKeyHeight);
		});
	}

	TEST(TEST_CLASS, AddAccountViaStateDoesNotOverrideKnownAccounts_AccountState) {
		AssertAddAccountViaStateDoesNotOverrideKnownAccounts([](auto& delta, const auto& accountState) {
			delta.addAccount(accountState);
		});
	}

	TEST(TEST_CLASS, AddAccountAutomaticallyOverridesAnyExistingMosaicIdBalanceOptimization) {
		// Arrange:
		auto accountState = CreateAccountStateWithRandomAddressAndPublicKey();
		AddThreeMosaicBalances(accountState);
		accountState.Balances.optimize(test::GenerateRandomValue<MosaicId>());

		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();

		// Act:
		delta->addAccount(accountState);

		// Assert:
		EXPECT_EQ(Currency_Mosaic_Id, delta->find(accountState.Address).get().Balances.begin()->first);
	}

	// endregion

	// region queueRemove / clearRemove / commitRemovals

	ID_BASED_TEST(Remove_QueueRemoveRemovesExistingAccountWhenHeightMatches) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountIdentifier = TTraits::GenerateAccountIdentifier();
		delta->addAccount(accountIdentifier, TTraits::Default_Height);

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountIdentifier, TTraits::Default_Height);
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(0u, delta->size());
		EXPECT_FALSE(!!utils::as_const(delta)->find(accountIdentifier).tryGet());
	}

	ID_BASED_TEST(Remove_QueueRemoveDoesNotRemoveExistingAccountWhenHeightDoesNotMatch) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountIdentifier = TTraits::GenerateAccountIdentifier();
		delta->addAccount(accountIdentifier, TTraits::Default_Height);
		const auto& expectedAccount = delta->find(accountIdentifier).get();

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountIdentifier, TTraits::Default_Height + Height(1));
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(1u, delta->size());
		EXPECT_EQ(&expectedAccount, TTraits::Find(*delta, accountIdentifier).tryGet());
	}

	ID_BASED_TEST(Remove_QueueRemoveCanBeCalledOnNonexistentAccount) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		DefaultFillCache(cache, 10);
		auto delta = cache.createDelta();
		auto accountIdentifier = TTraits::GenerateAccountIdentifier();

		// Sanity:
		EXPECT_EQ(10u, delta->size());

		// Act:
		delta->queueRemove(accountIdentifier, Height(1236));
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(10u, delta->size());
	}

	ID_BASED_TEST(Remove_QueueRemoveDoesNotRemoveImmediately) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountIdentifier = TTraits::GenerateAccountIdentifier();
		delta->addAccount(accountIdentifier, TTraits::Default_Height);
		const auto& expectedAccount = delta->find(accountIdentifier).get();

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountIdentifier, TTraits::Default_Height);

		// Assert: the account was not removed yet
		EXPECT_EQ(1u, delta->size());
		EXPECT_EQ(&expectedAccount, TTraits::Find(*delta, accountIdentifier).tryGet());
	}

	ID_BASED_TEST(Remove_ClearRemovePreventsRemovalOfAccountQueuedForRemovalWhenHeightMatches) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountIdentifier = TTraits::GenerateAccountIdentifier();
		delta->addAccount(accountIdentifier, TTraits::Default_Height);
		const auto& expectedAccount = delta->find(accountIdentifier).get();

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountIdentifier, TTraits::Default_Height);
		delta->clearRemove(accountIdentifier, TTraits::Default_Height);
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(1u, delta->size());
		EXPECT_EQ(&expectedAccount, TTraits::Find(*delta, accountIdentifier).tryGet());
	}

	ID_BASED_TEST(Remove_ClearRemoveDoesNotPreventRemovalOfAccountQueuedForRemovalWhenHeightDoesNotMatch) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountIdentifier = TTraits::GenerateAccountIdentifier();
		delta->addAccount(accountIdentifier, TTraits::Default_Height);

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountIdentifier, TTraits::Default_Height);
		delta->clearRemove(accountIdentifier, TTraits::Default_Height + Height(1));
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(0u, delta->size());
		EXPECT_FALSE(!!utils::as_const(delta)->find(accountIdentifier).tryGet());
	}

	ID_BASED_TEST(Remove_CommitRemovalsDoesNothingWhenNoRemovalsHaveBeenQueued) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		delta->addAccount(TTraits::GenerateAccountIdentifier(), TTraits::Default_Height);

		// Act:
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(1u, delta->size());
	}

	ID_BASED_TEST(Remove_CommitRemovalsCanCommitQueuedRemovalsOfMultipleAccounts) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();

		// Act: add 5 (0..4) accounts and queue removals of two (1, 3)
		for (auto i = 0u; i < 5; ++i) {
			auto accountIdentifier = TTraits::GenerateAccountIdentifier();
			auto height = TTraits::Default_Height + Height(i);
			delta->addAccount(accountIdentifier, height);

			if (1u == i % 2)
				delta->queueRemove(accountIdentifier, height);
		}

		// Sanity:
		EXPECT_EQ(5u, delta->size());

		// Act:
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(3u, delta->size());
	}

	// endregion

	// region mixed state remove (public key added at a different height)

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
			auto address = ToAddress(publicKey);

			AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
			auto delta = cache.createDelta();
			AddAccountToCacheDelta<AddressTraits>(*delta, address, Amount(1234));
			AddAccountToCacheDelta<PublicKeyTraits>(*delta, publicKey, Amount());

			// Act:
			delta->queueRemove(publicKey, removalHeight);
			delta->commitRemovals();
			const auto* pAccountState = PublicKeyTraits::Find(utils::as_const(*delta), publicKey).tryGet();

			// Assert:
			if (PublicKeyTraits::Default_Height != removalHeight)
				AssertState(PublicKeyTraits::Default_Height, *delta, address, *pAccountState);
			else
				EXPECT_FALSE(!!pAccountState);
		}

		void AssertRemoveByAddressAtHeight(Height removalHeight) {
			// Arrange:
			auto publicKey = GenerateRandomPublicKey();
			auto address = ToAddress(publicKey);

			AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
			auto delta = cache.createDelta();
			AddAccountToCacheDelta<AddressTraits>(*delta, address, Amount(1234));
			AddAccountToCacheDelta<PublicKeyTraits>(*delta, publicKey, Amount());

			// Act:
			delta->queueRemove(address, removalHeight);
			delta->commitRemovals();
			const auto* pAccountState = AddressTraits::Find(utils::as_const(*delta), address).tryGet();

			if (AddressTraits::Default_Height == removalHeight) {
				EXPECT_EQ(0u, delta->size());
				EXPECT_FALSE(!!pAccountState);
				return;
			}

			AssertState(PublicKeyTraits::Default_Height, *delta, address, *pAccountState);
		}
	}

	TEST(TEST_CLASS, MixedState_Remove_ByKeyRemovesOnlyPublicKeyWhenHeightMatches) {
		AssertRemoveByKeyAtHeight(PublicKeyTraits::Default_Height);
	}

	TEST(TEST_CLASS, MixedState_Remove_ByKeyDoesNotRemoveWhenHeightDoesNotMatch) {
		AssertRemoveByKeyAtHeight(PublicKeyTraits::Default_Height + Height(1));
	}

	TEST(TEST_CLASS, MixedState_Remove_ByAddressRemovesAnAccountWhenHeightMatches) {
		AssertRemoveByAddressAtHeight(AddressTraits::Default_Height);
	}

	TEST(TEST_CLASS, MixedState_Remove_ByAddressDoesNotRemoveWhenHeightDoesNotMatch) {
		AssertRemoveByAddressAtHeight(AddressTraits::Default_Height + Height(1));
	}

	TEST(TEST_CLASS, CanQueueMultipleRemovalsOfSameAccount) {
		// Arrange:
		auto publicKey = GenerateRandomPublicKey();
		auto address = ToAddress(publicKey);

		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto delta = cache.createDelta();
		AddAccountToCacheDelta<AddressTraits>(*delta, address, Amount(1234));
		AddAccountToCacheDelta<PublicKeyTraits>(*delta, publicKey, Amount());

		// Act:
		delta->queueRemove(publicKey, PublicKeyTraits::Default_Height);
		delta->queueRemove(address, AddressTraits::Default_Height);
		delta->queueRemove(publicKey, PublicKeyTraits::Default_Height);
		delta->queueRemove(address, AddressTraits::Default_Height);
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(0u, delta->size());
		EXPECT_FALSE(!!utils::as_const(delta)->find(address).tryGet());
	}

	// endregion

	// region mixed state remove (public key added at same height)

	namespace {
		template<typename TAction>
		void PrepareCommitRemovalsTest(TAction action) {
			// Arrange:
			auto key = GenerateRandomPublicKey();
			auto address = model::PublicKeyToAddress(key, Network_Identifier);
			AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);

			// - add account as key and address at same height
			auto delta = cache.createDelta();
			delta->addAccount(key, Height(123));
			delta->addAccount(address, Height(123));

			// Act + Assert:
			action(*delta, key, address);
		}
	}

	TEST(TEST_CLASS, CommitRemovals_CanRemoveAccountByAddress_Linked) {
		// Arrange:
		PrepareCommitRemovalsTest([](auto& delta, const auto& key, const auto& address) {
			// Act:
			delta.queueRemove(address, Height(123));
			delta.commitRemovals(AccountStateCacheDelta::CommitRemovalsMode::Linked);

			// Assert:
			EXPECT_EQ(0u, delta.size());
			EXPECT_FALSE(delta.contains(key));
			EXPECT_FALSE(delta.contains(address));
		});
	}

	TEST(TEST_CLASS, CommitRemovals_CanRemoveAccountByPublicKey_Linked) {
		// Arrange:
		PrepareCommitRemovalsTest([](auto& delta, const auto& key, const auto& address) {
			// Act:
			delta.queueRemove(key, Height(123));
			delta.commitRemovals(AccountStateCacheDelta::CommitRemovalsMode::Linked);

			// Assert:
			EXPECT_EQ(0u, delta.size());
			EXPECT_FALSE(delta.contains(key));
			EXPECT_FALSE(delta.contains(address));
		});
	}

	TEST(TEST_CLASS, CommitRemovals_CanRemoveAccountByAddress_Unlinked) {
		// Arrange:
		PrepareCommitRemovalsTest([](auto& delta, const auto& key, const auto& address) {
			// Act:
			delta.queueRemove(address, Height(123));
			delta.commitRemovals(AccountStateCacheDelta::CommitRemovalsMode::Unlinked);

			// Assert:
			EXPECT_EQ(0u, delta.size());
			EXPECT_FALSE(delta.contains(key));
			EXPECT_FALSE(delta.contains(address));
		});
	}

	TEST(TEST_CLASS, CommitRemovals_CanRemoveAccountByPublicKey_Unlinked) {
		// Arrange:
		PrepareCommitRemovalsTest([](auto& delta, const auto& key, const auto& address) {
			// Act:
			delta.queueRemove(key, Height(123));
			delta.commitRemovals(AccountStateCacheDelta::CommitRemovalsMode::Unlinked);

			// Assert: only public key is removed
			EXPECT_EQ(1u, delta.size());
			EXPECT_FALSE(delta.contains(key));
			EXPECT_TRUE(delta.contains(address));

			auto accountStateIter = delta.find(address);
			const auto& accountState = accountStateIter.get();
			EXPECT_EQ(address, accountState.Address);
			EXPECT_EQ(Height(123), accountState.AddressHeight);
			EXPECT_EQ(Key(), accountState.PublicKey);
			EXPECT_EQ(Height(), accountState.PublicKeyHeight);
		});
	}

	// endregion

	// region mixed state find / contains

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
				accountState.SupplementalPublicKeys.voting().add(test::GenerateRandomPackedStruct<model::PinnedVotingKey>());
				accountState.Balances.credit(Harvesting_Mosaic_Id, balances[i]);
			}

			return addresses;
		}

		const auto& GetHighValueAddresses(const AccountStateCacheView& view) {
			return view.highValueAccounts().addresses();
		}

		const auto& CalculateHighValueAccounts(AccountStateCacheDelta& delta) {
			delta.updateHighValueAccounts(Height(1));
			return delta.highValueAccounts();
		}

		template<typename TDeltaAction, typename TViewAction>
		void RunHighValueAddressesTest(const std::vector<Amount>& balances, TDeltaAction deltaAction, TViewAction viewAction) {
			// Arrange: set min balance to 1M
			auto options = Default_Cache_Options;
			options.MinHarvesterBalance = Amount(1'000'000);
			AccountStateCache cache(CacheConfiguration(), options);

			// - prepare delta with requested accounts
			std::vector<Address> addresses;
			{
				{
					auto delta = cache.createDelta();
					addresses = AddAccountsWithBalances(*delta, balances);
					delta->updateHighValueAccounts(Height(1));
					cache.commit();
				}

				// Act + Assert: run the test (need a new delta because commit is destructive)
				auto delta = cache.createDelta();
				deltaAction(addresses, delta);
			}

			// Assert: check the view
			viewAction(addresses, cache.createView());
		}
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsEmptySetWhenNoAccountsMeetCriteria) {
		// Arrange:
		auto deltaAction = [](const auto&, auto& delta) {
			// Act + Assert:
			const auto& highValueAccounts = CalculateHighValueAccounts(*delta);
			EXPECT_TRUE(highValueAccounts.addresses().empty());
			EXPECT_TRUE(highValueAccounts.removedAddresses().empty());
		};
		auto viewAction = [](const auto&, const auto& view) {
			// Act + Assert:
			EXPECT_TRUE(GetHighValueAddresses(*view).empty());
		};

		// - add 0/3 with sufficient balance
		auto balances = std::vector<Amount>{ Amount(999'999), Amount(1'000), Amount(1) };
		RunHighValueAddressesTest(balances, deltaAction, viewAction);
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsOriginalAccountsMeetingCriteria) {
		// Arrange:
		auto deltaAction = [](const auto& addresses, auto& delta) {
			// Act + Assert:
			const auto& highValueAccounts = CalculateHighValueAccounts(*delta);
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), highValueAccounts.addresses());
			EXPECT_TRUE(highValueAccounts.removedAddresses().empty());
		};
		auto viewAction = [](const auto& addresses, const auto& view) {
			// Act + Assert:
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), GetHighValueAddresses(*view));
		};

		// - add 2/3 accounts with sufficient balance
		auto balances = std::vector<Amount>{ Amount(1'100'000), Amount(900'000), Amount(1'000'000) };
		RunHighValueAddressesTest(balances, deltaAction, viewAction);
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsAddedAccountsMeetingCriteria) {
		// Arrange:
		auto deltaAction = [](const auto&, auto& delta) {
			// - add 2/3 accounts with sufficient balance (uncommitted)
			auto balances = std::vector<Amount>{ Amount(1'100'000), Amount(900'000), Amount(1'000'000) };
			auto addresses = AddAccountsWithBalances(*delta, balances);

			// Act + Assert:
			const auto& highValueAccounts = CalculateHighValueAccounts(*delta);
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), highValueAccounts.addresses());
			EXPECT_TRUE(highValueAccounts.removedAddresses().empty());
		};
		auto viewAction = [](const auto&, const auto& view) {
			// Act + Assert:
			EXPECT_TRUE(GetHighValueAddresses(*view).empty());
		};

		RunHighValueAddressesTest({}, deltaAction, viewAction);
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsModifiedAccountsMeetingCriteria) {
		// Arrange:
		auto deltaAction = [](const auto& addresses, auto& delta) {
			// - increment balances of all accounts (this will make 2/3 have sufficient balance)
			for (const auto& address : addresses)
				delta->find(address).get().Balances.credit(Harvesting_Mosaic_Id, Amount(1));

			// Act + Assert:
			const auto& highValueAccounts = CalculateHighValueAccounts(*delta);
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), highValueAccounts.addresses());
			EXPECT_TRUE(highValueAccounts.removedAddresses().empty());
		};
		auto viewAction = [](const auto& addresses, const auto& view) {
			// Act + Assert:
			EXPECT_EQ(model::AddressSet({ addresses[0] }), GetHighValueAddresses(*view));
		};

		// - add 1/3 accounts with sufficient balance
		auto balances = std::vector<Amount>{ Amount(1'100'000 - 1), Amount(900'000 - 1), Amount(1'000'000 - 1) };
		RunHighValueAddressesTest(balances, deltaAction, viewAction);
	}

	TEST(TEST_CLASS, HighValueAddressesDoesNotReturnRemovedAccountsMeetingCriteria) {
		// Arrange:
		auto deltaAction = [](const auto& addresses, auto& delta) {
			// - remove high value accounts
			delta->queueRemove(addresses[0], Height(1));
			delta->queueRemove(addresses[2], Height(1));
			delta->commitRemovals();

			// Act + Assert:
			const auto& highValueAccounts = CalculateHighValueAccounts(*delta);
			EXPECT_TRUE(highValueAccounts.addresses().empty());
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), highValueAccounts.removedAddresses());
		};
		auto viewAction = [](const auto& addresses, const auto& view) {
			// Act + Assert:
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), GetHighValueAddresses(*view));
		};

		// - add 2/3 accounts with sufficient balance
		auto balances = std::vector<Amount>{ Amount(1'100'000), Amount(900'000), Amount(1'000'000) };
		RunHighValueAddressesTest(balances, deltaAction, viewAction);
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsAllAccountsMeetingCriteria) {
		// Arrange:
		auto deltaAction = [](const auto& addresses, auto& delta) {
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
			const auto& highValueAccounts = CalculateHighValueAccounts(*delta);
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[1], uncommittedAddresses[2] }), highValueAccounts.addresses());
			EXPECT_EQ(model::AddressSet({ addresses[2], addresses[4] }), highValueAccounts.removedAddresses());
		};
		auto viewAction = [](const auto& addresses, const auto& view) {
			// Act + Assert:
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2], addresses[4] }), GetHighValueAddresses(*view));
		};

		// - add 3/5 accounts with sufficient balance [3 match]
		auto balances = std::vector<Amount>{ Amount(1'100'000), Amount(900'000), Amount(1'000'000), Amount(800'000), Amount(1'200'000) };
		RunHighValueAddressesTest(balances, deltaAction, viewAction);
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsAllAccountsMeetingCriteriaAfterDeltaChangesAreThrownAway) {
		// Arrange: set min balance to 1M
		auto options = Default_Cache_Options;
		options.MinHarvesterBalance = Amount(1'000'000);
		AccountStateCache cache(CacheConfiguration(), options);

		// - prepare delta with requested accounts
		std::vector<Address> addresses;
		{
			{
				// - add 3/5 accounts with sufficient balance [3 match]
				auto delta = cache.createDelta();
				addresses = AddAccountsWithBalances(*delta, {
					Amount(1'100'000), Amount(900'000), Amount(1'000'000), Amount(800'000), Amount(1'200'000)
				});
				delta->updateHighValueAccounts(Height(1));
				cache.commit();
			}

			// - make changes to delta but do not commit
			// - add 2/3 accounts with sufficient balance (uncommitted) [5 match]
			auto delta = cache.createDelta();
			auto uncommittedAddresses = AddAccountsWithBalances(*delta, { Amount(1'100'000), Amount(900'000), Amount(1'000'000) });

			// - modify two [5 match]
			delta->find(addresses[1]).get().Balances.credit(Harvesting_Mosaic_Id, Amount(100'000));
			delta->find(addresses[4]).get().Balances.debit(Harvesting_Mosaic_Id, Amount(200'001));

			// - delete two [3 match]
			delta->queueRemove(addresses[2], Height(1));
			delta->queueRemove(uncommittedAddresses[0], Height(1));
			delta->commitRemovals();

			delta->updateHighValueAccounts(Height(1));
		}

		// Assert: only original accounts with highValue addresses are returned from new views
		{
			auto delta = cache.createDelta();
			const auto& highValueAccounts = CalculateHighValueAccounts(*delta);
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2], addresses[4] }), highValueAccounts.addresses());
			EXPECT_TRUE(highValueAccounts.removedAddresses().empty());
		}

		EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2], addresses[4] }), GetHighValueAddresses(*cache.createView()));
	}

	// endregion

	// region updateHighValueAccounts / detachHighValueAccounts

	TEST(TEST_CLASS, UpdateHighValueAccountsProcessesAccounts) {
		// Arrange: set min balance to 1M
		auto options = Default_Cache_Options;
		options.MinHarvesterBalance = Amount(1'000'000);
		AccountStateCache cache(CacheConfiguration(), options);

		// - prepare delta with 2/3 accounts with sufficient balance
		auto delta = cache.createDelta();
		auto addresses = AddAccountsWithBalances(*delta, { Amount(1'100'000), Amount(900'000), Amount(1'000'000) });

		// Sanity: before update, delta reports no addresses
		EXPECT_TRUE(delta->highValueAccounts().addresses().empty());

		// Act:
		delta->updateHighValueAccounts(Height(1));

		// Assert:
		EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), delta->highValueAccounts().addresses());
	}

	TEST(TEST_CLASS, DetachHighValueAccountsIsDestructive) {
		// Arrange: set min balance to 1M
		auto options = Default_Cache_Options;
		options.MinHarvesterBalance = Amount(1'000'000);
		AccountStateCache cache(CacheConfiguration(), options);

		// - prepare delta with 2/3 accounts with sufficient balance
		auto delta = cache.createDelta();
		auto addresses = AddAccountsWithBalances(*delta, { Amount(1'100'000), Amount(900'000), Amount(1'000'000) });
		delta->updateHighValueAccounts(Height(1));

		// Sanity: before detach, delta reports correct addresses
		EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), delta->highValueAccounts().addresses());

		// Act:
		auto detachedAccounts = delta->detachHighValueAccounts();

		// Assert:
		EXPECT_TRUE(delta->highValueAccounts().addresses().empty());

		EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), detachedAccounts.addresses());
	}

	// endregion

	// region prune

	namespace {
		void RunPruneTest(uint64_t grouping, Height pruneHeight, size_t expectedSizeAfterPrune) {
			// Arrange: set min voter balance to 1M
			auto options = Default_Cache_Options;
			options.VotingSetGrouping = grouping;
			options.MinVoterBalance = Amount(1'000'000);
			AccountStateCache cache(CacheConfiguration(), options);

			// - seed cache with 3/3 accounts with sufficient (voter) balance
			std::vector<Address> addresses;
			{
				auto delta = cache.createDelta();
				addresses = AddAccountsWithBalances(*delta, { Amount(1'100'000), Amount(1'200'000), Amount(1'000'000) });
				delta->updateHighValueAccounts(Height(3));
				cache.commit();
			}

			// - decrease the balance of two accounts below the threshold
			auto delta = cache.createDelta();
			delta->find(addresses[0]).get().Balances.debit(Harvesting_Mosaic_Id, Amount(300'000));
			delta->updateHighValueAccounts(Height(4));

			delta->find(addresses[2]).get().Balances.debit(Harvesting_Mosaic_Id, Amount(300'000));
			delta->updateHighValueAccounts(Height(6));

			// Sanity: before pruning, all three accounts are still tracked
			EXPECT_EQ(3u, delta->highValueAccounts().accountHistories().size());

			// Act:
			delta->prune(pruneHeight);

			// Assert: the account(s) decreased below the threshold should be untracked
			EXPECT_EQ(expectedSizeAfterPrune, delta->highValueAccounts().accountHistories().size())
					<< "grouping = " << grouping << ", pruneHeight " << pruneHeight;
		}
	}

	TEST(TEST_CLASS, CanPruneHighValueAccounts) {
		RunPruneTest(1, Height(4), 3); // prune <= 3
		RunPruneTest(1, Height(5), 2); // prune <= 4
		RunPruneTest(1, Height(6), 2); // prune <= 5
		RunPruneTest(1, Height(7), 1); // prune <= 6
		RunPruneTest(1, Height(8), 1); // prune <= 7
		RunPruneTest(1, Height(9), 1); // prune <= 8
	}

	TEST(TEST_CLASS, CanPruneHighValueAccounts_CustomGrouping) {
		RunPruneTest(4, Height(4), 3); // prune <= 1
		RunPruneTest(4, Height(5), 2); // prune <= 4
		RunPruneTest(4, Height(6), 2); // prune <= 4
		RunPruneTest(4, Height(7), 2); // prune <= 4
		RunPruneTest(4, Height(8), 2); // prune <= 4
		RunPruneTest(4, Height(9), 1); // prune <= 8
	}

	// endregion

	// region cache init

	TEST(TEST_CLASS, CanSpecifyInitialValuesViaInit) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto addresses = test::GenerateRandomDataVector<Address>(3);
		auto addressSet = model::AddressSet(addresses.cbegin(), addresses.cend());

		// Act:
		cache.init(HighValueAccounts(addressSet, AddressAccountHistoryMap()));

		// Assert:
		auto view = cache.createView();
		EXPECT_EQ(addressSet, GetHighValueAddresses(*view));
	}

	// endregion
}}
