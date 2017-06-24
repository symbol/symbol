#pragma once
#include "tests/catapult/observers/utils/MockTaggedBreadcrumbObserver.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/other/mocks/MockNotificationObserver.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

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
		state::CatapultState state;
		cache::CatapultCache cache({});
		auto cacheDelta = cache.createDelta();
		auto context = test::CreateObserverContext(cacheDelta, state, Height(123), NotifyMode::Commit);

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
		state::CatapultState state;
		cache::CatapultCache cache({});
		auto cacheDelta = cache.createDelta();
		auto context = test::CreateObserverContext(cacheDelta, state, Height(123), NotifyMode::Commit);

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
