/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "src/observers/Observers.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

	DEFINE_COMMON_OBSERVER_TESTS(AccountAddress,)
	DEFINE_COMMON_OBSERVER_TESTS(AccountPublicKey,)

	// region traits

	namespace {
		struct AddressTraits {
			static Address CreateKey() {
				return test::GenerateRandomByteArray<Address>();
			}

			static auto CreateNotification(const Address& key) {
				return model::AccountAddressNotification(test::UnresolveXor(key));
			}

			static auto CreateObserver() {
				return CreateAccountAddressObserver();
			}
		};

		struct PublicKeyTraits {
			static Key CreateKey() {
				return test::GenerateRandomByteArray<Key>();
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
	TEST(AccountAddressObserverTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	TEST(AccountPublicKeyObserverTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PublicKeyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

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

			auto cache = test::CreateEmptyCatapultCache();
			auto cacheDelta = cache.createDelta();

			// - commit
			auto commitContext = test::CreateObserverContext(cacheDelta, commitHeight, NotifyMode::Commit);
			test::ObserveNotification(*pObserver, TTraits::CreateNotification(key), commitContext);

			// Sanity: the account was added
			auto& accountStateCache = cacheDelta.sub<cache::AccountStateCache>();
			EXPECT_EQ(1u, accountStateCache.size());

			// Act: rollback
			auto rollbackContext = test::CreateObserverContext(cacheDelta, rollbackHeight, NotifyMode::Rollback);
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
		AssertAccountObserverRollback<TTraits>(1, Height(1234), Height(1234));
	}

	ACCOUNT_KEY_TEST(RollbackDoesNotQueueRemovalAtDifferentHeight) {
		AssertAccountObserverRollback<TTraits>(0, Height(1234), Height(1235));
	}

	// endregion
}}
