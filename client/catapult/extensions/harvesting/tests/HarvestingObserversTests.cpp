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

#include "harvesting/src/HarvestingObservers.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "tests/test/plugins/ObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define ADDRESS_TEST_CLASS HarvestingAccountAddressObserverTests
#define PUBLIC_KEY_TEST_CLASS HarvestingAccountPublicKeyObserverTests

	// region traits

	namespace {
		struct AddressTraits {
			static auto CreateAccountIdentifier() {
				return test::GenerateRandomByteArray<Address>();
			}

			static auto CreateNotification(const Address& address) {
				return model::AccountAddressNotification(test::UnresolveXor(address));
			}

			static auto CreateObserver(HarvestingAffectedAccounts& accounts) {
				return CreateHarvestingAccountAddressObserver(accounts);
			}

			static void AssertSize(const HarvestingAffectedAccounts& accounts, size_t size) {
				EXPECT_EQ(size, accounts.Addresses.size());
				EXPECT_EQ(0u, accounts.PublicKeys.size());
			}

			static void AssertContains(const HarvestingAffectedAccounts& accounts, const Address& address, size_t count) {
				auto iter = accounts.Addresses.find(address);
				ASSERT_NE(accounts.Addresses.cend(), iter);
				EXPECT_EQ(count, iter->second);
			}
		};

		struct PublicKeyTraits {
			static auto CreateAccountIdentifier() {
				return test::GenerateRandomByteArray<Key>();
			}

			static auto CreateNotification(const Key& key) {
				return model::AccountPublicKeyNotification(key);
			}

			static auto CreateObserver(HarvestingAffectedAccounts& accounts) {
				return CreateHarvestingAccountPublicKeyObserver(accounts);
			}

			static void AssertSize(const HarvestingAffectedAccounts& accounts, size_t size) {
				EXPECT_EQ(size, accounts.Addresses.size());
				EXPECT_EQ(size, accounts.PublicKeys.size());
			}

			static void AssertContains(const HarvestingAffectedAccounts& accounts, const Key& key, size_t count) {
				auto address = model::PublicKeyToAddress(key, model::NetworkIdentifier::Zero);
				AddressTraits::AssertContains(accounts, address, count);

				auto iter = accounts.PublicKeys.find(key);
				ASSERT_NE(accounts.PublicKeys.cend(), iter);
				EXPECT_EQ(count, iter->second);
			}
		};
	}

