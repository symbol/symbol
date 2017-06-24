#pragma once
#include "ObserverTestContext.h"
#include "catapult/observers/NotificationObserver.h"

namespace catapult { namespace test {

	// region NotificationObserver

	/// Observes \a notification with \a observer using \a context.
	template<typename TNotification>
	void ObserveNotification(
			const observers::NotificationObserverT<TNotification>& observer,
			const TNotification& notification,
			const observers::ObserverContext& context) {
		observer.notify(notification, context);
	}

	/// Observes \a notification with \a observer using \a context.
	template<typename TNotification, typename TCacheFactory>
	void ObserveNotification(
			const observers::NotificationObserverT<TNotification>& observer,
			const TNotification& notification,
			const ObserverTestContextT<TCacheFactory>& context) {
		ObserveNotification(observer, notification, context.observerContext());
	}

	// endregion

	/// Creates an observer context around \a state at \a height with specified \a mode.
	constexpr observers::ObserverContext CreateObserverContext(
			const observers::ObserverState& state,
			Height height,
			observers::NotifyMode mode) {
		return observers::ObserverContext(state, height, mode);
	}

	/// Creates an observer context around \a cache and \a state at \a height with specified \a mode.
	constexpr observers::ObserverContext CreateObserverContext(
			cache::CatapultCacheDelta& cache,
			state::CatapultState& state,
			Height height,
			observers::NotifyMode mode) {
		return observers::ObserverContext(cache, state, height, mode);
	}

/// Defines common observer tests for an observer with \a NAME.
#define DEFINE_COMMON_OBSERVER_TESTS(NAME, ...) \
	TEST(NAME##ObserverTests, CanCreate##NAME##Observer) { \
		auto pObserver = Create##NAME##Observer(__VA_ARGS__); \
		EXPECT_EQ(#NAME "Observer", pObserver->name()); \
	}
}}
