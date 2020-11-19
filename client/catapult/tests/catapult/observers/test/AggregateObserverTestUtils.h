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

#pragma once
#include "MockTaggedBreadcrumbObserver.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/other/mocks/MockNotificationObserver.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	template<typename TBuilder, typename TAddObserver>
	std::vector<const mocks::MockNotificationObserver*> AddSubObservers(TBuilder& builder, TAddObserver addObserver, size_t count) {
		std::vector<const mocks::MockNotificationObserver*> observers;
		for (auto i = 0u; i < count; ++i) {
			auto pMockObserver = std::make_unique<mocks::MockNotificationObserver>(std::to_string(i));
			observers.push_back(pMockObserver.get());
			addObserver(builder, std::move(pMockObserver));
		}

		return observers;
	}

	/// Asserts that the observer created by \a builder delegates to all sub observers and passes notifications correctly.
	/// \a addObserver is used to add sub observers to \a builder.
	template<typename TBuilder, typename TAddObserver>
	void AssertNotificationsAreForwardedToChildObservers(TBuilder&& builder, TAddObserver addObserver) {
		// Arrange:
		cache::CatapultCache cache({});
		auto cacheDelta = cache.createDelta();
		auto context = test::CreateObserverContext(cacheDelta, Height(123), observers::NotifyMode::Commit);

		// - create an aggregate with five observers
		auto observers = AddSubObservers(builder, addObserver, 5);
		auto pAggregateObserver = builder.build();

		// - create two notifications
		auto notification1 = test::CreateNotification(static_cast<model::NotificationType>(7));
		auto notification2 = test::CreateNotification(static_cast<model::NotificationType>(2));

		// Act:
		pAggregateObserver->notify(notification1, context);
		pAggregateObserver->notify(notification2, context);

		// Assert:
		auto i = 0u;
		for (const auto& pObserver : observers) {
			const auto& notificationTypes = pObserver->notificationTypes();
			const auto message = "observer at " + std::to_string(i);

			ASSERT_EQ(2u, notificationTypes.size()) << message;
			EXPECT_EQ(notification1.Type, notificationTypes[0]) << message;
			EXPECT_EQ(notification2.Type, notificationTypes[1]) << message;
			++i;
		}
	}

	/// Asserts that the observer created by \a builder delegates to all sub observers and passes contexts correctly.
	/// \a addObserver is used to add sub observers to \a builder.
	template<typename TBuilder, typename TAddObserver>
	void AssertContextsAreForwardedToChildObservers(TBuilder&& builder, TAddObserver addObserver) {
		// Arrange:
		cache::CatapultCache cache({});
		auto cacheDelta = cache.createDelta();
		auto context = test::CreateObserverContext(cacheDelta, Height(123), observers::NotifyMode::Commit);

		// - create an aggregate with five observers
		auto observers = AddSubObservers(builder, addObserver, 5);
		auto pAggregateObserver = builder.build();

		// - create two notifications
		auto notification1 = test::CreateNotification(static_cast<model::NotificationType>(7));
		auto notification2 = test::CreateNotification(static_cast<model::NotificationType>(2));

		// Act:
		pAggregateObserver->notify(notification1, context);
		pAggregateObserver->notify(notification2, context);

		// Assert:
		auto i = 0u;
		for (const auto& pObserver : observers) {
			const auto& contextPointers = pObserver->contextPointers();
			const auto message = "observer at " + std::to_string(i);

			ASSERT_EQ(2u, contextPointers.size()) << message;
			EXPECT_EQ(&context, contextPointers[0]) << message;
			EXPECT_EQ(&context, contextPointers[1]) << message;
			++i;
		}
	}
}}
