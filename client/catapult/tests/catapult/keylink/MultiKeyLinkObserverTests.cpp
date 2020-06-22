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

#include "catapult/keylink/MultiKeyLinkObserver.h"
#include "tests/catapult/keylink/test/KeyLinkTestUtils.h"
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace keylink {

#define TEST_CLASS MultiKeyLinkObserverTests

	// region test utils

	namespace {
		using Notification = model::BasicKeyLinkNotification<model::PinnedVotingKey, static_cast<model::NotificationType>(0)>;

		struct Accessor {
			static auto& Get(state::AccountState& accountState) {
				return accountState.SupplementalPublicKeys.voting();
			}
		};

		auto CreateKeyLinkObserver(const std::string& name) {
			return keylink::CreateMultiKeyLinkObserver<Notification, Accessor>(name);
		}

		auto RunKeyLinkObserverTest(
				observers::NotifyMode notifyMode,
				model::LinkAction linkAction,
				const std::vector<model::PinnedVotingKey>& cacheLinkedPublicKeys,
				const model::PinnedVotingKey& notificationLinkedPublicKey) {
			// Arrange:
			test::AccountObserverTestContext context(notifyMode);
			auto mainAccountPublicKey = test::AddAccountWithMultiVotingLinks(context.cache(), cacheLinkedPublicKeys);

			auto pObserver = CreateKeyLinkObserver("");
			auto notification = Notification(mainAccountPublicKey, notificationLinkedPublicKey, linkAction);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			auto& accountStateCache = context.cache().sub<cache::AccountStateCache>();
			return Accessor::Get(accountStateCache.find(mainAccountPublicKey).get()).getAll();
		}

		model::PinnedVotingKey CreatePinnedVotingKey(FinalizationPoint::ValueType startPoint, FinalizationPoint::ValueType endPoint) {
			return { test::GenerateRandomByteArray<VotingKey>(), FinalizationPoint(startPoint), FinalizationPoint(endPoint) };
		}
	}

	// endregion

	// region name

	TEST(TEST_CLASS, ObserverHasCorrectName) {
		// Act:
		auto pObserver = CreateKeyLinkObserver("Foo");

		// Assert:
		EXPECT_EQ("FooMultiKeyLinkObserver", pObserver->name());
	}

	// endregion

	// region link

	TEST(TEST_CLASS, LinkCommitAddsLink) {
		// Arrange:
		constexpr auto Mode = observers::NotifyMode::Commit;

		// Act:
		auto pinnedVotingKeys = std::vector<model::PinnedVotingKey>{ CreatePinnedVotingKey(100, 150), CreatePinnedVotingKey(200, 250) };
		auto finalPinnedVotingKeys = RunKeyLinkObserverTest(Mode, model::LinkAction::Link, { pinnedVotingKeys[0] }, pinnedVotingKeys[1]);

		// Assert:
		EXPECT_EQ(pinnedVotingKeys, finalPinnedVotingKeys);
	}

	TEST(TEST_CLASS, LinkRollbackRemovesLink) {
		// Arrange:
		constexpr auto Mode = observers::NotifyMode::Rollback;

		// Act:
		auto pinnedVotingKeys = std::vector<model::PinnedVotingKey>{ CreatePinnedVotingKey(100, 150), CreatePinnedVotingKey(200, 250) };
		auto finalPinnedVotingKeys = RunKeyLinkObserverTest(Mode, model::LinkAction::Link, pinnedVotingKeys, pinnedVotingKeys[0]);

		// Assert:
		EXPECT_EQ(std::vector<model::PinnedVotingKey>{ pinnedVotingKeys[1] }, finalPinnedVotingKeys);
	}

	// endregion

	// region unlink

	TEST(TEST_CLASS, UnlinkCommitRemovesLink) {
		// Arrange:
		constexpr auto Mode = observers::NotifyMode::Commit;

		// Act:
		auto pinnedVotingKeys = std::vector<model::PinnedVotingKey>{ CreatePinnedVotingKey(100, 150), CreatePinnedVotingKey(200, 250) };
		auto finalPinnedVotingKeys = RunKeyLinkObserverTest(Mode, model::LinkAction::Unlink, pinnedVotingKeys, pinnedVotingKeys[0]);

		// Assert:
		EXPECT_EQ(std::vector<model::PinnedVotingKey>{ pinnedVotingKeys[1] }, finalPinnedVotingKeys);
	}

	TEST(TEST_CLASS, UnlinkRollbackAddsLink) {
		// Arrange:
		constexpr auto Mode = observers::NotifyMode::Rollback;

		// Act:
		auto pinnedVotingKeys = std::vector<model::PinnedVotingKey>{ CreatePinnedVotingKey(100, 150), CreatePinnedVotingKey(200, 250) };
		auto finalPinnedVotingKeys = RunKeyLinkObserverTest(Mode, model::LinkAction::Unlink, { pinnedVotingKeys[0] }, pinnedVotingKeys[1]);

		// Assert:
		EXPECT_EQ(pinnedVotingKeys, finalPinnedVotingKeys);
	}

	// endregion
}}
