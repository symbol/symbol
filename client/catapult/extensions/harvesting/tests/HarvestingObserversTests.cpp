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

#include "harvesting/src/HarvestingObservers.h"
#include "tests/test/plugins/ObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

	// region traits

	namespace {
		struct AddressTraits {
			static Address CreateKey() {
				return test::GenerateRandomByteArray<Address>();
			}

			static auto CreateNotification(const Address& key) {
				return model::AccountAddressNotification(test::UnresolveXor(key));
			}

			static auto CreateObserver(RefCountedAccountIdentifiers<Address>& accountIdentifiers) {
				return CreateHarvestingAccountAddressObserver(accountIdentifiers);
			}
		};

		struct PublicKeyTraits {
			static Key CreateKey() {
				return test::GenerateRandomByteArray<Key>();
			}

			static auto CreateNotification(const Key& key) {
				return model::AccountPublicKeyNotification(key);
			}

			static auto CreateObserver(RefCountedAccountIdentifiers<Key>& accountIdentifiers) {
				return CreateHarvestingAccountPublicKeyObserver(accountIdentifiers);
			}
		};
	}

#define ACCOUNT_KEY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(HarvestingAccountAddressObserverTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	TEST(HarvestingAccountPublicKeyObserverTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PublicKeyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region tests

	ACCOUNT_KEY_TEST(AccountObserverAddsAccountOnCommit) {
		// Arrange:
		auto accountIdentifiers = RefCountedAccountIdentifiers<decltype(TTraits::CreateKey())>();
		auto pObserver = TTraits::CreateObserver(accountIdentifiers);

		auto key = TTraits::CreateKey();
		auto notification = TTraits::CreateNotification(key);

		// Act:
		test::ObserverTestContext context(observers::NotifyMode::Commit);
		test::ObserveNotification(*pObserver, notification, context);

		// Assert: the account was captured
		EXPECT_EQ(1u, accountIdentifiers.size());

		auto iter = accountIdentifiers.find(key);
		ASSERT_NE(accountIdentifiers.end(), iter);
		EXPECT_EQ(1u, iter->second);
	}

	ACCOUNT_KEY_TEST(AccountObserverAddsAccountOnEachCommit) {
		// Arrange:
		auto accountIdentifiers = RefCountedAccountIdentifiers<decltype(TTraits::CreateKey())>();
		auto pObserver = TTraits::CreateObserver(accountIdentifiers);

		auto key = TTraits::CreateKey();
		auto notification = TTraits::CreateNotification(key);

		// Act:
		test::ObserverTestContext context(observers::NotifyMode::Commit);
		test::ObserveNotification(*pObserver, notification, context);
		test::ObserveNotification(*pObserver, notification, context);
		test::ObserveNotification(*pObserver, notification, context);

		// Assert: the account was captured
		EXPECT_EQ(1u, accountIdentifiers.size());

		auto iter = accountIdentifiers.find(key);
		ASSERT_NE(accountIdentifiers.end(), iter);
		EXPECT_EQ(3u, iter->second);
	}

	ACCOUNT_KEY_TEST(AccountObserverRemovesAccountWhenLastReferenceIsRemovedOnRollback) {
		// Arrange:
		auto accountIdentifiers = RefCountedAccountIdentifiers<decltype(TTraits::CreateKey())>();
		auto pObserver = TTraits::CreateObserver(accountIdentifiers);

		auto key = TTraits::CreateKey();
		auto notification = TTraits::CreateNotification(key);

		test::ObserverTestContext commitContext(observers::NotifyMode::Commit);
		test::ObserveNotification(*pObserver, notification, commitContext);

		// Sanity:
		EXPECT_EQ(1u, accountIdentifiers.size());

		// Act:
		test::ObserverTestContext rollbackContext(observers::NotifyMode::Rollback);
		test::ObserveNotification(*pObserver, notification, rollbackContext);

		// Assert: the account was removed
		EXPECT_EQ(0u, accountIdentifiers.size());
	}

	ACCOUNT_KEY_TEST(AccountObserverDoesNotRemoveAccountWhenIntermediateReferenceIsRemovedOnRollback) {
		// Arrange:
		auto accountIdentifiers = RefCountedAccountIdentifiers<decltype(TTraits::CreateKey())>();
		auto pObserver = TTraits::CreateObserver(accountIdentifiers);

		auto key = TTraits::CreateKey();
		auto notification = TTraits::CreateNotification(key);

		test::ObserverTestContext commitContext(observers::NotifyMode::Commit);
		test::ObserveNotification(*pObserver, notification, commitContext);
		test::ObserveNotification(*pObserver, notification, commitContext);

		// Sanity:
		EXPECT_EQ(1u, accountIdentifiers.size());

		// Act:
		test::ObserverTestContext rollbackContext(observers::NotifyMode::Rollback);
		test::ObserveNotification(*pObserver, notification, rollbackContext);

		// Assert: the account was not removed
		EXPECT_EQ(1u, accountIdentifiers.size());

		auto iter = accountIdentifiers.find(key);
		ASSERT_NE(accountIdentifiers.end(), iter);
		EXPECT_EQ(1u, iter->second);
	}

	// endregion
}}
