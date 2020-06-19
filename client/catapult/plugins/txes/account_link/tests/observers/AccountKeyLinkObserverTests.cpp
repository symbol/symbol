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

#include "src/observers/Observers.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS AccountKeyLinkObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::CoreSystemCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(AccountKeyLink,)

	namespace {
		struct CommitTraits {
			static constexpr auto Notify_Mode = NotifyMode::Commit;
			static constexpr auto Create_Link = model::LinkAction::Link;
			static constexpr auto Remove_Link = model::LinkAction::Unlink;
		};

		struct RollbackTraits {
			static constexpr auto Notify_Mode = NotifyMode::Rollback;
			// during rollback actions need to be reversed to create or remove link
			static constexpr auto Create_Link = model::LinkAction::Unlink;
			static constexpr auto Remove_Link = model::LinkAction::Link;
		};
	}

#define ACCOUNT_LINK_OBSERVER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Commit) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CommitTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Rollback) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RollbackTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		template<typename TAction>
		void RunTwoAccountTest(cache::AccountStateCacheDelta& accountStateCacheDelta, TAction action) {
			// Arrange:
			auto mainAccountPublicKey = test::GenerateRandomByteArray<Key>();
			auto linkedPublicKey = test::GenerateRandomByteArray<Key>();

			accountStateCacheDelta.addAccount(mainAccountPublicKey, Height(444));
			accountStateCacheDelta.addAccount(linkedPublicKey, Height(444));

			auto mainAccountStateIter = accountStateCacheDelta.find(mainAccountPublicKey);
			auto& mainAccountState = mainAccountStateIter.get();

			auto remoteAccountStateIter = accountStateCacheDelta.find(linkedPublicKey);
			auto& remoteAccountState = remoteAccountStateIter.get();

			// Act + Assert:
			action(mainAccountState, remoteAccountState);
		}
	}

	ACCOUNT_LINK_OBSERVER_TEST(ObserverCreatesLink) {
		// Arrange:
		auto context = ObserverTestContext(TTraits::Notify_Mode, Height(888));
		RunTwoAccountTest(context.cache().sub<cache::AccountStateCache>(), [&context](
				const auto& mainAccountState,
				const auto& remoteAccountState) {
			auto mainAccountPublicKey = mainAccountState.PublicKey;
			auto linkedPublicKey = remoteAccountState.PublicKey;

			auto notification = model::RemoteAccountKeyLinkNotification(mainAccountPublicKey, linkedPublicKey, TTraits::Create_Link);
			auto pObserver = CreateAccountKeyLinkObserver();

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: link was created
			EXPECT_EQ(state::AccountType::Main, mainAccountState.AccountType);
			EXPECT_EQ(linkedPublicKey, state::GetLinkedPublicKey(mainAccountState));

			EXPECT_EQ(state::AccountType::Remote, remoteAccountState.AccountType);
			EXPECT_EQ(mainAccountPublicKey, state::GetLinkedPublicKey(remoteAccountState));
		});
	}

	ACCOUNT_LINK_OBSERVER_TEST(ObserverRemovesLink) {
		// Arrange:
		auto context = ObserverTestContext(TTraits::Notify_Mode, Height(888));
		RunTwoAccountTest(context.cache().sub<cache::AccountStateCache>(), [&context](auto& mainAccountState, auto& remoteAccountState) {
			auto mainAccountPublicKey = mainAccountState.PublicKey;
			auto linkedPublicKey = remoteAccountState.PublicKey;

			mainAccountState.SupplementalPublicKeys.linked().set(linkedPublicKey);
			mainAccountState.AccountType = state::AccountType::Main;

			remoteAccountState.SupplementalPublicKeys.linked().set(mainAccountPublicKey);
			remoteAccountState.AccountType = state::AccountType::Remote;

			auto notification = model::RemoteAccountKeyLinkNotification(mainAccountPublicKey, linkedPublicKey, TTraits::Remove_Link);
			auto pObserver = CreateAccountKeyLinkObserver();

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: link was removed
			EXPECT_EQ(state::AccountType::Unlinked, mainAccountState.AccountType);
			EXPECT_EQ(Key(), state::GetLinkedPublicKey(mainAccountState));

			EXPECT_EQ(state::AccountType::Remote_Unlinked, remoteAccountState.AccountType);
			EXPECT_EQ(Key(), state::GetLinkedPublicKey(remoteAccountState));
		});
	}
}}
