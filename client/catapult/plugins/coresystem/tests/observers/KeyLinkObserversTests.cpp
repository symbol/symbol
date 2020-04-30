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

#include "src/observers/KeyLinkObservers.h"
#include "plugins/coresystem/tests/test/KeyLinkTestUtils.h"
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

	DEFINE_COMMON_OBSERVER_TESTS(VotingKeyLink,)
	DEFINE_COMMON_OBSERVER_TESTS(VrfKeyLink,)

#define TEST_CLASS KeyLinkObserversTests // used to generate unique function names in macros
#define VOTING_TEST_CLASS VotingKeyLinkObserverTests
#define VRF_TEST_CLASS VrfKeyLinkObserverTests

	// region traits

	namespace {
		struct VotingTraits : test::BasicVotingKeyLinkTestTraits {
			static constexpr auto CreateObserver = CreateVotingKeyLinkObserver;
		};

		struct VrfTraits : test::BasicVrfKeyLinkTestTraits {
			static constexpr auto CreateObserver = CreateVrfKeyLinkObserver;
		};
	}

#define KEY_LINK_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(VOTING_TEST_CLASS, TEST_NAME##_Voting) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VotingTraits>(); } \
	TEST(VRF_TEST_CLASS, TEST_NAME##_Vrf) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VrfTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region test utils

	namespace {
		template<typename TTraits>
		auto RunKeyLinkObserverTest(
				NotifyMode notifyMode,
				model::LinkAction linkAction,
				const typename TTraits::KeyType& cacheLinkedPublicKey,
				const typename TTraits::KeyType& notificationLinkedPublicKey) {
			// Arrange:
			test::AccountObserverTestContext context(notifyMode);
			auto mainAccountPublicKey = test::AddAccountWithLink<TTraits>(context.cache(), cacheLinkedPublicKey);

			auto pObserver = TTraits::CreateObserver();
			auto notification = typename TTraits::NotificationType(mainAccountPublicKey, notificationLinkedPublicKey, linkAction);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			auto& accountStateCache = context.cache().template sub<cache::AccountStateCache>();
			return TTraits::GetKeyAccessor(accountStateCache.find(mainAccountPublicKey).get()).get();
		}
	}

	// endregion

	// region link

	KEY_LINK_TEST(LinkCommitSetsLink) {
		// Act:
		auto notificationLinkedPublicKey = test::GenerateRandomByteArray<typename TTraits::KeyType>();
		auto finalLinkedPublicKey = RunKeyLinkObserverTest<TTraits>(
				NotifyMode::Commit,
				model::LinkAction::Link,
				typename TTraits::KeyType(),
				notificationLinkedPublicKey);

		// Assert:
		EXPECT_EQ(notificationLinkedPublicKey, finalLinkedPublicKey);
	}

	KEY_LINK_TEST(LinkRollbackUnsetsLink) {
		// Act:
		auto notificationLinkedPublicKey = test::GenerateRandomByteArray<typename TTraits::KeyType>();
		auto finalLinkedPublicKey = RunKeyLinkObserverTest<TTraits>(
				NotifyMode::Rollback,
				model::LinkAction::Link,
				notificationLinkedPublicKey,
				notificationLinkedPublicKey);

		// Assert:
		EXPECT_EQ(typename TTraits::KeyType(), finalLinkedPublicKey);
	}

	// endregion

	// region unlink

	KEY_LINK_TEST(UnlinkCommitUnsetsLink) {
		// Act:
		auto notificationLinkedPublicKey = test::GenerateRandomByteArray<typename TTraits::KeyType>();
		auto finalLinkedPublicKey = RunKeyLinkObserverTest<TTraits>(
				NotifyMode::Commit,
				model::LinkAction::Unlink,
				notificationLinkedPublicKey,
				notificationLinkedPublicKey);

		// Assert:
		EXPECT_EQ(typename TTraits::KeyType(), finalLinkedPublicKey);
	}

	KEY_LINK_TEST(UnlinkRollbackSetsLink) {
		// Act:
		auto notificationLinkedPublicKey = test::GenerateRandomByteArray<typename TTraits::KeyType>();
		auto finalLinkedPublicKey = RunKeyLinkObserverTest<TTraits>(
				NotifyMode::Rollback,
				model::LinkAction::Unlink,
				typename TTraits::KeyType(),
				notificationLinkedPublicKey);

		// Assert:
		EXPECT_EQ(notificationLinkedPublicKey, finalLinkedPublicKey);

	}

	// endregion
}}
