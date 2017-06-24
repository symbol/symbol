#include "src/cache/ReadOnlyAccountStateCache.h"
#include "src/cache/AccountStateCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ReadOnlyAccountStateCacheTests

	// region cache properties

	TEST(TEST_CLASS, NetworkIdentifierIsExposed) {
		// Arrange:
		auto networkIdentifier = static_cast<model::NetworkIdentifier>(19);
		AccountStateCache originalCache(networkIdentifier, 123);

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, ReadOnlyAccountStateCache(*originalCache.createView()).networkIdentifier());
		EXPECT_EQ(networkIdentifier, ReadOnlyAccountStateCache(*originalCache.createDelta()).networkIdentifier());
	}

	TEST(TEST_CLASS, ImportanceGroupingIsExposed) {
		// Arrange:
		AccountStateCache originalCache(static_cast<model::NetworkIdentifier>(19), 123);

		// Act + Assert:
		EXPECT_EQ(123u, ReadOnlyAccountStateCache(*originalCache.createView()).importanceGrouping());
		EXPECT_EQ(123u, ReadOnlyAccountStateCache(*originalCache.createDelta()).importanceGrouping());
	}

	// endregion

	namespace {
		struct AccountStateCacheByAddressTraits {
			static auto CreateEntity(uint8_t tag) {
				return Address({ { static_cast<uint8_t>(tag * tag) } });
			}
		};

		struct AccountStateCacheByKeyTraits {
			static auto CreateEntity(uint8_t tag) {
				return Key({ { static_cast<uint8_t>(tag * tag) } });
			}
		};
	}

#define ACCOUNT_KEY_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ByAddress) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountStateCacheByAddressTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ByKey) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountStateCacheByKeyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ACCOUNT_KEY_BASED_TEST(ReadOnlyViewOnlyContainsCommittedEntities) {
		// Arrange:
		AccountStateCache cache(model::NetworkIdentifier::Mijin_Test, 543);
		{
			auto cacheDelta = cache.createDelta();
			cacheDelta->addAccount(TTraits::CreateEntity(1), Height(123)); // committed
			cache.commit();
			cacheDelta->addAccount(TTraits::CreateEntity(2), Height(123)); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		ReadOnlyAccountStateCache readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_TRUE(readOnlyCache.contains(TTraits::CreateEntity(1)));
		EXPECT_FALSE(readOnlyCache.contains(TTraits::CreateEntity(2)));
		EXPECT_FALSE(readOnlyCache.contains(TTraits::CreateEntity(3)));
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyDeltaContainsBothCommittedAndUncommittedEntities) {
		// Arrange:
		AccountStateCache cache(model::NetworkIdentifier::Mijin_Test, 543);
		auto cacheDelta = cache.createDelta();
		cacheDelta->addAccount(TTraits::CreateEntity(1), Height(123)); // committed
		cache.commit();
		cacheDelta->addAccount(TTraits::CreateEntity(2), Height(123)); // uncommitted

		// Act:
		ReadOnlyAccountStateCache readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_TRUE(readOnlyCache.contains(TTraits::CreateEntity(1)));
		EXPECT_TRUE(readOnlyCache.contains(TTraits::CreateEntity(2)));
		EXPECT_FALSE(readOnlyCache.contains(TTraits::CreateEntity(3)));
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyViewOnlyCanAccessCommittedEntitiesViaFind) {
		// Arrange:
		AccountStateCache cache(model::NetworkIdentifier::Mijin_Test, 543);
		std::shared_ptr<state::AccountState> pAccount1;
		{
			auto cacheDelta = cache.createDelta();
			pAccount1 = cacheDelta->addAccount(TTraits::CreateEntity(1), Height(123)); // committed
			cache.commit();
			cacheDelta->addAccount(TTraits::CreateEntity(2), Height(123)); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		ReadOnlyAccountStateCache readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_EQ(pAccount1, readOnlyCache.findAccount(TTraits::CreateEntity(1)));
		EXPECT_FALSE(!!readOnlyCache.findAccount(TTraits::CreateEntity(2)));
		EXPECT_FALSE(!!readOnlyCache.findAccount(TTraits::CreateEntity(3)));
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyDeltaCanAccessBothCommittedAndUncommittedEntitiesViaFind) {
		// Arrange:
		AccountStateCache cache(model::NetworkIdentifier::Mijin_Test, 543);
		auto cacheDelta = cache.createDelta();
		auto pAccount1 = cacheDelta->addAccount(TTraits::CreateEntity(1), Height(123)); // committed
		cache.commit();
		auto pAccount2 = cacheDelta->addAccount(TTraits::CreateEntity(2), Height(123)); // uncommitted

		// Act:
		ReadOnlyAccountStateCache readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_EQ(pAccount1, readOnlyCache.findAccount(TTraits::CreateEntity(1)));
		EXPECT_EQ(pAccount2, readOnlyCache.findAccount(TTraits::CreateEntity(2)));
		EXPECT_FALSE(!!readOnlyCache.findAccount(TTraits::CreateEntity(3)));
	}
}}
