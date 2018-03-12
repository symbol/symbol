#include "catapult/cache_core/ReadOnlyAccountStateCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ReadOnlyAccountStateCacheTests

	// region cache properties

	TEST(TEST_CLASS, NetworkIdentifierIsExposed) {
		// Arrange:
		auto networkIdentifier = static_cast<model::NetworkIdentifier>(19);
		AccountStateCache originalCache({ networkIdentifier, 123, Amount() });

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, ReadOnlyAccountStateCache(*originalCache.createView()).networkIdentifier());
		EXPECT_EQ(networkIdentifier, ReadOnlyAccountStateCache(*originalCache.createDelta()).networkIdentifier());
	}

	TEST(TEST_CLASS, ImportanceGroupingIsExposed) {
		// Arrange:
		AccountStateCache originalCache({ static_cast<model::NetworkIdentifier>(19), 123, Amount() });

		// Act + Assert:
		EXPECT_EQ(123u, ReadOnlyAccountStateCache(*originalCache.createView()).importanceGrouping());
		EXPECT_EQ(123u, ReadOnlyAccountStateCache(*originalCache.createDelta()).importanceGrouping());
	}

	// endregion

	namespace {
		constexpr auto Default_Cache_Options = AccountStateCacheTypes::Options{
			model::NetworkIdentifier::Mijin_Test,
			543,
			Amount(std::numeric_limits<Amount::ValueType>::max())
		};

		struct AccountStateCacheByAddressTraits {
			static auto CreateElement(uint8_t tag) {
				return Address({ { static_cast<uint8_t>(tag * tag) } });
			}
		};

		struct AccountStateCacheByKeyTraits {
			static auto CreateElement(uint8_t tag) {
				return Key({ { static_cast<uint8_t>(tag * tag) } });
			}
		};
	}

#define ACCOUNT_KEY_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ByAddress) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountStateCacheByAddressTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ByKey) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountStateCacheByKeyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ACCOUNT_KEY_BASED_TEST(ReadOnlyViewOnlyContainsCommittedElements) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		{
			auto cacheDelta = cache.createDelta();
			cacheDelta->addAccount(TTraits::CreateElement(1), Height(123)); // committed
			cache.commit();
			cacheDelta->addAccount(TTraits::CreateElement(2), Height(123)); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		ReadOnlyAccountStateCache readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_TRUE(readOnlyCache.contains(TTraits::CreateElement(1)));
		EXPECT_FALSE(readOnlyCache.contains(TTraits::CreateElement(2)));
		EXPECT_FALSE(readOnlyCache.contains(TTraits::CreateElement(3)));
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyDeltaContainsBothCommittedAndUncommittedElements) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto cacheDelta = cache.createDelta();
		cacheDelta->addAccount(TTraits::CreateElement(1), Height(123)); // committed
		cache.commit();
		cacheDelta->addAccount(TTraits::CreateElement(2), Height(123)); // uncommitted

		// Act:
		ReadOnlyAccountStateCache readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_TRUE(readOnlyCache.contains(TTraits::CreateElement(1)));
		EXPECT_TRUE(readOnlyCache.contains(TTraits::CreateElement(2)));
		EXPECT_FALSE(readOnlyCache.contains(TTraits::CreateElement(3)));
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyViewOnlyCanAccessCommittedElementsViaGet) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		const state::AccountState* pAccount1;
		{
			auto cacheDelta = cache.createDelta();
			pAccount1 = &cacheDelta->addAccount(TTraits::CreateElement(1), Height(123)); // committed;
			cache.commit();
			cacheDelta->addAccount(TTraits::CreateElement(2), Height(123)); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		ReadOnlyAccountStateCache readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_EQ(pAccount1, &readOnlyCache.get(TTraits::CreateElement(1)));
		EXPECT_THROW(readOnlyCache.get(TTraits::CreateElement(2)), catapult_invalid_argument);
		EXPECT_THROW(readOnlyCache.get(TTraits::CreateElement(3)), catapult_invalid_argument);
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyDeltaCanAccessBothCommittedAndUncommittedElementsViaGet) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto cacheDelta = cache.createDelta();
		const auto& account1 = cacheDelta->addAccount(TTraits::CreateElement(1), Height(123)); // committed
		cache.commit();
		const auto& account2 = cacheDelta->addAccount(TTraits::CreateElement(2), Height(123)); // uncommitted

		// Act:
		ReadOnlyAccountStateCache readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_EQ(&account1, &readOnlyCache.get(TTraits::CreateElement(1)));
		EXPECT_EQ(&account2, &readOnlyCache.get(TTraits::CreateElement(2)));
		EXPECT_THROW(readOnlyCache.get(TTraits::CreateElement(3)), catapult_invalid_argument);
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyViewOnlyCanAccessCommittedElementsViaTryGet) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		const state::AccountState* pAccount1;
		{
			auto cacheDelta = cache.createDelta();
			pAccount1 = &cacheDelta->addAccount(TTraits::CreateElement(1), Height(123)); // committed;
			cache.commit();
			cacheDelta->addAccount(TTraits::CreateElement(2), Height(123)); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		ReadOnlyAccountStateCache readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_EQ(pAccount1, readOnlyCache.tryGet(TTraits::CreateElement(1)));
		EXPECT_FALSE(!!readOnlyCache.tryGet(TTraits::CreateElement(2)));
		EXPECT_FALSE(!!readOnlyCache.tryGet(TTraits::CreateElement(3)));
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyDeltaCanAccessBothCommittedAndUncommittedElementsViaTryGet) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto cacheDelta = cache.createDelta();
		const auto& account1 = cacheDelta->addAccount(TTraits::CreateElement(1), Height(123)); // committed
		cache.commit();
		const auto& account2 = cacheDelta->addAccount(TTraits::CreateElement(2), Height(123)); // uncommitted

		// Act:
		ReadOnlyAccountStateCache readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_EQ(&account1, readOnlyCache.tryGet(TTraits::CreateElement(1)));
		EXPECT_EQ(&account2, readOnlyCache.tryGet(TTraits::CreateElement(2)));
		EXPECT_FALSE(!!readOnlyCache.tryGet(TTraits::CreateElement(3)));
	}
}}
