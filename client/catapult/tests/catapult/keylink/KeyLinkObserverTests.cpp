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

#include "catapult/keylink/KeyLinkObserver.h"
#include "tests/catapult/keylink/test/KeyLinkTestUtils.h"
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace keylink {

#define TEST_CLASS KeyLinkObserverTests

	// region test utils

	namespace {
		using Notification = model::BasicKeyLinkNotification<Key, static_cast<model::NotificationType>(0)>;

		struct Accessor {
			static auto& Get(state::AccountState& accountState) {
				return accountState.SupplementalPublicKeys.linked();
			}
		};

		auto CreateKeyLinkObserver(const std::string& name) {
			return keylink::CreateKeyLinkObserver<Notification, Accessor>(name);
		}

		auto RunKeyLinkObserverTest(
				observers::NotifyMode notifyMode,
				model::LinkAction linkAction,
				const Key& cacheLinkedPublicKey,
				const Key& notificationLinkedPublicKey) {
			// Arrange:
			test::AccountObserverTestContext context(notifyMode);
			auto mainAccountPublicKey = test::AddAccountWithLink(context.cache(), cacheLinkedPublicKey);

			auto pObserver = CreateKeyLinkObserver("");
			auto notification = Notification(mainAccountPublicKey, notificationLinkedPublicKey, linkAction);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			auto& accountStateCache = context.cache().sub<cache::AccountStateCache>();
			return Accessor::Get(accountStateCache.find(mainAccountPublicKey).get()).get();
		}
	}

	// endregion

	// region name

	TEST(TEST_CLASS, ObserverHasCorrectName) {
		// Act:
		auto pObserver = CreateKeyLinkObserver("Foo");

		// Assert:
		EXPECT_EQ("FooKeyLinkObserver", pObserver->name());
	}

	// endregion

	// region link

	TEST(TEST_CLASS, LinkCommitSetsLink) {
		// Act:
		auto notificationLinkedPublicKey = test::GenerateRandomByteArray<Key>();
		auto finalLinkedPublicKey = RunKeyLinkObserverTest(
				observers::NotifyMode::Commit,
				model::LinkAction::Link,
				Key(),
				notificationLinkedPublicKey);

		// Assert:
		EXPECT_EQ(notificationLinkedPublicKey, finalLinkedPublicKey);
	}

	TEST(TEST_CLASS, LinkRollbackUnsetsLink) {
		// Act:
		auto notificationLinkedPublicKey = test::GenerateRandomByteArray<Key>();
		auto finalLinkedPublicKey = RunKeyLinkObserverTest(
				observers::NotifyMode::Rollback,
				model::LinkAction::Link,
				notificationLinkedPublicKey,
				notificationLinkedPublicKey);

		// Assert:
		EXPECT_EQ(Key(), finalLinkedPublicKey);
	}

	// endregion

	// region unlink

	TEST(TEST_CLASS, UnlinkCommitUnsetsLink) {
		// Act:
		auto notificationLinkedPublicKey = test::GenerateRandomByteArray<Key>();
		auto finalLinkedPublicKey = RunKeyLinkObserverTest(
				observers::NotifyMode::Commit,
				model::LinkAction::Unlink,
				notificationLinkedPublicKey,
				notificationLinkedPublicKey);

		// Assert:
		EXPECT_EQ(Key(), finalLinkedPublicKey);
	}

	TEST(TEST_CLASS, UnlinkRollbackSetsLink) {
		// Act:
		auto notificationLinkedPublicKey = test::GenerateRandomByteArray<Key>();
		auto finalLinkedPublicKey = RunKeyLinkObserverTest(
				observers::NotifyMode::Rollback,
				model::LinkAction::Unlink,
				Key(),
				notificationLinkedPublicKey);

		// Assert:
		EXPECT_EQ(notificationLinkedPublicKey, finalLinkedPublicKey);
	}

	// endregion
}}
