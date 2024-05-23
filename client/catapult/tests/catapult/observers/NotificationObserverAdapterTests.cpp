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

#include "catapult/observers/NotificationObserverAdapter.h"
#include "tests/TestHarness.h"
#include "tests/test/core/mocks/MockNotificationPublisher.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/other/mocks/MockNotificationObserver.h"
#include "tests/test/plugins/ObserverTestUtils.h"

namespace catapult {
namespace observers {

#define TEST_CLASS NotificationObserverAdapterTests

	namespace {
		void ObserveEntity(const EntityObserver& observer, const model::VerifiableEntity& entity, test::ObserverTestContext& context) {
			Hash256 hash;
			observer.notify(model::WeakEntityInfo(entity, hash), context.observerContext());
		}

		template <typename TRunTestFunc>
		void RunTest(TRunTestFunc runTest) {
			// Arrange:
			auto pObserver = std::make_unique<mocks::MockNotificationObserver>("alpha");
			const auto& observer = *pObserver;

			auto registry = mocks::CreateDefaultTransactionRegistry(mocks::PluginOptionFlags::Publish_Custom_Notifications);
			auto pPublisher = model::CreateNotificationPublisher(registry, UnresolvedMosaicId(), Height());
			NotificationObserverAdapter adapter(std::move(pObserver), std::move(pPublisher));

			// Act + Assert:
			runTest(adapter, observer);
		}
	}

	TEST(TEST_CLASS, CanCreateAdapter) {
		// Arrange:
		RunTest([](const auto& adapter, const auto&) {
			// Assert:
			EXPECT_EQ("alpha", adapter.name());
		});
	}

	TEST(TEST_CLASS, ExtractsAndForwardsNotificationsFromEntity) {
		// Arrange:
		RunTest([](const auto& adapter, const auto& observer) {
			test::ObserverTestContext context(NotifyMode::Commit);

			// Act:
			auto pTransaction = mocks::CreateMockTransaction(0);
			ObserveEntity(adapter, *pTransaction, context);

			// Assert: the mock transaction plugin sends one additional public key notification and 6 custom notifications
			//         (notice that only 4/6 are raised on observer channel)
			EXPECT_EQ(5u + 1 + 4, observer.notificationTypes().size());

			auto expectedNotificationTypes = mocks::GetExpectedMockTransactionObserverNotificationTypes();
			EXPECT_EQ(expectedNotificationTypes, observer.notificationTypes());

			// - spot check the account public keys as a proxy for verifying data integrity
			ASSERT_EQ(2u, observer.accountPublicKeys().size());
			EXPECT_EQ(pTransaction->SignerPublicKey, observer.accountPublicKeys()[0]);
			EXPECT_EQ(pTransaction->RecipientPublicKey, observer.accountPublicKeys()[1]);
		});
	}

	TEST(TEST_CLASS, ForwardsObserverContexts) {
		// Arrange:
		RunTest([](const auto& adapter, const auto& observer) {
			test::ObserverTestContext context(NotifyMode::Commit);

			// Act:
			auto pTransaction = mocks::CreateMockTransaction(0);
			ObserveEntity(adapter, *pTransaction, context);

			// Assert: the context was forwarded to the notification observer
			ASSERT_EQ(5u + 1 + 4, observer.contextPointers().size());
			for (auto i = 0u; i < observer.contextPointers().size(); ++i)
				EXPECT_EQ(&context.observerContext(), observer.contextPointers()[i]) << "context at " << i;
		});
	}

	TEST(TEST_CLASS, CanSpecifyCustomPublisher) {
		// Arrange:
		auto pObserver = std::make_unique<mocks::MockNotificationObserver>("alpha");
		const auto& observer = *pObserver;

		auto pPublisher = std::make_unique<mocks::MockNotificationPublisher>();
		const auto& publisher = *pPublisher;

		NotificationObserverAdapter adapter(std::move(pObserver), std::move(pPublisher));

		auto pTransaction = mocks::CreateMockTransaction(0);
		test::ObserverTestContext context(NotifyMode::Commit);

		// Act:
		ObserveEntity(adapter, *pTransaction, context);

		// Assert: the publisher shouldn't produce any notifications, so the observer should never get called
		EXPECT_EQ(1u, publisher.numPublishCalls());
		EXPECT_EQ(0u, observer.notificationTypes().size());
	}
}
}