#define ACCOUNT_IDENTIFIER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(ADDRESS_TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	TEST(PUBLIC_KEY_TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PublicKeyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region tracking tests

	ACCOUNT_IDENTIFIER_TEST(AccountObserverAddsAccountOnCommit) {
		// Arrange:
		HarvestingAffectedAccounts accounts;
		auto pObserver = TTraits::CreateObserver(accounts);

		auto accountIdentifier = TTraits::CreateAccountIdentifier();
		auto notification = TTraits::CreateNotification(accountIdentifier);

		// Act:
		test::ObserverTestContext context(observers::NotifyMode::Commit);
		test::ObserveNotification(*pObserver, notification, context);

		// Assert: the account was captured
		TTraits::AssertSize(accounts, 1);
		TTraits::AssertContains(accounts, accountIdentifier, 1);
	}

	ACCOUNT_IDENTIFIER_TEST(AccountObserverAddsAccountOnEachCommit) {
		// Arrange:
		HarvestingAffectedAccounts accounts;
		auto pObserver = TTraits::CreateObserver(accounts);

		auto accountIdentifier = TTraits::CreateAccountIdentifier();
		auto notification = TTraits::CreateNotification(accountIdentifier);

		// Act:
		test::ObserverTestContext context(observers::NotifyMode::Commit);
		test::ObserveNotification(*pObserver, notification, context);
		test::ObserveNotification(*pObserver, notification, context);
		test::ObserveNotification(*pObserver, notification, context);

		// Assert: the account was captured
		TTraits::AssertSize(accounts, 1);
		TTraits::AssertContains(accounts, accountIdentifier, 3);
	}

	ACCOUNT_IDENTIFIER_TEST(AccountObserverRemovesAccountWhenLastReferenceIsRemovedOnRollback) {
		// Arrange:
		HarvestingAffectedAccounts accounts;
		auto pObserver = TTraits::CreateObserver(accounts);

		auto accountIdentifier = TTraits::CreateAccountIdentifier();
		auto notification = TTraits::CreateNotification(accountIdentifier);

		test::ObserverTestContext commitContext(observers::NotifyMode::Commit);
		test::ObserveNotification(*pObserver, notification, commitContext);

		// Sanity:
		TTraits::AssertSize(accounts, 1);

		// Act:
		test::ObserverTestContext rollbackContext(observers::NotifyMode::Rollback);
		test::ObserveNotification(*pObserver, notification, rollbackContext);

		// Assert: the account was removed
		TTraits::AssertSize(accounts, 0);
	}

	ACCOUNT_IDENTIFIER_TEST(AccountObserverDoesNotRemoveAccountWhenIntermediateReferenceIsRemovedOnRollback) {
		// Arrange:
		HarvestingAffectedAccounts accounts;
		auto pObserver = TTraits::CreateObserver(accounts);

		auto accountIdentifier = TTraits::CreateAccountIdentifier();
		auto notification = TTraits::CreateNotification(accountIdentifier);

		test::ObserverTestContext commitContext(observers::NotifyMode::Commit);
		test::ObserveNotification(*pObserver, notification, commitContext);
		test::ObserveNotification(*pObserver, notification, commitContext);

		// Sanity:
		TTraits::AssertSize(accounts, 1);

		// Act:
		test::ObserverTestContext rollbackContext(observers::NotifyMode::Rollback);
		test::ObserveNotification(*pObserver, notification, rollbackContext);

		// Assert: the account was not removed
		TTraits::AssertSize(accounts, 1);
		TTraits::AssertContains(accounts, accountIdentifier, 1);
	}

	// endregion

	// region queue removal tests

	namespace {
		template<typename TTraits, typename TAction>
		void RunQueueRemovalTest(TAction action) {
			// Arrange:
			HarvestingAffectedAccounts accounts;
			auto pObserver = TTraits::CreateObserver(accounts);

			auto accountIdentifier = TTraits::CreateAccountIdentifier();
			auto notification = TTraits::CreateNotification(accountIdentifier);

			test::ObserverTestContext commitContext(observers::NotifyMode::Commit);
			test::ObserveNotification(*pObserver, notification, commitContext);
			test::ObserveNotification(*pObserver, notification, commitContext);

			// Act:
			test::ObserverTestContext rollbackContext(observers::NotifyMode::Rollback);
			test::ObserveNotification(*pObserver, notification, rollbackContext);

			// Sanity:
			TTraits::AssertSize(accounts, 1);

			// Assert: add account to cache and then call commit removals to check if anything was queued for removal
			auto& accountStateCacheDelta = rollbackContext.cache().sub<cache::AccountStateCache>();
			accountStateCacheDelta.addAccount(accountIdentifier, rollbackContext.observerContext().Height);
			accountStateCacheDelta.commitRemovals();

			action(accountStateCacheDelta, accountIdentifier);
		}
	}

	TEST(ADDRESS_TEST_CLASS, QueueRemoveIsNotCalledForAddressDuringRollback) {
		// Act:
		RunQueueRemovalTest<AddressTraits>([](const auto& accountStateCache, const auto& address) {
			// Assert: nothing should have been queued for removal
			EXPECT_EQ(1u, accountStateCache.size());
			EXPECT_TRUE(accountStateCache.contains(address));
		});
	}

	TEST(PUBLIC_KEY_TEST_CLASS, QueueRemoveIsCalledForAddressDuringRollback) {
		// Act:
		RunQueueRemovalTest<PublicKeyTraits>([](const auto& accountStateCache, const auto&) {
			// Assert: address should have been queued for removal
			EXPECT_EQ(0u, accountStateCache.size());
		});
	}

	// endregion
}}
