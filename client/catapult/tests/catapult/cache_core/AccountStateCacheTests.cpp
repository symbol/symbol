#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/state/AccountStateAdapter.h"
#include "catapult/utils/Casting.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountStateCacheTests

	// region key traits

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

		constexpr auto Default_Cache_Options = AccountStateCacheTypes::Options{
			Network_Identifier,
			543,
			Amount(std::numeric_limits<Amount::ValueType>::max())
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
			static auto* TryGet(TCache& cache, const Address& address) {
				return cache.tryGet(address);
			}

			static Type ToKey(const state::AccountState& accountState) {
				return accountState.Address;
			}

			static void AssertFoundAccount(const Type& address, const state::AccountState& foundAccount) {
				EXPECT_EQ(AddressTraits::DefaultHeight(), foundAccount.AddressHeight);
				EXPECT_EQ(address, foundAccount.Address);
				EXPECT_EQ(Height(0), foundAccount.PublicKeyHeight);
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
			static auto* TryGet(TCache& cache, const Key& publicKey) {
				return cache.tryGet(publicKey);
			}

			static Type ToKey(const state::AccountState& accountState) {
				return accountState.PublicKey;
			}

			static void AssertFoundAccount(const Type& publicKey, const state::AccountState& foundAccount) {
				auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
				EXPECT_EQ(PublicKeyTraits::DefaultHeight(), foundAccount.AddressHeight);
				EXPECT_EQ(address, foundAccount.Address);
				EXPECT_EQ(PublicKeyTraits::DefaultHeight(), foundAccount.PublicKeyHeight);
				EXPECT_EQ(publicKey, foundAccount.PublicKey);
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

			auto get(const IdType& accountId) const {
				return m_delta->get(accountId);
			}

			auto tryGet(const IdType& accountId) const {
				return m_delta->tryGet(accountId);
			}

			void insert(const std::shared_ptr<state::AccountState>& pAccountState) {
				m_delta->addAccount(TTraits::ToKey(*pAccountState), Height(7));
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
			CacheProxy() : AccountStateCache(Default_Cache_Options)
			{}

		public:
			auto createDelta() {
				return std::make_unique<DeltaProxy<TTraits>>(AccountStateCache::createDelta());
			}
		};

		template<typename TTraits>
		struct AccountStateMixinTraits {
			using CacheType = CacheProxy<TTraits>;
			using IdType = typename TTraits::Type;
			using ValueType = std::shared_ptr<state::AccountState>;

			static uint8_t GetRawId(const IdType& id) {
				return id[0];
			}

			static IdType GetId(const std::pair<Key, Address>& pair) {
				return pair.first;
			}

			static IdType GetId(const state::AccountState& accountState) {
				return TTraits::ToKey(accountState);
			}

			static IdType GetId(const std::shared_ptr<state::AccountState>& pAccountState) {
				return TTraits::ToKey(*pAccountState);
			}

			static IdType MakeId(uint8_t id) {
				return IdType{ { id } };
			}

			static ValueType CreateWithId(uint8_t id) {
				// store same id in both Address and PublicKey so these traits work for both
				auto pAccountState = std::make_shared<state::AccountState>(Address{ { id } }, Height());
				pAccountState->PublicKey = { { id } };
				return pAccountState;
			}
		};
	}

#define DEFINE_ACCOUNT_STATE_CACHE_TESTS(TRAITS, SUFFIX) \
	DEFINE_CACHE_CONTAINS_TESTS(TRAITS, ViewAccessor, _View##SUFFIX); \
	DEFINE_CACHE_CONTAINS_TESTS(TRAITS, DeltaAccessor, _Delta##SUFFIX); \
	\
	DEFINE_CACHE_ACCESSOR_TESTS(TRAITS, ViewAccessor, MutableAccessor, _ViewMutable##SUFFIX); \
	DEFINE_CACHE_ACCESSOR_TESTS(TRAITS, ViewAccessor, ConstAccessor, _ViewConst##SUFFIX); \
	DEFINE_CACHE_ACCESSOR_TESTS(TRAITS, DeltaAccessor, MutableAccessor, _DeltaMutable##SUFFIX); \
	DEFINE_CACHE_ACCESSOR_TESTS(TRAITS, DeltaAccessor, ConstAccessor, _DeltaConst##SUFFIX); \
	\
	DEFINE_DELTA_ELEMENTS_MIXIN_TESTS(TRAITS, _Delta##SUFFIX);

	DEFINE_ACCOUNT_STATE_CACHE_TESTS(AccountStateMixinTraits<AddressTraits>, _Address);

	// only address-based iteration is supported
	DEFINE_CACHE_ITERATION_TESTS(AccountStateMixinTraits<AddressTraits>, ViewAccessor, _View_Address);

	DEFINE_ACCOUNT_STATE_CACHE_TESTS(AccountStateMixinTraits<PublicKeyTraits>, _PublicKey);

	// only one set of basic tests are needed
	DEFINE_CACHE_BASIC_TESTS(AccountStateMixinTraits<AddressTraits>,)

	// endregion

	// *** custom tests ***

	namespace {
		Key GenerateRandomPublicKey() {
			return PublicKeyTraits::GenerateAccountId();
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

		template<typename TKeyTraits>
		state::AccountState* AddAccountToCacheDelta(AccountStateCacheDelta& delta, const typename TKeyTraits::Type& key, Amount balance) {
			auto& accountState = delta.addAccount(key, TKeyTraits::DefaultHeight());
			accountState.Balances.credit(Xem_Id, balance);
			return &accountState;
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
		AccountStateCache cache({ networkIdentifier, 234, Amount() });

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, cache.networkIdentifier());
	}

	TEST(TEST_CLASS, CacheWrappersExposeNetworkIdentifier) {
		// Arrange:
		auto networkIdentifier = static_cast<model::NetworkIdentifier>(18);
		AccountStateCache cache({ networkIdentifier, 543, Amount() });

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, cache.createView()->networkIdentifier());
		EXPECT_EQ(networkIdentifier, cache.createDelta()->networkIdentifier());
		EXPECT_EQ(networkIdentifier, cache.createDetachedDelta().lock()->networkIdentifier());
	}

	TEST(TEST_CLASS, CacheExposesImportanceGrouping) {
		// Arrange:
		AccountStateCache cache({ static_cast<model::NetworkIdentifier>(17), 234, Amount() });

		// Act + Assert:
		EXPECT_EQ(234u, cache.importanceGrouping());
	}

	TEST(TEST_CLASS, CacheWrappersExposeImportanceGrouping) {
		// Arrange:
		AccountStateCache cache({ static_cast<model::NetworkIdentifier>(18), 543, Amount() });

		// Act + Assert:
		EXPECT_EQ(543u, cache.createView()->importanceGrouping());
		EXPECT_EQ(543u, cache.createDelta()->importanceGrouping());
		EXPECT_EQ(543u, cache.createDetachedDelta().lock()->importanceGrouping());
	}

	// endregion

	// region tryGet (custom tests due to multiple views into the cache)

	ID_BASED_TEST(TryGetReturnsSameStateAsAdd) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		const auto& addedAccount = delta->addAccount(accountId, TTraits::DefaultHeight());
		cache.commit();

		auto view = cache.createView();

		// Act:
		const auto* pFoundAccount = TTraits::TryGet(*view, accountId);

		// Assert:
		EXPECT_EQ(1u, view->size());
		EXPECT_EQ(&addedAccount, pFoundAccount);
		EXPECT_EQ(Amount(0), GetBalance(*pFoundAccount));

		TTraits::AssertFoundAccount(accountId, *pFoundAccount);
	}

	TEST(TEST_CLASS, TryGetByKeyReturnsNullptrForUnknownPublicKeyButKnownAddress_View) {
		// Arrange:
		auto publicKey = GenerateRandomPublicKey();
		auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();
		delta->addAccount(address, Height());
		cache.commit();

		auto view = cache.createView();

		// Act:
		const auto* pAccountState = PublicKeyTraits::TryGet(*view, publicKey);

		// Assert:
		EXPECT_EQ(1u, view->size());
		EXPECT_FALSE(!!pAccountState);
	}

	TEST(TEST_CLASS, TryGetByKeyConstReturnsNullptrForUnknownPublicKeyButKnownAddress_Delta) {
		// Arrange:
		auto publicKey = GenerateRandomPublicKey();
		auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();
		delta->addAccount(address, Height());
		cache.commit();

		// Act:
		const auto* pAccountState = PublicKeyTraits::TryGet(utils::as_const(*delta), publicKey);

		// Assert:
		EXPECT_EQ(1u, delta->size());
		EXPECT_FALSE(!!pAccountState);
	}

	// endregion

	// region addAccount (basic)

	ID_BASED_TEST(AddAccountChangesSizeOfCache) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		DefaultFillCache(cache, 10);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();

		// Act:
		delta->addAccount(accountId, TTraits::DefaultHeight());

		// Assert:
		EXPECT_EQ(11u, delta->size());
		EXPECT_TRUE(!!delta->tryGet(accountId));
	}

	ID_BASED_TEST(SubsequentAddAccountsReturnExistingAccount) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		const auto& addedAccount = delta->addAccount(accountId, TTraits::DefaultHeight());

		// Act + Assert:
		for (auto i = 0u; i < 10; ++i) {
			EXPECT_EQ(&addedAccount, &delta->addAccount(accountId, Height(1235u + i)));

			// The height is not supposed to change after multiple adds
			EXPECT_EQ(TTraits::DefaultHeight(), addedAccount.AddressHeight);
		}
	}

	// endregion

	// region queueRemove / commitRemovals

	ID_BASED_TEST(Remove_RemovesExistingAccountIfHeightMatches) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto accountId = AddRandomAccount<TTraits>(cache);
		auto delta = cache.createDelta();

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountId, TTraits::DefaultHeight());
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(0u, delta->size());
		EXPECT_FALSE(!!utils::as_const(delta)->tryGet(accountId));
	}

	ID_BASED_TEST(Remove_DoesNotRemovesExistingAccountIfHeightDoesNotMatch) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		const auto& expectedAccount = delta->addAccount(accountId, TTraits::DefaultHeight());

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountId, TTraits::DefaultHeight() + Height(1));
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(1u, delta->size());
		EXPECT_EQ(&expectedAccount, TTraits::TryGet(*delta, accountId));
	}

	ID_BASED_TEST(Remove_CanBeCalledOnNonExistingAccount) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
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
		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();
		auto accountId = TTraits::GenerateAccountId();
		const auto& expectedAccount = delta->addAccount(accountId, TTraits::DefaultHeight());

		// Sanity:
		EXPECT_EQ(1u, delta->size());

		// Act:
		delta->queueRemove(accountId, TTraits::DefaultHeight());

		// Assert: the account was not removed yet
		EXPECT_EQ(1u, delta->size());
		EXPECT_EQ(&expectedAccount, TTraits::TryGet(*delta, accountId));
	}

	ID_BASED_TEST(CommitRemovals_DoesNothingIfNoRemovalsHaveBeenQueued) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();
		delta->addAccount(TTraits::GenerateAccountId(), TTraits::DefaultHeight());

		// Act:
		delta->commitRemovals();

		// Assert:
		EXPECT_EQ(1u, delta->size());
	}

	ID_BASED_TEST(CommitRemovals_CanCommitQueuedRemovalsOfMultipleAccounts) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
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
			EXPECT_EQ(delta.get(address).PublicKey, accountState.PublicKey);
		}

		void AssertRemoveByKeyAtHeight(Height removalHeight) {
			// Arrange:
			auto publicKey = GenerateRandomPublicKey();
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

			AccountStateCache cache(Default_Cache_Options);
			auto delta = cache.createDelta();
			AddAccountToCacheDelta<AddressTraits>(*delta, address, Amount(1234));
			AddAccountToCacheDelta<PublicKeyTraits>(*delta, publicKey, Amount());

			// Act:
			delta->queueRemove(publicKey, removalHeight);
			delta->commitRemovals();
			const auto* pAccountState = PublicKeyTraits::TryGet(utils::as_const(*delta), publicKey);

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

			AccountStateCache cache(Default_Cache_Options);
			auto delta = cache.createDelta();
			AddAccountToCacheDelta<AddressTraits>(*delta, address, Amount(1234));
			AddAccountToCacheDelta<PublicKeyTraits>(*delta, publicKey, Amount());

			// Act:
			delta->queueRemove(address, removalHeight);
			delta->commitRemovals();
			const auto* pAccountState = AddressTraits::TryGet(utils::as_const(*delta), address);

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

		AccountStateCache cache(Default_Cache_Options);
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
		EXPECT_FALSE(!!utils::as_const(delta)->tryGet(address));
	}

	namespace {
		model::AccountInfo CreateRandomAccountInfoWithKey() {
			model::AccountInfo info;
			info.Size = sizeof(model::AccountInfo);
			info.MosaicsCount = 0;

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
			EXPECT_TRUE(!!qualifiedCache.tryGet(address1));
			EXPECT_FALSE(qualifiedCache.contains(address2));
			EXPECT_TRUE(!!qualifiedCache.tryGet(address3));
			EXPECT_FALSE(qualifiedCache.contains(address4));
			EXPECT_TRUE(!!qualifiedCache.tryGet(address5)); // added by key but accessible via address

			EXPECT_TRUE(!!qualifiedCache.tryGet(info6.Address));
			EXPECT_TRUE(!!qualifiedCache.tryGet(info7.Address));
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
			EXPECT_TRUE(!!qualifiedCache.tryGet(key1));
			EXPECT_FALSE(qualifiedCache.contains(key2));
			EXPECT_TRUE(!!qualifiedCache.tryGet(key3));
			EXPECT_FALSE(qualifiedCache.contains(key4));
			EXPECT_FALSE(qualifiedCache.contains(key5)); // only added via address

			EXPECT_TRUE(!!qualifiedCache.tryGet(info6.PublicKey));
			EXPECT_FALSE(qualifiedCache.contains(info7.PublicKey)); // only added via address
			EXPECT_FALSE(qualifiedCache.contains(info8.PublicKey));
			EXPECT_FALSE(qualifiedCache.contains(info9.PublicKey));
		}
	}

	TEST(TEST_CLASS, CanAccessAllAccountsThroughFindByAddress) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();

		// Assert:
		AssertCanAccessAllAccountsThroughFindByAddress(*delta, [](auto& c) {
			return &const_cast<AccountStateCacheDelta&>(c);
		});
	}

	TEST(TEST_CLASS, CanAccessAllAccountsThroughFindConstByAddress) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();

		// Assert:
		AssertCanAccessAllAccountsThroughFindByAddress(*delta, [](auto& c) {
			return &const_cast<const AccountStateCacheDelta&>(c);
		});
	}

	TEST(TEST_CLASS, CanAccessAllAccountsThroughFindByPublicKey) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();

		// Assert:
		AssertCanAccessAllAccountsThroughFindByPublicKey(*delta, [](auto& c) {
			return &const_cast<AccountStateCacheDelta&>(c);
		});
	}

	TEST(TEST_CLASS, CanAccessAllAccountsThroughFindConstByPublicKey) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
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
			info.MosaicsCount = 0;
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

		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();

		// Act:
		const auto& accountState = delta->addAccount(info);
		const auto* pAccountStateFromAddress = AddressTraits::TryGet(*delta, info.Address);
		const auto* pAccountStateFromKey = PublicKeyTraits::TryGet(*delta, info.PublicKey);

		// Assert:
		// - info properties should have been copied into the state
		// - state is only accessible by address because public key height is 0
		ASSERT_TRUE(!!pAccountStateFromAddress);
		EXPECT_FALSE(!!pAccountStateFromKey);

		AssertEqual(info, accountState, "accountState");
		AssertEqual(info, *pAccountStateFromAddress, "pAccountStateFromAddress");
	}

	TEST(TEST_CLASS, CanAddAccountViaAccountInfoWithPublicKey) {
		// Arrange: note that public key height is not 0
		auto info = CreateInconsistentAccountInfo();
		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();

		// Act:
		const auto& accountState = delta->addAccount(info);
		const auto* pAccountStateFromAddress = AddressTraits::TryGet(*delta, info.Address);
		const auto* pAccountStateFromKey = PublicKeyTraits::TryGet(*delta, info.PublicKey);

		// Assert:
		// - info properties should have been copied into the state
		// - state is accessible by address and public key (note that info mapping, if present, is trusted)
		ASSERT_TRUE(!!pAccountStateFromAddress);
		ASSERT_TRUE(!!pAccountStateFromKey);

		AssertEqual(info, accountState, "accountState");
		AssertEqual(info, *pAccountStateFromAddress, "pAccountStateFromAddress");
		AssertEqual(info, *pAccountStateFromKey, "pAccountStateFromKey");
	}

	namespace {
		template<typename TAdd>
		void AssertAddAccountViaInfoDoesNotOverrideKnownAccounts(TAdd add) {
			// Arrange: create an info
			auto info = CreateRandomAccountInfoWithKey();
			info.Importances[0] = Importance(777);

			AccountStateCache cache(Default_Cache_Options);
			auto delta = cache.createDelta();

			// Act: add the info using add and set importance to 123
			auto& addState1 = add(*delta, info);
			addState1.ImportanceInfo.set(Importance(123), model::ImportanceHeight(1));

			// - add the info again (with importance 777)
			const auto& addState2 = delta->addAccount(info);

			// - get the info from the cache
			const auto* pAccountState = delta->tryGet(info.Address);

			// Assert: the second add had no effect (the importance is still 123)
			ASSERT_TRUE(!!pAccountState);
			EXPECT_EQ(&addState1, pAccountState);
			EXPECT_EQ(&addState2, pAccountState);
			EXPECT_EQ(Importance(123), pAccountState->ImportanceInfo.current());
		}
	}

	TEST(TEST_CLASS, AddAccountViaInfoDoesNotOverrideKnownAccounts_Address) {
		// Assert:
		AssertAddAccountViaInfoDoesNotOverrideKnownAccounts([](auto& delta, const auto& info) -> state::AccountState& {
			return delta.addAccount(info.Address, info.AddressHeight);
		});
	}

	TEST(TEST_CLASS, AddAccountViaInfoDoesNotOverrideKnownAccounts_PublicKey) {
		// Assert:
		AssertAddAccountViaInfoDoesNotOverrideKnownAccounts([](auto& delta, const auto& info) -> state::AccountState& {
			return delta.addAccount(info.PublicKey, info.PublicKeyHeight);
		});
	}

	TEST(TEST_CLASS, AddAccountViaInfoDoesNotOverrideKnownAccounts_Info) {
		// Assert:
		AssertAddAccountViaInfoDoesNotOverrideKnownAccounts([](auto& delta, const auto& info) -> state::AccountState& {
			return delta.addAccount(info);
		});
	}

	// endregion

	// region highValueAddresses

	namespace {
		std::vector<Address> AddAccountsWithBalances(AccountStateCacheDelta& delta, const std::vector<Amount>& balances) {
			auto addresses = test::GenerateRandomDataVector<Address>(balances.size());
			for (auto i = 0u; i < balances.size(); ++i) {
				auto& accountState = delta.addAccount(addresses[i], Height(1));
				accountState.Balances.credit(Xem_Id, balances[i]);
			}

			return addresses;
		}

		template<typename TAction>
		void RunHighValueAddressesTest(const std::vector<Amount>& balances, TAction action) {
			// Arrange: set min balance to 1M
			auto config = Default_Cache_Options;
			config.MinHighValueAccountBalance = Amount(1'000'000);
			AccountStateCache cache(config);

			// - prepare delta with requested accounts
			auto delta = cache.createDelta();
			auto addresses = AddAccountsWithBalances(*delta, balances);
			cache.commit();

			// Act: run the test
			action(addresses, delta);
		}
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsEmptySetWhenNoAccountsMeetCriteria) {
		// Arrange: add 0/3 with sufficient balance
		auto balances = std::vector<Amount>{ Amount(999'999), Amount(1'000), Amount(1) };
		RunHighValueAddressesTest(balances, [](const auto&, const auto& delta) {
			// Act:
			auto highValueAddresses = delta->highValueAddresses();

			// Assert:
			EXPECT_TRUE(highValueAddresses.empty());
		});
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsOriginalAccountsMeetingCriteria) {
		// Arrange: add 2/3 accounts with sufficient balance
		auto balances = std::vector<Amount>{ Amount(1'100'000), Amount(900'000), Amount(1'000'000) };
		RunHighValueAddressesTest(balances, [](const auto& addresses, const auto& delta) {
			// Act:
			auto highValueAddresses = delta->highValueAddresses();

			// Assert:
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), highValueAddresses);
		});
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsAddedAccountsMeetingCriteria) {
		// Arrange:
		RunHighValueAddressesTest({}, [](const auto&, auto& delta) {
			// - add 2/3 accounts with sufficient balance (uncommitted)
			auto balances = std::vector<Amount>{ Amount(1'100'000), Amount(900'000), Amount(1'000'000) };
			auto addresses = AddAccountsWithBalances(*delta, balances);

			// Act:
			auto highValueAddresses = delta->highValueAddresses();

			// Assert:
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), highValueAddresses);
		});
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsModifiedAccountsMeetingCriteria) {
		// Arrange: add 1/3 accounts with sufficient balance
		auto balances = std::vector<Amount>{ Amount(1'100'000 - 1), Amount(900'000 - 1), Amount(1'000'000 - 1) };
		RunHighValueAddressesTest(balances, [](const auto& addresses, auto& delta) {
			// - increment balances of all accounts (this will make 2/3 have sufficient balance)
			for (const auto& address : addresses)
				delta->get(address).Balances.credit(Xem_Id, Amount(1));

			// Act:
			auto highValueAddresses = delta->highValueAddresses();

			// Assert:
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2] }), highValueAddresses);
		});
	}

	TEST(TEST_CLASS, HighValueAddressesDoesNotReturnRemovedAccountsMeetingCriteria) {
		// Arrange: add 2/3 accounts with sufficient balance
		auto balances = std::vector<Amount>{ Amount(1'100'000), Amount(900'000), Amount(1'000'000) };
		RunHighValueAddressesTest(balances, [](const auto& addresses, auto& delta) {
			// - remove high value accounts
			delta->queueRemove(addresses[0], Height(1));
			delta->queueRemove(addresses[2], Height(1));
			delta->commitRemovals();

			// Act:
			auto highValueAddresses = delta->highValueAddresses();

			// Assert:
			EXPECT_TRUE(highValueAddresses.empty());
		});
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsAllAccountsMeetingCriteria) {
		// Arrange: add 3/5 accounts with sufficient balance [3 match]
		auto balances = std::vector<Amount>{ Amount(1'100'000), Amount(900'000), Amount(1'000'000), Amount(800'000), Amount(1'200'000) };
		RunHighValueAddressesTest(balances, [](const auto& addresses, auto& delta) {
			// - add 2/3 accounts with sufficient balance (uncommitted) [5 match]
			auto uncommittedAddresses = AddAccountsWithBalances(*delta, { Amount(1'100'000), Amount(900'000), Amount(1'000'000) });

			// - modify two [5 match]
			delta->get(addresses[1]).Balances.credit(Xem_Id, Amount(100'000));
			delta->get(addresses[4]).Balances.debit(Xem_Id, Amount(200'001));

			// - delete two [3 match]
			delta->queueRemove(addresses[2], Height(1));
			delta->queueRemove(uncommittedAddresses[0], Height(1));
			delta->commitRemovals();

			// Act:
			auto highValueAddresses = delta->highValueAddresses();

			// Assert:
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[1], uncommittedAddresses[2] }), highValueAddresses);
		});
	}

	TEST(TEST_CLASS, HighValueAddressesReturnsAllAccountsMeetingCriteriaAfterDeltaChangesAreThrownAway) {
		// Arrange: set min balance to 1M
		auto config = Default_Cache_Options;
		config.MinHighValueAccountBalance = Amount(1'000'000);
		AccountStateCache cache(config);

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
			delta->get(addresses[1]).Balances.credit(Xem_Id, Amount(100'000));
			delta->get(addresses[4]).Balances.debit(Xem_Id, Amount(200'001));

			// - delete two [3 match]
			delta->queueRemove(addresses[2], Height(1));
			delta->queueRemove(uncommittedAddresses[0], Height(1));
			delta->commitRemovals();
		}

		// - create a new delta and get addresses from it
		{
			auto delta = cache.createDelta();

			// Act:
			auto highValueAddresses = delta->highValueAddresses();

			// Assert: only original accounts with highValue addresses are returned
			EXPECT_EQ(model::AddressSet({ addresses[0], addresses[2], addresses[4] }), highValueAddresses);
		}
	}

	// endregion
}}
