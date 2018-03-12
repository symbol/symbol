#include "catapult/cache_core/ImportanceView.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/model/NetworkInfo.h"
#include "tests/TestHarness.h"

using catapult::model::ImportanceHeight;
using catapult::model::ConvertToImportanceHeight;

namespace catapult { namespace cache {

#define TEST_CLASS ImportanceViewTests

	namespace {
		constexpr auto Default_Cache_Options = AccountStateCacheTypes::Options{
			model::NetworkIdentifier::Mijin_Test,
			123,
			Amount(std::numeric_limits<Amount::ValueType>::max())
		};

		struct AddressTraits {
			static auto& AddAccount(AccountStateCacheDelta& delta, const Key& publicKey, Height height) {
				return delta.addAccount(model::PublicKeyToAddress(publicKey, Default_Cache_Options.NetworkIdentifier), height);
			}
		};

		struct PublicKeyTraits {
			static auto& AddAccount(AccountStateCacheDelta& delta, const Key& publicKey, Height height) {
				return delta.addAccount(publicKey, height);
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
			auto& accountState = TTraits::AddAccount(*delta, publicKey, Height(100));
			accountState.ImportanceInfo.set(importance, importanceHeight);
			accountState.Balances.credit(Xem_Id, balance);
			cache.commit();
		}

		class ImportanceViewWrapper {
		public:
			explicit ImportanceViewWrapper(const AccountStateCache& cache)
					: m_cacheView(cache.createView())
					, m_readOnlyCache(*m_cacheView)
					, m_view(m_readOnlyCache)
			{}

		public:
			const ImportanceView& operator*() {
				return m_view;
			}

			const ImportanceView* operator->() {
				return &m_view;
			}

		private:
			LockedCacheView<AccountStateCacheView> m_cacheView;
			ReadOnlyAccountStateCache m_readOnlyCache;
			ImportanceView m_view;
		};

		ImportanceViewWrapper CreateImportanceView(const AccountStateCache& cache) {
			return ImportanceViewWrapper(cache);
		}

		auto ConvertToImportanceHeight(Height height) {
			return model::ConvertToImportanceHeight(height, Default_Cache_Options.ImportanceGrouping);
		}

		auto CreateAccountStateCache() {
			return std::make_unique<AccountStateCache>(Default_Cache_Options);
		}
	}

#define KEY_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_PublicKey) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PublicKeyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region tryGetAccountImportance / getAccountImportanceOrDefault

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
		auto key = test::GenerateRandomData<Key_Size>();
		auto height = Height(1000);
		auto pCache = CreateAccountStateCache();
		AddAccount<TTraits>(*pCache, key, Importance(1000), ConvertToImportanceHeight(height));
		auto pView = CreateImportanceView(*pCache);

		// Act + Assert: mismatched key
		AssertCannotFindImportance(*pView, test::GenerateRandomData<Key_Size>(), height);
	}

	KEY_TRAITS_BASED_TEST(CannotRetrieveImportanceForAccountAtMismatchedHeight) {
		// Arrange:
		auto key = test::GenerateRandomData<Key_Size>();
		auto pCache = CreateAccountStateCache();
		AddAccount<TTraits>(*pCache, key, Importance(1000), ConvertToImportanceHeight(Height(10000)));
		auto pView = CreateImportanceView(*pCache);

		// Act + Assert: mismatched height
		AssertCannotFindImportance(*pView, key, Height(3333));
	}

	namespace {
		template<typename TTraits>
		void AssertCanFindImportance(Importance accountImportance) {
			// Arrange:
			auto key = test::GenerateRandomData<Key_Size>();
			auto height = Height(1000);
			auto pCache = CreateAccountStateCache();
			AddAccount<TTraits>(*pCache, key, accountImportance, ConvertToImportanceHeight(height));
			auto pView = CreateImportanceView(*pCache);

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
		// Assert:
		AssertCanFindImportance<TTraits>(Importance(0));
	}

	KEY_TRAITS_BASED_TEST(CanRetrieveNonZeroImportanceFromAccount) {
		// Assert:
		AssertCanFindImportance<TTraits>(Importance(1234));
	}

	// endregion

	namespace {
		struct CanHarvestViaMemberTraits {
			static bool CanHarvest(const AccountStateCache& cache, const Key& publicKey, Height height, Amount minBalance) {
				auto pView = CreateImportanceView(cache);
				return pView->canHarvest(publicKey, height, minBalance);
			}
		};

		struct AddressCanHarvestViaMemberTraits : public AddressTraits, public CanHarvestViaMemberTraits {};
		struct PublicKeyCanHarvestViaMemberTraits : public PublicKeyTraits, public CanHarvestViaMemberTraits {};
	}

#define CAN_HARVEST_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_AddressViaMember) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressCanHarvestViaMemberTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_PublicKeyViaMember) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PublicKeyCanHarvestViaMemberTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region canHarvest

	CAN_HARVEST_TRAITS_BASED_TEST(CannotHarvestIfAccountIsUnknown) {
		// Arrange:
		auto key = test::GenerateRandomData<Key_Size>();
		auto height = Height(1000);
		auto pCache = CreateAccountStateCache();
		AddAccount<TTraits>(*pCache, key, Importance(1000), ConvertToImportanceHeight(height));

		// Act + Assert:
		EXPECT_FALSE(TTraits::CanHarvest(*pCache, test::GenerateRandomData<Key_Size>(), height, Amount(1234)));
	}

	namespace {
		template<typename TTraits>
		bool CanHarvest(int64_t minBalanceDelta, Importance importance, ImportanceHeight importanceHeight, Height testHeight) {
			// Arrange:
			auto key = test::GenerateRandomData<Key_Size>();
			auto pCache = CreateAccountStateCache();
			auto initialBalance = Amount(static_cast<Amount::ValueType>(1234 + minBalanceDelta));
			AddAccount<TTraits>(*pCache, key, importance, importanceHeight, initialBalance);

			// Act:
			return TTraits::CanHarvest(*pCache, key, testHeight, Amount(1234));
		}
	}

	CAN_HARVEST_TRAITS_BASED_TEST(CannotHarvestIfBalanceIsBelowMinBalance) {
		// Assert:
		auto height = Height(10000);
		EXPECT_FALSE(CanHarvest<TTraits>(-1, Importance(123), ConvertToImportanceHeight(height), height));
		EXPECT_FALSE(CanHarvest<TTraits>(-100, Importance(123), ConvertToImportanceHeight(height), height));
	}

	CAN_HARVEST_TRAITS_BASED_TEST(CannotHarvestIfImportanceIsZero) {
		// Assert:
		auto height = Height(10000);
		EXPECT_FALSE(CanHarvest<TTraits>(12345, Importance(0), ConvertToImportanceHeight(height), height));
	}

	CAN_HARVEST_TRAITS_BASED_TEST(CannotHarvestIfImportanceIsNotSetAtCorrectHeight) {
		// Assert:
		EXPECT_FALSE(CanHarvest<TTraits>(12345, Importance(0), ImportanceHeight(123), Height(1234)));
	}

	CAN_HARVEST_TRAITS_BASED_TEST(CanHarvestIfAllCriteriaAreMet) {
		// Assert:
		auto height = Height(10000);
		EXPECT_TRUE(CanHarvest<TTraits>(0, Importance(123), ConvertToImportanceHeight(height), height));
		EXPECT_TRUE(CanHarvest<TTraits>(1, Importance(123), ConvertToImportanceHeight(height), height));
		EXPECT_TRUE(CanHarvest<TTraits>(12345, Importance(123), ConvertToImportanceHeight(height), height));
	}

	// endregion
}}
