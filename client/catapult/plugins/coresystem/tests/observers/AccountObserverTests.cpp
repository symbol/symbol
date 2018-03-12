#include "src/observers/Observers.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

	DEFINE_COMMON_OBSERVER_TESTS(AccountAddress,)
	DEFINE_COMMON_OBSERVER_TESTS(AccountPublicKey,)

	namespace {
		struct AddressTraits {
			static Address CreateKey() {
				return test::GenerateRandomData<Address_Decoded_Size>();
			}

			static auto CreateNotification(const Address& key) {
				return model::AccountAddressNotification(key);
			}

			static auto CreateObserver() {
				return CreateAccountAddressObserver();
			}
		};

		struct PublicKeyTraits {
			static Key CreateKey() {
				return test::GenerateRandomData<Key_Size>();
			}

			static auto CreateNotification(const Key& key) {
				return model::AccountPublicKeyNotification(key);
			}

			static auto CreateObserver() {
				return CreateAccountPublicKeyObserver();
			}
		};
	}

#define ACCOUNT_KEY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(AccountAddressObserverTests, TEST_NAME##_Id) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	TEST(AccountPublicKeyObserverTests, TEST_NAME##_Name) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PublicKeyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region commit

	ACCOUNT_KEY_TEST(AccountObserverAddsAccountOnCommit) {
		// Arrange:
		test::AccountObserverTestContext context(NotifyMode::Commit);
		auto pObserver = TTraits::CreateObserver();

		auto key = TTraits::CreateKey();
		auto notification = TTraits::CreateNotification(key);

		// Act:
		test::ObserveNotification(*pObserver, notification, context);

		// Assert: the account was added to the cache
		EXPECT_EQ(1u, context.cache().sub<cache::AccountStateCache>().size());
		EXPECT_TRUE(!!context.find(key));
	}

	ACCOUNT_KEY_TEST(SubsequentNotificationsDoNotInvalidatePointers) {
		// Arrange:
		test::AccountObserverTestContext context(NotifyMode::Commit);
		auto pObserver = TTraits::CreateObserver();

		auto key = TTraits::CreateKey();

		// Act:
		test::ObserveNotification(*pObserver, TTraits::CreateNotification(key), context);
		auto pStateBefore = context.find(key);

		test::ObserveNotification(*pObserver, TTraits::CreateNotification(key), context);
		auto pStateAfter = context.find(key);

		// Assert:
		// - accounts added to the cache by first notify were not invalidated/overwritten
		//   by the observations of same accounts in the second notify
		EXPECT_EQ(1u, context.cache().sub<cache::AccountStateCache>().size());
		EXPECT_EQ(pStateBefore, pStateAfter);
	}

	// endregion

	// region rollback

	namespace {
		template<typename TTraits>
		void AssertAccountObserverRollback(size_t removedSize, Height commitHeight, Height rollbackHeight) {
			// Arrange:
			auto pObserver = TTraits::CreateObserver();

			auto key = TTraits::CreateKey();

			state::CatapultState state;
			auto cache = test::CreateEmptyCatapultCache();
			auto cacheDelta = cache.createDelta();

			// - commit
			auto commitContext = test::CreateObserverContext(cacheDelta, state, commitHeight, NotifyMode::Commit);
			test::ObserveNotification(*pObserver, TTraits::CreateNotification(key), commitContext);

			// Sanity: the account was added
			auto& accountStateCache = cacheDelta.sub<cache::AccountStateCache>();
			EXPECT_EQ(1u, accountStateCache.size());

			// Act: rollback
			auto rollbackContext = test::CreateObserverContext(cacheDelta, state, rollbackHeight, NotifyMode::Rollback);
			test::ObserveNotification(*pObserver, TTraits::CreateNotification(key), rollbackContext);

			// Sanity: nothing changed so far
			EXPECT_EQ(1u, accountStateCache.size());

			// Act: commit the removals
			accountStateCache.commitRemovals();

			// Assert:
			EXPECT_EQ(1u - removedSize, accountStateCache.size());
		}
	}

	ACCOUNT_KEY_TEST(RollbackQueuesRemovalOfAccountAtSameHeight) {
		// Assert:
		AssertAccountObserverRollback<TTraits>(1, Height(1234), Height(1234));
	}

	ACCOUNT_KEY_TEST(RollbackDoesNotQueueRemovalAtDifferentHeight) {
		// Assert:
		AssertAccountObserverRollback<TTraits>(0, Height(1234), Height(1235));
	}

	// endregion
}}
